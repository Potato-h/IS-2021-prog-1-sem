// Audio meta tag parser. See https://id3.org/id3v2.4.0-structure document
// to understand topic and comments style agreement.

#include "libid3v2.h"

// TODO: Solve memory allocation issues
// TODO: Think about right API for show functions (return string instead of write into file)
// TODO: Print line in the error output
// TODO: Check that padding in structs doesn't cause any fread/fwrite problems
// TODO: Make `user` readable output

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */

const char* id3v2_encoding_to_str(enum id3v2_encoding encoding) {
    static const char* encodings[] = {
        "ISO",
        "UTF16",
        "UTF16BE",
        "UTF8",
    };
    
    return encodings[encoding];
}

uint32_t id3v2_synchsafe_to_uint32(id3v2_synchsafe32_t synchsafe) {
    uint32_t byte0 = synchsafe.inner[0];
    uint32_t byte1 = synchsafe.inner[1];
    uint32_t byte2 = synchsafe.inner[2];
    uint32_t byte3 = synchsafe.inner[3];

    return byte0 << 21 | byte1 << 14 | byte2 << 7 | byte3;
}

// Decode header from input to header. Seek input to header size. 
// TODO: add ID3 data search
int id3v2_decode_header(FILE* input, struct id3v2_header* header) {    
    return fread((void*)header, sizeof(struct id3v2_header), 1, input) < 1;
}

int id3v2_encode_header(FILE* output, struct id3v2_header* header) {
    return fwrite((void*)header, sizeof(struct id3v2_header), 1, output) < 1;
}

// TODO: Check for error
int id3v2_show_header(FILE* output, struct id3v2_header* header) {
    fprintf(
        output, 
        "id: %s\n"
        "major: %u, minor: %u\n"
        "flags: 0b"BYTE_TO_BINARY_PATTERN"\n"
        "size: %u\n",
        header->id3,
        header->version[0], header->version[1],
        BYTE_TO_BINARY(header->flags),
        id3v2_synchsafe_to_uint32(header->size)
    );

    return 0;
}

struct id3v2_exheader* id3v2_allocate_exheader() {
    struct id3v2_exheader* exheader = (struct id3v2_exheader*)malloc(sizeof(struct id3v2_exheader));
    exheader->flags_info = NULL; 
    return exheader;
}

// Decode extender header from input to exheader. 
// Allocate exheader via malloc. Seek input to extended header size.
int id3v2_decode_exheader(FILE* input, struct id3v2_exheader** exheader) {
    fprintf(stderr, YELLOW "unimplemented %s:%d\n" RESET, __FILE__, __LINE__);
    return 1;
}

int id3v2_encode_exheader(FILE* ouput, struct id3v2_exheader* exheader) {
    if (!exheader)
        return 0;

    fprintf(stderr, YELLOW "unimplemented %s:%d\n" RESET, __FILE__, __LINE__);
    return 1;
}

int id3v2_show_exheader(FILE* output, struct id3v2_exheader* exheader) {
    if (!exheader)
        return 0;

    fprintf(stderr, YELLOW "unimplemented %s:%d\n" RESET, __FILE__, __LINE__);
    return 1;
}

// Free extended header. Set exheader pointer to NULL.
void id3v2_free_exheader(struct id3v2_exheader** exheader) {
    if (!*exheader)
        return;

    fprintf(stderr, YELLOW "unimplemented %s:%d\n" RESET, __FILE__, __LINE__);
    return;
}

struct id3v2_frame* id3v2_allocate_frame() {
    struct id3v2_frame* frame = (struct id3v2_frame*)malloc(sizeof(struct id3v2_frame));
    frame->content = NULL;
    frame->next = NULL;
    return frame;
}

enum id3v2_frame_type id3v2_get_frame_type(char* id) {
    if (id[0] == 'T' && strncmp(id, "TXXX", strlen("TXXX")) != 0) {
        return ID3V2_TEXT;
    }

    if (id[0] == 'W' && strncmp(id, "WXXX", strlen("WXXX")) != 0) {
        return ID3V2_URL;
    }

    if (strncmp(id, "COMM", strlen("COMM")) == 0) {
        return ID3V2_COMMENT; 
    }

    // TODO

