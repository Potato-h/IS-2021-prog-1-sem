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

// TODO: Think about right API for show functions (return string instead of write into file)
// TODO: Make `user` readable output
// TODO: Should char* contains \0?
// TODO: Check all offset and member_size
// TODO: Add return code desription.
// TODO: Move all doc in one place
// TODO: Group all functions by their relation to special frame

// Synchsafe 32 bit integer (in this format: 4 * %0xxxxxxx)
// To understand why it is exist see mp3 format
typedef struct id3v2_synchsafe32_t {
    uint8_t inner[4];
} id3v2_synchsafe32_t;

uint32_t id3v2_synchsafe_to_uint32(id3v2_synchsafe32_t synchsafe);

// TODO: make union from version or replace it with separate 
// major and revision fields
// Constrain: decode and encode functions consider, that there is no aligen
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
} __attribute__((packed));

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
} __attribute__((packed));

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
} __attribute__((packed));

// All uses in function table MUST have same order
enum id3v2_frame_type {
    ID3V2_TEXT,
    ID3V2_URL,
    ID3V2_COMMENT,
    ID3V2_UNSUPPORTED,
};

enum id3v2_encoding {
    ID3V2_ISO,
    ID3V2_UTF16,
    ID3V2_UTF16BE,
    ID3V2_UTF8,
};

// Convert enum into human readable string. Share static memory
// so free call on this pointer is forbidden. 
const char* id3v2_encoding_to_str(enum id3v2_encoding encoding);

enum id3v2_frame_type id3v2_get_frame_type(char* id);

struct id3v2_text_frame_content {
    enum id3v2_encoding encoding; // $xx
    char* text;
};

// Text frame functions

struct id3v2_text_frame_content* id3v2_allocate_text_frame_content(size_t content_size);

int id3v2_decode_text_frame(FILE* input, struct id3v2_frame* frame);

int id3v2_encode_text_frame(FILE* output, struct id3v2_frame* frame);

// TODO: implement different encodings
int id3v2_show_text_frame(FILE* output, struct id3v2_frame* frame);

void id3v2_free_text_frame_content(void** content);

struct id3v2_url_frame_content {
    char* url;
};

// URL frame functions

struct id3v2_url_frame_content* id3v2_allocate_url_frame_content(size_t content_size);

int id3v2_decode_url_frame(FILE* input, struct id3v2_frame* frame);

int id3v2_encode_url_frame(FILE* output, struct id3v2_frame* frame);

int id3v2_show_url_frame(FILE* output, struct id3v2_frame* frame);

void id3v2_free_url_frame_content(void** content);

struct id3v2_comment_frame_content {
    uint8_t encoding;       // $xx
    uint8_t language[3];    // $xx xx xx
    char* description;      // Description always ends with $00
    char* text;             // Actual comment
} __attribute__((packed));

// Comment frame functions

struct id3v2_comment_frame_content* id3v2_allocate_comment_frame_content();

int id3v2_decode_comment_frame(FILE* input, struct id3v2_frame* frame);

int id3v2_encode_comment_frame(FILE* output, struct id3v2_frame* frame);

int id3v2_show_comment_frame(FILE* output, struct id3v2_frame* frame);

void id3v2_free_comment_frame_content(void** content);

// Unsupported frame functions
void* id3v2_allocate_unsupported_frame_content(size_t content_size);

int id3v2_decode_unsupported_frame(FILE* input, struct id3v2_frame* frame);

int id3v2_encode_unsupported_frame(FILE* output, struct id3v2_frame* frame);

int id3v2_show_unsupported_frame(FILE* output, struct id3v2_frame* frame);

void id3v2_free_unsupported_frame_content(void** content);

// General frame functions

// Decode one frame from input to frame. Seek input to frame size.
int id3v2_decode_frame(FILE* input, struct id3v2_frame** frame);

int id3v2_encode_frame(FILE* output, struct id3v2_frame* frame);

int id3v2_show_frame(FILE* output, struct id3v2_frame* frame);

struct id3v2_frame* id3v2_allocate_frame();

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
} __attribute__((packed));

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

// Get prop frame information.
// Return information from get_prop frame. 
// Result can be NULL.
struct id3v2_frame* id3v2_get(struct id3v2_tag* tag, char* prop);

// TODO: Is this right signature? For example, we can set some T*** tag
// but we need to know (?) encoding to do this. Is encoding is part of value 
// or there is should be special variable or encoding is always UTF-8 
// (which will be very strange). 
//
// Set prop frame to value.
// Return error code (0 in success case).
int id3v2_set(struct id3v2_tag* tag, struct id3v2_frame* frame);

// Deallocate memory for tag and set pointer to NULL.
// Guarantee correct behaviour, when NULL passed.
void id3v2_free_tag(struct id3v2_tag** tag);