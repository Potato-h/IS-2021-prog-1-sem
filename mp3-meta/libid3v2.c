// Audio meta tag parser. See https://id3.org/id3v2.4.0-structure document
// to understand topic and comments style agreement.

#include "libid3v2.h"

// TODO: Think about right API for show functions (return string instead of write into file)
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

// TODO:
#define log(format, ...)            fprintf(stderr, "%s(): " format ": %s:%d\n", __func__, ##__VA_ARGS__, __FILE__, __LINE__)
#define log_warning(format, ...)    fprintf(stderr, YELLOW "%s(): " format ": %s:%d\n" RESET, __func__, ##__VA_ARGS__, __FILE__, __LINE__)
#define log_error(format, ...)      fprintf(stderr, RED "%s(): " format ": %s:%d\n" RESET, __func__, ##__VA_ARGS__, __FILE__, __LINE__)

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

    log_warning("unimplemented");
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
// Decode url frame. Seek input to frame size.
int id3v2_decode_text_frame(FILE* input, struct id3v2_frame* frame) {
    size_t content_size = id3v2_synchsafe_to_uint32(frame->size);
    frame->content = id3v2_allocate_text_frame_content(content_size); 
    struct id3v2_text_frame_content* content = frame->content; 

    fread(&content->encoding, sizeof(char), 1, input);
    fread(content->text, sizeof(char), content_size - 1, input);

    return 0;
}

// Encode text frame into output.
int id3v2_encode_text_frame(FILE* output, struct id3v2_frame* frame) {
    size_t content_size = id3v2_synchsafe_to_uint32(frame->size);
    struct id3v2_text_frame_content* content = frame->content;

    fwrite(&content->encoding, sizeof(char), 1, output);
    fwrite(content->text, sizeof(char), content_size - 1, output);

    return 0;
}