    return ID3V2_UNSUPPORTED;
}

struct id3v2_text_frame_content* id3v2_allocate_text_frame_content(size_t content_size) {
    struct id3v2_text_frame_content* content = malloc(sizeof(struct id3v2_text_frame_content));
    content->text = malloc(sizeof(char) * (content_size - 1)); // 1 byte for encoding
    return content;
}

// FIXME: Need allocators for each frame content type
// FIXME: Handle all errors in fread
int id3v2_decode_text_frame(FILE* input, struct id3v2_frame* frame) {
    size_t content_size = id3v2_synchsafe_to_uint32(frame->size);
    frame->content = id3v2_allocate_text_frame_content(content_size); 
    struct id3v2_text_frame_content* content = frame->content; 

    fread(&content->encoding, sizeof(char), 1, input);
    fread(content->text, sizeof(char), content_size - 1, input);

    return 0;
}

int id3v2_encode_text_frame(FILE* output, struct id3v2_frame* frame) {
    size_t content_size = id3v2_synchsafe_to_uint32(frame->size);
    struct id3v2_text_frame_content* content = frame->content;

    fwrite(&content->encoding, sizeof(char), 1, output);
    fwrite(content->text, sizeof(char), content_size - 1, output);

    return 0;
}

// TODO: implement different encodings
int id3v2_show_text_frame(FILE* output, struct id3v2_frame* frame) {
    size_t content_size = id3v2_synchsafe_to_uint32(frame->size);
    struct id3v2_text_frame_content* content = frame->content;
    
    fprintf(
        output,
        "id: %.4s\n"
        "size: %u\n"
        "flags: 0b" BYTE_TO_BINARY_PATTERN " 0b" BYTE_TO_BINARY_PATTERN "\n"
        "encoding: %s\n"
        "content: %.*s\n",
        frame->id,
        content_size,
        BYTE_TO_BINARY(frame->flags[0]), BYTE_TO_BINARY(frame->flags[1]),
        id3v2_encoding_to_str(content->encoding),
        content_size - 1, content->text
    );
    
    return 0;
}

void id3v2_free_text_frame_content(void** content) {
    struct id3v2_text_frame_content* text_content = *content; 
    free(text_content->text);
    free(text_content);
    *content = NULL;
}

struct id3v2_url_frame_content* id3v2_allocate_url_frame_content(size_t content_size) {
    struct id3v2_url_frame_content* content = malloc(sizeof(struct id3v2_url_frame_content));
    content->url = malloc(sizeof(char) * content_size);
    return content;
}

int id3v2_decode_url_frame(FILE* input, struct id3v2_frame* frame) {
    size_t text_size = id3v2_synchsafe_to_uint32(frame->size); 
    frame->content = id3v2_allocate_url_frame_content(text_size);
    struct id3v2_url_frame_content* content = frame->content;

    if (fread(content->url, sizeof(char), text_size, input) < text_size) {
        fprintf(stderr, YELLOW "%s(): Failed to read content of frame from file: %s:%d\n" RESET, __func__, __FILE__, __LINE__);
        return 1;
    }
    
    return 0;
}

int id3v2_encode_url_frame(FILE* output, struct id3v2_frame* frame) {
    size_t text_size = id3v2_synchsafe_to_uint32(frame->size);
    struct id3v2_url_frame_content* content = frame->content;

    if (fwrite(content->url, sizeof(char), text_size, output) < text_size) {
        fprintf(stderr, YELLOW "%s(): Failed to write content of frame into file: %s:%d\n" RESET, __func__, __FILE__, __LINE__);
        return 1;
    }

    return 0;   
}

int id3v2_show_url_frame(FILE* output, struct id3v2_frame* frame) {
    size_t text_size = id3v2_synchsafe_to_uint32(frame->size);
    
    fprintf(
        output,
        "id: %.4s\n"
        "size: %u\n"
        "flags: 0b" BYTE_TO_BINARY_PATTERN " 0b" BYTE_TO_BINARY_PATTERN "\n"
        "content: %.*s\n",
        frame->id,
        text_size,
        BYTE_TO_BINARY(frame->flags[0]), BYTE_TO_BINARY(frame->flags[1]),
        text_size, frame->content
    );
    
    return 0;
}

