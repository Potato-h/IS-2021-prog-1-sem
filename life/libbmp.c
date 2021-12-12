#include "libbmp.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// TODO: rewrite all logs and erros
// TODO: make solution independent in terms of endianness
// TODO: all magic constants MUST BE erased
// TODO: see gtol/ltoh functions

uint16_t le16_to_cpu(uint8_t* x) {
    return (uint16_t)x[0]
    | ((uint16_t)x[1] << 8);
}

uint32_t le32_to_cpu(uint8_t* x) {
    return (uint32_t)x[0] 
    | ((uint32_t)x[1] << 8) 
    | ((uint32_t)x[2] << 16) 
    | ((uint32_t)x[3] << 24);
}

uint16_t cpu_to_le16(uint16_t x) {
    uint8_t buf[] = { x & 0xFF, (x >> 8) & 0xFF };
    return *(uint16_t*)buf;
}

uint32_t cpu_to_le32(uint32_t x) {
    uint8_t buf[] = { x & 0xFF, (x >> 8) & 0xFF, (x >> 16) & 0xFF, (x >> 24) & 0xFF };
    return *(uint32_t*)buf;
}

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
    fread(&image->bmfh, sizeof(struct bmp_file_header), 1, input);
    fread(&image->bmih, sizeof(struct bmp_info_header), 1, input);

    image->bmfh.off_bits     = le32_to_cpu((uint8_t*)&image->bmfh.off_bits);
    image->bmfh.size         = le32_to_cpu((uint8_t*)&image->bmfh.size);

    // FIXME: fix endianness for other fields
    image->bmih.height       = le32_to_cpu((uint8_t*)&image->bmih.height);
    image->bmih.width        = le32_to_cpu((uint8_t*)&image->bmih.width); 
    image->bmih.bit_count    = le16_to_cpu((uint8_t*)&image->bmih.bit_count);

    // Something with color table
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

// FIXME: Write trash on simple read/write without changes
// FIXME: Write in file with correct endianness
// FIXME: Invalid read because of pixel bit count 
// See description in libbmp.h
int bmp_encode_image(FILE* output, struct bmp_image* image) {
    uint32_t off_bits = image->bmfh.off_bits;
    
    image->bmfh.off_bits     = cpu_to_le32(image->bmfh.off_bits);
    image->bmfh.size         = cpu_to_le32(image->bmfh.size);

    // FIXME: fix endianness for other fields
    image->bmih.height       = cpu_to_le32(image->bmih.height);
    image->bmih.width        = cpu_to_le32(image->bmih.width); 
    image->bmih.bit_count    = cpu_to_le16(image->bmih.bit_count);

    fwrite(&image->bmfh, sizeof(struct bmp_file_header), 1, output);
    fwrite(&image->bmih, sizeof(struct bmp_info_header), 1, output);

    // Something with color table
    // TODO: make normal color table output
    char black[] = { 0, 0, 0, 0 };
    char white[] = { 255, 255, 255, 0 };

    fwrite(black, sizeof(black), 1, output);
    fwrite(white, sizeof(white), 1, output);

    // skip color table
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

    memcpy(&copy->bmfh, &image->bmfh, sizeof(image->bmfh));
    memcpy(&copy->bmih, &image->bmih, sizeof(image->bmih));
    // copy color table

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
    for (size_t i = 0; i < (*image)->bmih.height; i++) {
        free((*image)->bitmap[i]);
    }

    free((*image)->bitmap);
    free(*image);
    *image = NULL;
}
