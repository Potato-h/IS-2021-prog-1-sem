// Audio meta tag parser. See https://id3.org/id3v2.4.0-structure document
// to understand topic and comments style agreement.

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

// TODO: Add macros to get particular bits from flags
#define UNSYNCH     0b10000000
#define EXHEADER    0b01000000
#define EXPERIMENT  0b00100000
#define FOOTER      0b00010000

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

#define member_size(type, member) sizeof(((type *)0)->member)

// TODO: Solve memory allocation issues
// TODO: Think about right API for show functions (return string instead of write into file)
// TODO: Print line in the error output
// TODO: Check that padding in structs doesn't cause any fread/fwrite problems
// TODO: Make `user` readable output

// Synchsafe 32 bit integer (in this format: 4 * %0xxxxxxx)
// To understand why it is exist see mp3 format
typedef struct id3v2_synchsafe32_t {
    uint8_t inner[4];
} id3v2_synchsafe32_t;

uint32_t id3v2_synchsafe_to_uint32(id3v2_synchsafe32_t synchsafe);

// TODO: make union from version or replace it with separate 
// major and revision fields
struct id3v2_header {
    char id3[3];                // Always holds "ID3"

    uint8_t version[2];         // Version. Always holds $40 00. 
                                // version[0] -- major version
                                // version[1] -- revision number 

    uint8_t flags;              // Flag in following format: %abcd0000
                                // a -- Unsynchronisation
                                // b -- Extended header
                                // c -- Experimental indicator
                                // d -- Footer present

    id3v2_synchsafe32_t size;   // Size as synchsafe 32 bit integer.
                                // Size is the sum of the byte length of the extended
                                // header, the padding and the frames after unsynchronisation.
};

// Decode header from input to header. Seek input to header size. 
// TODO: add ID3 data search
int id3v2_decode_header(FILE* input, struct id3v2_header* header);

int id3v2_encode_header(FILE* output, struct id3v2_header* header);

// TODO: Check for error
int id3v2_show_header(FILE* output, struct id3v2_header* header);

// TODO: update doc
struct id3v2_exheader {
    id3v2_synchsafe32_t size;   // Size as synchsafe 32 bit integer.

    uint8_t flag_size;          // Number of flag bytes = $01
                                // so size is 1 byte for each flag

    uint8_t exflags;            // Extended Flags in following format: $xx 

    void* flags_info;
};

struct id3v2_exheader* id3v2_allocate_exheader();

// Decode extender header from input to exheader. 
// Allocate exheader via malloc. Seek input to extended header size.
int id3v2_decode_exheader(FILE* input, struct id3v2_exheader** exheader);

int id3v2_encode_exheader(FILE* ouput, struct id3v2_exheader* exheader);

int id3v2_show_exheader(FILE* output, struct id3v2_exheader* exheader);

// Free extended header. Set exheader pointer to NULL.
void id3v2_free_exheader(struct id3v2_exheader** exheader);

// TODO: change void* to union
// Constranins: content ptr should always hold memory allocated 
// by malloc for correct free call.
struct id3v2_frame {
    char id[4];                 // Frame ID in following format: $xx xx xx xx

    id3v2_synchsafe32_t size;   // Size as synchsafe 32 bit integer.

    uint8_t flags[2];           // Flags in following format: $xx xx

    void* content;              // Special content for each frame type, 
                                // for example, TXXX frames has text after header,
                                // but MLLT has own table format.

    struct id3v2_frame* next;
};

enum id3v2_frame_type {
    ID3V2_TEXT,
    ID3V2_URL,
};

struct id3v2_frame* id3v2_allocate_frame();

enum id3v2_frame_type id3v2_get_frame_type(char* id);

int id3v2_decode_text_frame(FILE* input, struct id3v2_frame** frame);

int id3v2_encode_text_frame(FILE* output, struct id3v2_frame* frame);

// TODO: implement different encodings
int id3v2_show_text_frame(FILE* output, struct id3v2_frame* frame);

// Decode one frame from input to frame. Seek input to frame size.
int id3v2_decode_frame(FILE* input, struct id3v2_frame** frame);

int id3v2_encode_frame(FILE* output, struct id3v2_frame* frame);

int id3v2_show_frame(FILE* output, struct id3v2_frame* frame);

// Free frame. Set exheader pointer to NULL.
void id3v2_free_frame(struct id3v2_frame** frame);

struct id3v2_padding {
    uint8_t padding;
};

// Copy of the header, but with a different identifier.
// Can be used to speed up search from the end of the file.
struct id3v2_footer {
    char id3[3];                // Always holds "3DI"

    uint8_t version[2];         // Version. Always holds $40 00. 
                                // version[0] -- major version
                                // version[1] -- revision number 

    uint8_t flags;              // Flag in following format: %abcd0000
                                // a -- Unsynchronisation
                                // b -- Extended header
                                // c -- Experimental indicator
                                // d -- Footer present

    id3v2_synchsafe32_t size;   // Size as synchsafe 32 bit integer.
                                // Size is the sum of the byte length of the extended
                                // header, the padding and the frames after unsynchronisation.
};

struct id3v2_tag {
    struct id3v2_header header;
    struct id3v2_exheader* exheader;
    struct id3v2_frame* frames_head;
    struct id3v2_padding* padding;
    struct id3v2_footer* footer;  
};

// Read tag information from input and decode it into tag.
// Allocate memory for tag via malloc, so free_tag call is needed.
// Return error code (0 in success case).
int id3v2_decode_tag(FILE* input, struct id3v2_tag** tag);

// Write raw tag information to output.
// Return error code (0 in success case).
int id3v2_encode_tag(FILE* output, struct id3v2_tag* tag);

// Write human-readable tag information to output.
// Return error code (0 in success case).
int id3v2_show_tag(FILE* output, struct id3v2_tag* tag);

// TODO: Is this right signature? For example, we can set some T*** tag
// but we need to know (?) encoding to do this. Is encoding is part of value 
// or there is should be special variable or encoding is always UTF-8 
// (which will be very strange). 
//
// Set prop frame to value.
// Return error code (0 in success case).
int id3v2_set(struct id3v2_tag* tag, char* prop, char* value);

// Get prop frame information.
// Return information from get_prop frame. 
// Result can be NULL.
char* id3v2_get(struct id3v2_tag* tag, char* prop);

// Deallocate memory for tag and set pointer to NULL.
// Guarantee correct behaviour, when NULL passed.
void id3v2_free_tag(struct id3v2_tag** tag);