void id3v2_free_url_frame_content(void** content) {
    struct id3v2_url_frame_content* url_content = *content;
    free(url_content->url);
    free(url_content);
    *content = NULL;
}

struct id3v2_comment_frame_content* id3v2_allocate_comment_frame_content() {
    return malloc(sizeof(struct id3v2_comment_frame_content));
}

int id3v2_decode_comment_frame(FILE* input, struct id3v2_frame* frame) {
    size_t header_size = 
        offsetof(struct id3v2_comment_frame_content, language) 
        + member_size(struct id3v2_comment_frame_content, language);

    size_t content_size = id3v2_synchsafe_to_uint32(frame->size) - header_size;
    frame->content = id3v2_allocate_comment_frame_content(); 
    struct id3v2_comment_frame_content* content = frame->content;
    char* buffer = malloc(sizeof(char) * content_size);

    // Read COMM header
    if (fread(content, sizeof(char), header_size, input) < header_size) {
        fprintf(stderr, YELLOW "%s(): Failed to read metadata of COMM frame from file: %s:%d\n" RESET, __func__, __FILE__, __LINE__);
        return 1;
    }

    // Read description and text
    if (fread(buffer, sizeof(char), content_size, input) < content_size) {
        fprintf(stderr, YELLOW "%s(): Failed to read content of COMM frame from file: %s:%d\n" RESET, __func__, __FILE__, __LINE__);
        return 1;
    }

    // Fill description. It is end with \0 according to the format.
    size_t descr_len = strlen(buffer);
    content->description = malloc(sizeof(char) * (descr_len + 1));
    strncpy(content->description, buffer, descr_len);
    content->description[descr_len] = 0;

    // Fill actual text. It isn't end with \0 according to the format.
    size_t text_len = content_size - descr_len - 1; // -1 for \0
    content->text = malloc(sizeof(char) * text_len);
    strncpy(content->text, buffer + descr_len + 1, text_len);

    free(buffer);

    return 0;
}

int id3v2_encode_comment_frame(FILE* output, struct id3v2_frame* frame) {
    size_t header_size = 
        offsetof(struct id3v2_comment_frame_content, language) 
        + member_size(struct id3v2_comment_frame_content, language);

    size_t content_size = id3v2_synchsafe_to_uint32(frame->size) - header_size;
    struct id3v2_comment_frame_content* content = frame->content;

    if (fwrite(content, sizeof(char), header_size, output) < header_size) {
        fprintf(stderr, YELLOW "%s(): Failed to write metadata of COMM frame into file: %s:%d\n" RESET, __func__, __FILE__, __LINE__);
        return 1;
    }

    size_t descr_len = strlen(content->description);
    size_t text_len = content_size - descr_len - 1; // -1 for \0

    char* buffer = malloc(sizeof(char) * content_size);
    // According to the format, \0 at the end is important
    strncpy(buffer, content->description, descr_len);
    strncpy(buffer + descr_len + 1, content->text, text_len);

    if (fwrite(buffer, sizeof(char), content_size, output) < content_size) {
        fprintf(stderr, YELLOW "%s(): Failed to write content of COMM frame into file: %s:%d\n" RESET, __func__, __FILE__, __LINE__);
        return 1;
    }

    free(buffer);

    return 0;
}

int id3v2_show_comment_frame(FILE* output, struct id3v2_frame* frame) {
    struct id3v2_comment_frame_content* content = frame->content;
    
    size_t header_size = 
        offsetof(struct id3v2_comment_frame_content, language) 
        + member_size(struct id3v2_comment_frame_content, language);

    size_t content_size = id3v2_synchsafe_to_uint32(frame->size) - header_size;
    
    fprintf(
        output,
        "id: %.4s\n"
        "size: %u\n"
        "flags: 0b" BYTE_TO_BINARY_PATTERN " 0b" BYTE_TO_BINARY_PATTERN "\n"
        "encoding: %s\n"
        "language: %.3s\n"
        "description: %s\n"
        "text: %.*s\n",
        frame->id,
        content_size + header_size,
        BYTE_TO_BINARY(frame->flags[0]), BYTE_TO_BINARY(frame->flags[1]),
        id3v2_encoding_to_str(content->encoding),
        content->language,
        content->description,
        content_size - strlen(content->description) - 1, content->text
    );
}

