#include "libbmp.h"
#include <stdlib.h>

// TODO: rewrite all logs and erros

// FIXME: Invalid read because of pixel bit count 
// See description in libbmp.h
int bmp_decode_image(FILE* input, struct bmp_image** image) {
    *image = malloc(sizeof(struct bmp_image));
    fread(&(*image)->bmfh, sizeof(struct bmp_file_header), 1, input);
    fread(&(*image)->bmih, sizeof(struct bmp_info_header), 1, input);

    // Something with color table

    // Forget anything else and seek to actual data
    fseek(input, (*image)->bmfh.off_bits, SEEK_SET);

    uint32_t height = (*image)->bmih.height;
    uint32_t width = (*image)->bmih.width;
    // uint16_t pixel_size = (*image)->bmih.bit_count;
    (*image)->bitmap = malloc(sizeof(char*) * height);

    for (size_t i = 0; i < height; i++) {
        (*image)->bitmap[i] = malloc(sizeof(char) * width);
        fread((*image)->bitmap[i], sizeof(char), width, input);
    }

    return 0;
}

// FIXME: Invalid read because of pixel bit count 
// See description in libbmp.h
int bmp_encode_image(FILE* output, struct bmp_image* image) {
    fwrite(&image->bmfh, sizeof(struct bmp_file_header), 1, output);
    fwrite(&image->bmih, sizeof(struct bmp_info_header), 1, output);

    // Something with color table

    uint32_t height = image->bmih.height;
    uint32_t width = image->bmih.width;
    
    for (size_t i = 0; i < height; i++) {
        fwrite(image->bitmap[i], sizeof(char), width, output);
    }

    return 0;
}

// See description in libbmp.h
char* bmp_pixel(struct bmp_image* image, uint32_t line, uint32_t column) {
    return &image->bitmap[image->bmih.height - line][column];
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
