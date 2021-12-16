#include "libbmp.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define _BSD_SOURCE
#define __USE_BSD
#include <endian.h>

// TODO: Add hasher
// TODO: rewrite all logs and erros
// TODO: all magic constants MUST BE erased

uint32_t bmp_pixels_to_bytes(uint32_t width) {
    // width is provided in pixel and we assume, that image is 1-bit colored
    uint32_t bytes_in_row = (sizeof(uint8_t) * width + 7) / 8;
    // bmp format want to have number of bytes in a row reaches a multiple of four
    bytes_in_row += (4 - (bytes_in_row % 4)) % 4;

    return bytes_in_row;
}

// See description in libbmp.h
int bmp_decode_image(FILE* input, struct bmp_image** _image) {
    *_image = malloc(sizeof(struct bmp_image));
    struct bmp_image* image = *_image;
    // read headers
    fread(&image->bmfh, sizeof(struct bmp_file_header), 1, input);
    fread(&image->bmih, sizeof(struct bmp_info_header), 1, input);

    // bmp format is little endian
    image->bmfh.size            = le32toh(image->bmfh.size);
    image->bmfh.off_bits        = le32toh(image->bmfh.off_bits);

    // bmp format is little endian
    image->bmih.size            = le32toh(image->bmih.size);	        
	image->bmih.width           = le32toh(image->bmih.width);          
	image->bmih.height          = le32toh(image->bmih.height);	        
	image->bmih.planes          = le16toh(image->bmih.planes);	        
	image->bmih.bit_count       = le16toh(image->bmih.bit_count);      
	image->bmih.compression     = le32toh(image->bmih.compression);    
	image->bmih.size_image      = le32toh(image->bmih.size_image);     
	image->bmih.xPelsPerMeter   = le32toh(image->bmih.xPelsPerMeter);  
	image->bmih.yPelsPerMeter   = le32toh(image->bmih.yPelsPerMeter);  
	image->bmih.clr_used        = le32toh(image->bmih.clr_used);       
	image->bmih.clr_important   = le32toh(image->bmih.clr_important);

    // read color table
    image->colors = malloc(sizeof(struct bmp_rgbquad) * (1 << image->bmih.bit_count));
    fread(image->colors, sizeof(struct bmp_rgbquad), 1 << image->bmih.bit_count, input);
    
    // TODO: add support for multi color image
    // Suppose, that we have black/white image
    assert(image->bmih.bit_count == 1);
    
    uint16_t bytes_in_row = bmp_pixels_to_bytes(image->bmih.width);
    image->bitmap = malloc(sizeof(uint8_t*) * image->bmih.height);

    // Forget anything else and seek to actual data
    fseek(input, image->bmfh.off_bits, SEEK_SET);

    for (size_t i = 0; i < image->bmih.height; i++) {
        image->bitmap[i] = malloc(sizeof(uint8_t) * bytes_in_row);
        fread(image->bitmap[i], sizeof(uint8_t), bytes_in_row, input);
    }

    return 0;
}

// FIXME: mess up endianess after encode
// See description in libbmp.h
int bmp_encode_image(FILE* output, struct bmp_image* image) {
    uint32_t off_bits = image->bmfh.off_bits;
    
    // bmp format is little endian
    image->bmfh.size            = htole32(image->bmfh.size);
    image->bmfh.off_bits        = htole32(image->bmfh.off_bits);

    // bmp format is little endian
    image->bmih.size            = htole32(image->bmih.size);	        
	image->bmih.width           = htole32(image->bmih.width);          
	image->bmih.height          = htole32(image->bmih.height);	        
	image->bmih.planes          = htole16(image->bmih.planes);	        
	image->bmih.bit_count       = htole16(image->bmih.bit_count);      
	image->bmih.compression     = htole32(image->bmih.compression);    
	image->bmih.size_image      = htole32(image->bmih.size_image);     
	image->bmih.xPelsPerMeter   = htole32(image->bmih.xPelsPerMeter);  
	image->bmih.yPelsPerMeter   = htole32(image->bmih.yPelsPerMeter);  
	image->bmih.clr_used        = htole32(image->bmih.clr_used);       
	image->bmih.clr_important   = htole32(image->bmih.clr_important);

    // write headers
    fwrite(&image->bmfh, sizeof(struct bmp_file_header), 1, output);
    fwrite(&image->bmih, sizeof(struct bmp_info_header), 1, output);

    // write color table
    fwrite(image->colors, sizeof(struct bmp_rgbquad), 1 << image->bmih.bit_count, output);

    fseek(output, off_bits, SEEK_SET);

    uint32_t bytes_in_row = bmp_pixels_to_bytes(image->bmih.width);

    for (size_t i = 0; i < image->bmih.height; i++) {
        fwrite(image->bitmap[i], sizeof(uint8_t), bytes_in_row, output);
    }

    return 0;
}

// FIXME: very terrifing expression
// FIXME: function hardcoded to work with 1-bit pixel
// See description in libbmp.h
uint32_t bmp_get_pixel(struct bmp_image* image, uint32_t row, uint32_t column) {
    uint8_t byte = image->bitmap[image->bmih.height - row - 1][column / 8];
    return (byte >> ((7 - (column % 8)) % 8)) & 1;
}

// FIXME: very terrifing expression
// FIXME: function hardcoded to work with 1-bit pixel
// See description in libbmp.h
void bmp_set_pixel(struct bmp_image* image, uint32_t row, uint32_t column, uint32_t color) {
    uint8_t* byte = &image->bitmap[image->bmih.height - row - 1][column / 8];
    *byte ^= (-color ^ *byte) & (1 << ((7 - (column % 8)) % 8));
}

// See description in libbmp.h
struct bmp_image* bmp_copy_image(struct bmp_image* image) {
    struct bmp_image* copy = malloc(sizeof(struct bmp_image));

    // copy headers
    memcpy(&copy->bmfh, &image->bmfh, sizeof(image->bmfh));
    memcpy(&copy->bmih, &image->bmih, sizeof(image->bmih));
    
    // copy color table
    copy->colors = malloc(sizeof(struct bmp_rgbquad) * (1 << image->bmih.bit_count));
    memcpy(copy->colors, image->colors, sizeof(struct bmp_rgbquad) * (1 << image->bmih.bit_count));

    uint32_t bytes_in_row = bmp_pixels_to_bytes(image->bmih.width);
    copy->bitmap = malloc(sizeof(uint8_t*) * image->bmih.height);

    for (size_t i = 0; i < image->bmih.height; i++) {
        copy->bitmap[i] = malloc(sizeof(uint8_t) * bytes_in_row);
        memcpy(copy->bitmap[i], image->bitmap[i], sizeof(uint8_t) * bytes_in_row);
    }

    return copy;
}

// See description in libbmp.h
void bmp_free_image(struct bmp_image** image) {
    if (image == NULL || *image == NULL) {
        return;
    }

    for (size_t i = 0; i < (*image)->bmih.height; i++) {
        free((*image)->bitmap[i]);
    }

    free((*image)->colors);
    free((*image)->bitmap);
    free(*image);
    *image = NULL;
}