void id3v2_free_comment_frame_content(void** content) {
    struct id3v2_comment_frame_content* comment_content = *content;
    free(comment_content->description);
    free(comment_content->text);
    free(comment_content);
    *content = NULL;
}

// Decode one frame from input to frame. Seek input to frame size.
int id3v2_decode_frame(FILE* input, struct id3v2_frame** frame) {
    static int (*frame_decoders[])(FILE*, struct id3v2_frame*) = {
        id3v2_decode_text_frame,            // ID3V2_TEXT
        id3v2_decode_url_frame,             // ID3V2_URL
        id3v2_decode_comment_frame,         // ID3V2_COMMENT 
    };
    
    *frame = id3v2_allocate_frame();

    // Read frame header 
    if (fread(*frame, offsetof(struct id3v2_frame, flags) + member_size(struct id3v2_frame, flags), 1, input) < 1) {
        fprintf(stderr, YELLOW "%s(): Failed to read header of frame from file: %s:%d\n" RESET, __func__, __FILE__, __LINE__);
        return 1;
    }

    enum id3v2_frame_type type = id3v2_get_frame_type((*frame)->id);

    if (type == ID3V2_UNSUPPORTED) {
        fprintf(stderr, YELLOW "%s(): Can not decode unsupported type frame (%.4s): %s:%d\n" RESET, __func__, (*frame)->id, __FILE__, __LINE__);
        fseek(input, id3v2_synchsafe_to_uint32((*frame)->size), SEEK_CUR); // Skip unsupported frame
        return 1;
    }

    return frame_decoders[type](input, *frame);
}

int id3v2_encode_frame(FILE* output, struct id3v2_frame* frame) {
    static int (*frame_encoders[])(FILE* output, struct id3v2_frame*) = {
        id3v2_encode_text_frame,        // ID3V2_TEXT
        id3v2_encode_url_frame,         // ID3V2_URL
        id3v2_encode_comment_frame,     // ID3V2_COMMENT
    };
    
    // Write frame header
    if (fwrite(frame, offsetof(struct id3v2_frame, flags) + member_size(struct id3v2_frame, flags), 1, output) < 1) {
        fprintf(stderr, YELLOW "%s(): Failed to write header of frame into file: %s:%d\n" RESET, __func__, __FILE__, __LINE__);
        return 1;
    }
    
    enum id3v2_frame_type type = id3v2_get_frame_type(frame->id);

    if (type == ID3V2_UNSUPPORTED) {
        fprintf(stderr, YELLOW "%s(): Can not encode unsupported frame type %.4s: %s:%d" RESET, __func__, frame->id, __FILE__, __LINE__);
        return 1;
    }

    return frame_encoders[type](output, frame);
}

int id3v2_show_frame(FILE* output, struct id3v2_frame* frame) {
    static int (*frame_printers[])(FILE*, struct id3v2_frame*) = {
        id3v2_show_text_frame,              // ID3V2_TEXT
        id3v2_show_url_frame,               // ID3V2_URL
        id3v2_show_comment_frame,           // ID3V2_COMMENT
    };

    enum id3v2_frame_type type = id3v2_get_frame_type(frame->id);
    
    if (type == ID3V2_UNSUPPORTED) {
        fprintf(stderr, YELLOW "%s(): Can not show unsupported frame type (%.4s): %s:%d\n" RESET, __func__, frame->id, __FILE__, __LINE__);
        return 1;
    }
    
    return frame_printers[type](output, frame);
}

// TODO: Need normal free since each frame has his own structure content
// Free frame. Set frame pointer to NULL.
void id3v2_free_frame(struct id3v2_frame** frame) {
    static void (*content_destructors[])(void**) = {
        id3v2_free_text_frame_content,      // ID3V2_TEXT
        id3v2_free_url_frame_content,       // ID3V2_URL
        id3v2_free_comment_frame_content,   // ID3V2_COMMENT
    };

    enum id3v2_frame_type type = id3v2_get_frame_type((*frame)->id);

    if (type < ID3V2_UNSUPPORTED) {
        content_destructors[type](&(*frame)->content);
    }

    free(*frame);
    *frame = NULL;
}