// TODO: implement different encodings
// Print human readable information about text frame into output.
int id3v2_show_text_frame(FILE* output, struct id3v2_frame* frame) {
    size_t content_size = id3v2_synchsafe_to_uint32(frame->size);
    struct id3v2_text_frame_content* content = frame->content;
    
    fprintf(
        output,
        "id: %.4s\n"
        "size: %lu\n"
        "flags: 0b" BYTE_TO_BINARY_PATTERN " 0b" BYTE_TO_BINARY_PATTERN "\n"
        "encoding: %s\n"
        "content: %.*s\n",
        frame->id,
        content_size,
        BYTE_TO_BINARY(frame->flags[0]), BYTE_TO_BINARY(frame->flags[1]),
        id3v2_encoding_to_str(content->encoding),
        (int)(content_size - 1), content->text
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

// Decode url frame. Seek input to frame size.
int id3v2_decode_url_frame(FILE* input, struct id3v2_frame* frame) {
    size_t text_size = id3v2_synchsafe_to_uint32(frame->size); 
    frame->content = id3v2_allocate_url_frame_content(text_size);
    struct id3v2_url_frame_content* content = frame->content;

    if (fread(content->url, sizeof(char), text_size, input) < text_size) {
        log_error("Failed to read content of frame from file");
        return 1;
    }
    
    return 0;
}

// Encode url frame into output.
int id3v2_encode_url_frame(FILE* output, struct id3v2_frame* frame) {
    size_t text_size = id3v2_synchsafe_to_uint32(frame->size);
    struct id3v2_url_frame_content* content = frame->content;

    if (fwrite(content->url, sizeof(char), text_size, output) < text_size) {
        log_error("Failed to write content of frame into file");
        return 1;
    }

    return 0;   
}

// Print human readable information about url frame into output.
int id3v2_show_url_frame(FILE* output, struct id3v2_frame* frame) {
    size_t text_size = id3v2_synchsafe_to_uint32(frame->size);
    struct id3v2_url_frame_content* content = frame->content;

    fprintf(
        output,
        "id: %.4s\n"
        "size: %lu\n"
        "flags: 0b" BYTE_TO_BINARY_PATTERN " 0b" BYTE_TO_BINARY_PATTERN "\n"
        "content: %.*s\n",
        frame->id,
        text_size,
        BYTE_TO_BINARY(frame->flags[0]), BYTE_TO_BINARY(frame->flags[1]),
        (int)text_size, content->url
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

// Decode comment frame. Seek input to frame size.
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
        log_error("Failed to read metadata of COMM frame from file");
        return 1;
    }

    // Read description and text
    if (fread(buffer, sizeof(char), content_size, input) < content_size) {
        log_error("Failed to read content of COMM frame from file");
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

// Encode comment frame into output.
int id3v2_encode_comment_frame(FILE* output, struct id3v2_frame* frame) {
    size_t header_size = 
        offsetof(struct id3v2_comment_frame_content, language) 
        + member_size(struct id3v2_comment_frame_content, language);

    size_t content_size = id3v2_synchsafe_to_uint32(frame->size) - header_size;
    struct id3v2_comment_frame_content* content = frame->content;

    if (fwrite(content, sizeof(char), header_size, output) < header_size) {
        log_error("Failed to write metadata of COMM frame into file");
        return 1;
    }

    size_t descr_len = strlen(content->description);
    size_t text_len = content_size - descr_len - 1; // -1 for \0

    char* buffer = malloc(sizeof(char) * content_size);
    // According to the format, \0 at the end is important
    strncpy(buffer, content->description, descr_len);
    strncpy(buffer + descr_len + 1, content->text, text_len);

    if (fwrite(buffer, sizeof(char), content_size, output) < content_size) {
        log_error("Failed to write content of COMM frame into file");
        return 1;
    }

    free(buffer);

    return 0;
}

// Print human readable information about comment frame into output.
int id3v2_show_comment_frame(FILE* output, struct id3v2_frame* frame) {
    struct id3v2_comment_frame_content* content = frame->content;
    
    size_t header_size = 
        offsetof(struct id3v2_comment_frame_content, language) 
        + member_size(struct id3v2_comment_frame_content, language);

    size_t content_size = id3v2_synchsafe_to_uint32(frame->size) - header_size;
    
    fprintf(
        output,
        "id: %.4s\n"
        "size: %lu\n"
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
        (int)(content_size - strlen(content->description) - 1), content->text
    );

    return 0;
}

void id3v2_free_comment_frame_content(void** content) {
    struct id3v2_comment_frame_content* comment_content = *content;
    free(comment_content->description);
    free(comment_content->text);
    free(comment_content);
    *content = NULL;
}

// Yes, it's just a malloc wrapper, but it make code more general
void* id3v2_allocate_unsupported_frame_content(size_t content_size) {
    return malloc(content_size);
}

// Decode unsupported frame from input. Seek input to frame size.
int id3v2_decode_unsupported_frame(FILE* input, struct id3v2_frame* frame) {
    size_t content_size = id3v2_synchsafe_to_uint32(frame->size);
    frame->content = id3v2_allocate_unsupported_frame_content(content_size);

    if (fread(frame->content, sizeof(char), content_size, input) < content_size) {
        log_error("Failed to read unsupported frame content");
        return 1;
    }
    
    return 0;
}

// Encode unsupported frame into output.
int id3v2_encode_unsupported_frame(FILE* output, struct id3v2_frame* frame) {
    size_t content_size = id3v2_synchsafe_to_uint32(frame->size);
    
    if (fwrite(frame->content, sizeof(char), content_size, output) < content_size) {
        log_error("Failed to write unsupported frame content");
        return 1;
    }
    
    return 0;
}

// Print human readable information about unsupported frame into output.
int id3v2_show_unsupported_frame(FILE* output, struct id3v2_frame* frame) {
    size_t content_size = id3v2_synchsafe_to_uint32(frame->size);

    fprintf(
        output,
        "id: %.4s\n"
        "size: %lu\n"
        "flags: 0b" BYTE_TO_BINARY_PATTERN " 0b" BYTE_TO_BINARY_PATTERN "\n"
        "Unknow content: %.*s\n",
        frame->id,
        content_size,
        BYTE_TO_BINARY(frame->flags[0]), BYTE_TO_BINARY(frame->flags[1]),
        (int)content_size, (char*)frame->content
    );
    
    return 0;
}

void id3v2_free_unsupported_free_content(void** content) {
    free(*content);
    *content = NULL;
}

// Decode one frame from input to frame. Seek input to frame size.
int id3v2_decode_frame(FILE* input, struct id3v2_frame** frame) {
    static int (*frame_decoders[])(FILE*, struct id3v2_frame*) = {
        id3v2_decode_text_frame,                // ID3V2_TEXT
        id3v2_decode_url_frame,                 // ID3V2_URL
        id3v2_decode_comment_frame,             // ID3V2_COMMENT 
        id3v2_decode_unsupported_frame,         // ID3V2_UNSUPPORTED
    };
    
    *frame = id3v2_allocate_frame();

    // Read frame header 
    if (fread(*frame, offsetof(struct id3v2_frame, flags) + member_size(struct id3v2_frame, flags), 1, input) < 1) {
        log_error("Failed to read header of frame from file");
        return 1;
    }

    if (strncmp((*frame)->id, "\0\0\0\0", 4) == 0) {
        // Padding has been detected
        return 1;
    }

    enum id3v2_frame_type type = id3v2_get_frame_type((*frame)->id);

    if (type == ID3V2_UNSUPPORTED) {
        log_warning("Try decode unsupported type frame (%.4s)", (*frame)->id);
    }

    return frame_decoders[type](input, *frame);
}

// Encode generic frame into output.
int id3v2_encode_frame(FILE* output, struct id3v2_frame* frame) {
    static int (*frame_encoders[])(FILE* output, struct id3v2_frame*) = {
        id3v2_encode_text_frame,                // ID3V2_TEXT
        id3v2_encode_url_frame,                 // ID3V2_URL
        id3v2_encode_comment_frame,             // ID3V2_COMMENT
        id3v2_encode_unsupported_frame,         // ID3V2_UNSUPPORTED
    };
    
    // Write frame header
    if (fwrite(frame, offsetof(struct id3v2_frame, flags) + member_size(struct id3v2_frame, flags), 1, output) < 1) {
        log_error("Failed to write header of frame into file");
        return 1;
    }
    
    enum id3v2_frame_type type = id3v2_get_frame_type(frame->id);

    if (type == ID3V2_UNSUPPORTED) {
        log_warning("Try encode unsupported frame type %.4s", frame->id);
    }

    return frame_encoders[type](output, frame);
}

// Print generic frame data in human readble format into output.
int id3v2_show_frame(FILE* output, struct id3v2_frame* frame) {
    static int (*frame_printers[])(FILE*, struct id3v2_frame*) = {
        id3v2_show_text_frame,                  // ID3V2_TEXT
        id3v2_show_url_frame,                   // ID3V2_URL
        id3v2_show_comment_frame,               // ID3V2_COMMENT
        id3v2_show_unsupported_frame,           // ID3V2_UNSUPPORTED
    };

    enum id3v2_frame_type type = id3v2_get_frame_type(frame->id);
    
    if (type == ID3V2_UNSUPPORTED) {
        log_warning("Try show unsupported frame type (%.4s)", frame->id);
    }
    
    return frame_printers[type](output, frame);
}

// Free frame. Set frame pointer to NULL.
void id3v2_free_frame(struct id3v2_frame** frame) {
    static void (*content_destructors[])(void**) = {
        id3v2_free_text_frame_content,          // ID3V2_TEXT
        id3v2_free_url_frame_content,           // ID3V2_URL
        id3v2_free_comment_frame_content,       // ID3V2_COMMENT
        id3v2_free_unsupported_free_content,    // ID3V2_UNSUPPORTED
    };

    enum id3v2_frame_type type = id3v2_get_frame_type((*frame)->id);

    content_destructors[type](&(*frame)->content);    
    free(*frame);
    *frame = NULL;
}

// Read tag information from input and decode it into tag.
// Allocate memory for tag via malloc, so free_tag call is needed.
// Return error code (0 in success case).
int id3v2_decode_tag(FILE* input, struct id3v2_tag** tag) {
    *tag = (struct id3v2_tag*)malloc(sizeof(struct id3v2_tag));

    if (id3v2_decode_header(input, &(*tag)->header) != 0) {
        log_error("Failed to decode header");
        return 1;
    }

    if ((*tag)->header.flags & EXHEADER) {
        if (id3v2_decode_exheader(input, &(*tag)->exheader) != 0) {
            log_error("Failed to decode extended header");
            return 1;
        }
    }

    size_t frames_size = 0;
    size_t tag_size = id3v2_synchsafe_to_uint32((*tag)->header.size);
    struct id3v2_frame** current_frame = &(*tag)->frames_head;

    // TODO: Check validity of while condition    
    while (frames_size < tag_size) {
        if (id3v2_decode_frame(input, current_frame) != 0) {
            log_error("Failed to decode frame");
            return 1;
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
        log_error("Failed to encode header");
        return 1;
    }

    if (id3v2_encode_exheader(output, tag->exheader) != 0) {
        log_error("Failed to encode exheader");
        return 1;
    }
    
    struct id3v2_frame* current_frame = tag->frames_head;
    
    while (current_frame) {
        if (id3v2_encode_frame(output, current_frame) != 0) {
            log_error("Failed to encode frame");
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
        log_error("Failed to show header");
        return 1;
    }

    if (id3v2_show_exheader(output, tag->exheader) != 0) {
        log_error("Failed to show exheader");
        return 1;
    }
    
    struct id3v2_frame* current_frame = tag->frames_head;
    
    while (current_frame) {
        if (id3v2_show_frame(output, current_frame) != 0) {
            log_error("Failed to show frame");
        }

        current_frame = current_frame->next;
    }

    return 0;
}

// Get prop frame information.
// Return information from get_prop frame. 
// Result can be NULL.
struct id3v2_frame* id3v2_get(struct id3v2_tag* tag, char* id) {
    struct id3v2_frame* current = tag->frames_head;
    const size_t id_len = 4;

    while (current) {
        if (strncmp(current->id, id, id_len) == 0)
            return current;
        
        current++;
    }

    return NULL;
}

// TODO: Is this right signature? For example, we can set some T*** tag
// but we need to know (?) encoding to do this. Is encoding is part of value 
// or there is should be special variable or encoding is always UTF-8 
// (which will be very strange). 
//
// Set prop frame to value.
// Return error code (0 in success case).
int id3v2_set(struct id3v2_tag* tag, struct id3v2_frame* frame) {
    struct id3v2_frame* candidate = id3v2_get(tag, frame->id);

    if (candidate) {
        // TODO: IT DOESN'T EXIST YET 
        // id3v2_free_frame_content(candidate);
        memmove(&candidate->size, &frame->size, member_size(struct id3v2_frame, size));
        memmove(&candidate->flags, &frame->flags, member_size(struct id3v2_frame, flags));
        memmove(&candidate->content, &frame->content, member_size(struct id3v2_frame, content));
        return 0;
    }

    frame->next = tag->frames_head;
    tag->frames_head = frame;

    return 0;
}

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