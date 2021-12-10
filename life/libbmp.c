#include "libbmp.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// TODO: rewrite all logs and erros
// TODO: make solution independent in terms of endianness

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

// FIXME: Invalid read because of pixel bit count 
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

    uint32_t height = image->bmih.height;
    uint32_t width = image->bmih.width;

    // Something with color table
    assert(image->bmih.bit_count == 1);
    
    // width is provided in pixel and we assume, that image is 1-bit colored
    uint32_t width_bytes = (sizeof(uint8_t) * width + 7) / 8;
    // bmp format want to have number of bytes in a row reaches a multiple of four
    width_bytes += 4 - (width_bytes % 4);
    image->bitmap = malloc(sizeof(uint8_t*) * height);

    // Forget anything else and seek to actual data
    fseek(input, image->bmfh.off_bits, SEEK_SET);

    for (size_t i = 0; i < height; i++) {
        image->bitmap[i] = malloc(sizeof(uint8_t) * width_bytes);
        fread(image->bitmap[i], sizeof(uint8_t), width_bytes, input);
    }

    return 0;
}

// FIXME: write in file with correct endianness
// FIXME: Invalid read because of pixel bit count 
// See description in libbmp.h
int bmp_encode_image(FILE* output, struct bmp_image* image) {
    fwrite(&image->bmfh, sizeof(struct bmp_file_header), 1, output);
    fwrite(&image->bmih, sizeof(struct bmp_info_header), 1, output);

    // Something with color table

    uint32_t height = image->bmih.height;
    uint32_t width = image->bmih.width;
    
    for (size_t i = 0; i < height; i++) {
        fwrite(image->bitmap[i], sizeof(uint8_t), width, output);
    }

    return 0;
}

// FIXME: function hardcoded to work with 1-bit pixel
// See description in libbmp.h
uint8_t bmp_get_pixel(struct bmp_image* image, uint32_t row, uint32_t column) {
    uint8_t byte = image->bitmap[image->bmih.height - row][column / 8];
    return (byte >> (column % 8)) & 1;
}

// FIXME: function hardcoded to work with 1-bit pixel
// See description in libbmp.h
void bmp_set_pixel(struct bmp_image* image, uint32_t row, uint32_t column, uint8_t pixel) {
    uint8_t* byte = &image->bitmap[image->bmih.height - row][column / 8];
    *byte ^= (-pixel ^ *byte) & (1UL << (column % 8));
}

// See description in libbmp.h
struct bmp_image* bmp_copy_image(struct bmp_image* image) {
    struct bmp_image* copy = malloc(sizeof(struct bmp_image));

    memcpy(&copy->bmfh, &image->bmfh, sizeof(image->bmfh));
    memcpy(&copy->bmih, &image->bmih, sizeof(image->bmih));
    // copy color table

    uint32_t height = image->bmih.height;
    uint32_t width = image->bmih.width;

    // width is provided in pixel and we assume, that image is 1-bit colored
    uint32_t width_bytes = (sizeof(uint8_t) * width + 7) / 8;
    // bmp format want to have number of bytes in a row reaches a multiple of four
    width_bytes += 4 - (width_bytes % 4);
    copy->bitmap = malloc(sizeof(uint8_t*) * height);

    for (size_t i = 0; i < height; i++) {
        copy->bitmap[i] = malloc(sizeof(uint8_t) * width_bytes);
        memcpy(copy->bitmap[i], image->bitmap[i], sizeof(uint8_t) * width_bytes);
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