// Read tag information from input and decode it into tag.
// Allocate memory for tag via malloc, so free_tag call is needed.
// Return error code (0 in success case).
int id3v2_decode_tag(FILE* input, struct id3v2_tag** tag) {
    *tag = (struct id3v2_tag*)malloc(sizeof(struct id3v2_tag));

    if (id3v2_decode_header(input, &(*tag)->header) != 0) {
        fprintf(stderr, YELLOW "%s(): Failed to decode header: %s:%d\n" RESET, __func__, __FILE__, __LINE__);
        return 1;
    }

    if ((*tag)->header.flags & EXHEADER) {
        if (id3v2_decode_exheader(input, &(*tag)->exheader) != 0) {
            fprintf(stderr, YELLOW "%s(): Failed to decode extended header: %s:%d\n" RESET, __func__, __FILE__, __LINE__);
            return 1;
        }
    }

    size_t frames_size = 0;
    size_t tag_size = id3v2_synchsafe_to_uint32((*tag)->header.size);
    struct id3v2_frame** current_frame = &(*tag)->frames_head;

    // TODO: Check validity of while condition    
    while (frames_size < tag_size) {
        if (id3v2_decode_frame(input, current_frame) != 0) {
            fprintf(stderr, YELLOW "%s(): Failed to decode frame: %s:%d\n" RESET, __func__, __FILE__, __LINE__);
        }

        frames_size += id3v2_synchsafe_to_uint32((*current_frame)->size);
        current_frame = &(*current_frame)->next;
    }

    // TODO: Check pading and footer

    return 0;
}

// Write raw tag information to output.
// Return error code (0 in success case).
int id3v2_encode_tag(FILE* output, struct id3v2_tag* tag) {
    if (id3v2_encode_header(output, &tag->header) != 0) {
        fprintf(stderr, YELLOW "%s(): Failed to encode header: %s:%d\n" RESET, __func__, __FILE__, __LINE__);
        return 1;
    }

    if (id3v2_encode_exheader(output, tag->exheader) != 0) {
        fprintf(stderr, YELLOW "%s(): Failed to encode exheader: %s:%d\n" RESET, __func__, __FILE__, __LINE__);
        return 1;
    }
    
    struct id3v2_frame* current_frame = tag->frames_head;
    
    while (current_frame) {
        if (id3v2_encode_frame(output, current_frame) != 0) {
            fprintf(stderr, YELLOW "%s(): Failed to encode frame: %s:%d\n" RESET, __func__, __FILE__, __LINE__);
            return 1;
        }

        current_frame = current_frame->next;
    }

    // TODO: Check padding and footer

    return 0;
}

// Write human-readable tag information to output.
// Return error code (0 in success case).
int id3v2_show_tag(FILE* output, struct id3v2_tag* tag) {
     if (id3v2_show_header(output, &tag->header) != 0) {
        fprintf(stderr, YELLOW "%s(): Failed to show header: %s:%d\n" RESET, __func__, __FILE__, __LINE__);
        return 1;
    }

    if (id3v2_show_exheader(output, tag->exheader) != 0) {
        fprintf(stderr, YELLOW "%s(): Failed to show exheader: %s:%d\n" RESET, __func__, __FILE__, __LINE__);
        return 1;
    }
    
    struct id3v2_frame* current_frame = tag->frames_head;
    
    while (current_frame) {
        if (id3v2_show_frame(output, current_frame) != 0) {
            fprintf(stderr, YELLOW "%s(): Failed to show frame: %s:%d\n" RESET, __func__, __FILE__, __LINE__);
        }

        current_frame = current_frame->next;
    }

    return 0;
}

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
void id3v2_free_tag(struct id3v2_tag** tag) {
    if (tag == NULL || *tag == NULL)
        return;
        
    id3v2_free_exheader(&(*tag)->exheader);
    
    struct id3v2_frame** current_frame = &(*tag)->frames_head;

    while (*current_frame) {
        struct id3v2_frame** next_frame = &(*current_frame)->next;
        id3v2_free_frame(current_frame);
        current_frame = next_frame;
    }
    
    *tag = NULL;

    // TODO: Check padding and footer
}