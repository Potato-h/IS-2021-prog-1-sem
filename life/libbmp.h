// Documentation of bitmap structure format was found there:
// https://web.archive.org/web/20080912171714/http://www.fortunecity.com/skyscraper/windows/364/bmpffrmt.html

#ifndef LIBBMP_HEADER
#define LIBBMP_HEADER

#include <stdio.h>
#include <stdint.h>

// File header of bitmap. Represent general information about file with image.
struct bmp_file_header {
    char        type[2];     // File type. Always holds 'BM'
    uint32_t    size;        // Size of file in bytes
    uint16_t    reserved1;   // Must always be set to zero
    uint16_t    reserved2;   // Must always be set to zero
    uint32_t    off_bits;    // Offset from beginning of the file to bitmap data
} __attribute__((packed));

// Info header of bitmap. Repesent actual information about image.
struct bmp_info_header {
    uint32_t    size;	        // Size of bmp_info_header    
	uint32_t	width;          // Width of the image in pixels
	uint32_t	height;	        // Height of the image in pixels
	uint16_t	planes;	        // ???
	uint16_t	bit_count;      // Number of bits per pixel
	uint32_t	compression;    // Type of compression (usually set to zero) 
	uint32_t	size_image;     // Size of the image data 
	uint32_t	xPelsPerMeter;  // Horizontal pixels per meter
	uint32_t	yPelsPerMeter;  // Vertical pixels per meter
	uint32_t	clr_used;       // Number of colors used in bitmap
	uint32_t	clr_important;  // Number of important colors
} __attribute__((packed));

struct bmp_rgbquad {
    uint8_t blue;       // Blue part of the color
    uint8_t green;      // Green part of the color
    uint8_t red;        // Red part of the color
    uint8_t reserved;   // Must always be set to zero
} __attribute__((packed));

struct bmp_image {
    struct bmp_file_header  bmfh;
    struct bmp_info_header  bmih;
    struct bmp_rgbquad*     colors;
    uint8_t**               bitmap;
};

// Parse from file.
int bmp_decode_image(FILE* input, struct bmp_image** image);

// Put bmp into file.
int bmp_encode_image(FILE* output, struct bmp_image* image);

// Get byte of bitmap in given position. Doesn't check range.
uint32_t bmp_get_pixel(struct bmp_image* image, uint32_t line, uint32_t column);

// Set byte of bitmap in given position. Doesn't check range.
void bmp_set_pixel(struct bmp_image* image, uint32_t row, uint32_t column, uint32_t color);

// Copy given image and allocate memory for it. So, bmp_free_image
// call at the end is needed.
struct bmp_image* bmp_copy_image(struct bmp_image* image);

// Free all memory allocated from decode function. Set image to NULL on success.
void bmp_free_image(struct bmp_image** image);

#endif