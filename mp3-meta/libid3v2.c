// Audio meta tag parser. See https://id3.org/id3v2.4.0-structure document
// to understand topic and comments style agreement.

#include <libid3v2.h>

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

uint32_t id3v2_synchsafe_to_uint32(id3v2_synchsafe32_t synchsafe) {
    uint32_t byte0 = synchsafe.inner[0];
    uint32_t byte1 = synchsafe.inner[1];
    uint32_t byte2 = synchsafe.inner[2];
    uint32_t byte3 = synchsafe.inner[3];

    return byte0 << 21 | byte1 << 14 | byte2 << 7 | byte3;
}

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

// TODO: update doc
struct id3v2_exheader {
    id3v2_synchsafe32_t size;   // Size as synchsafe 32 bit integer.

    uint8_t flag_size;          // Number of flag bytes = $01
                                // so size is 1 byte for each flag

    uint8_t exflags;            // Extended Flags in following format: $xx 

    void* flags_info;
};

struct id3v2_exheader* id3v2_allocate_exheader() {
    struct id3v2_exheader* exheader = (struct id3v2_exheader*)malloc(sizeof(struct id3v2_exheader));
    exheader->flags_info = NULL; 
    return exheader;
}

// Decode extender header from input to exheader. 
// Allocate exheader via malloc. Seek input to extended header size.
int id3v2_decode_exheader(FILE* input, struct id3v2_exheader** exheader) {
    fprintf(stderr, "unimplemented %s:%d\n", __FILE__, __LINE__);
    return 1;
}

int id3v2_encode_exheader(FILE* ouput, struct id3v2_exheader* exheader) {
    fprintf(stderr, "unimplemented %s:%d\n", __FILE__, __LINE__);
    return 1;
}

int id3v2_show_exheader(FILE* output, struct id3v2_exheader* exheader) {
    fprintf(stderr, "unimplemented %s:%d\n", __FILE__, __LINE__);
    return 1;
}

// Free extended header. Set exheader pointer to NULL.
void id3v2_free_exheader(struct id3v2_exheader** exheader) {
    fprintf(stderr, "unimplemented %s:%d\n", __FILE__, __LINE__);
    return;
}

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

    // TODO

    return -1;
}

int id3v2_decode_text_frame(FILE* input, struct id3v2_frame** frame) {
    *frame = id3v2_allocate_frame();

    // Read frame header 
    if (fread(*frame, offsetof(struct id3v2_frame, flags) + member_size(struct id3v2_frame, flags), 1, input) < 1) {
        fprintf(stderr, "Failed to read header of frame from file\n");
        return 1;
    }
    
    size_t text_size = id3v2_synchsafe_to_uint32((*frame)->size); 
    (*frame)->content = (char*)malloc(text_size);
    
    if (fread((*frame)->content, sizeof(char), text_size, input) < text_size) {
        fprintf(stderr, "Failed to read content of frame from file\n");
        return 1;
    }
    
    return 0;
}

int id3v2_encode_text_frame(FILE* output, struct id3v2_frame* frame) {
    // Write frame header
    if (fwrite(frame, offsetof(struct id3v2_frame, flags) + member_size(struct id3v2_frame, flags), 1, output) < 1) {
        fprintf(stderr, "Failed to write header of frame into file\n");
        return 1;
    }
    
    size_t text_size = id3v2_synchsafe_to_uint32(frame->size);

    if (fwrite(frame->content, sizeof(char), text_size, output) < text_size) {
        fprintf(stderr, "Failed to write content of frame into file\n");
        return 1;
    }

    return 0;
}

// TODO: implement different encodings
int id3v2_show_text_frame(FILE* output, struct id3v2_frame* frame) {
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

// Decode one frame from input to frame. Seek input to frame size.
int id3v2_decode_frame(FILE* input, struct id3v2_frame** frame) {
    static int (*frame_decoders[])(FILE*, struct id3v2_frame**) = {
        id3v2_decode_text_frame // ID3V2_TEXT
    };
    
    // Read id and return SEEK before id
    char id[4];
    fread(id, sizeof(id), 1, input);
    fseek(input, -sizeof(id), SEEK_CUR);

    enum id3v2_frame_type type = id3v2_get_frame_type(id);
    return type > ID3V2_TEXT || frame_decoders[type](input, frame);
}

int id3v2_encode_frame(FILE* output, struct id3v2_frame* frame) {
    fprintf(stderr, "unimplemented %s:%d\n", __FILE__, __LINE__);
    return 1;
}

int id3v2_show_frame(FILE* output, struct id3v2_frame* frame) {
    static int (*frame_printers[])(FILE*, struct id3v2_frame*) = {
        id3v2_show_text_frame // ID3V2_TEXT
    };

    enum id3v2_frame_type type = id3v2_get_frame_type(frame->id);
    return frame_printers[type](output, frame);
}

// Free frame. Set exheader pointer to NULL.
void id3v2_free_frame(struct id3v2_frame** frame) {
    free((*frame)->content);
    free(*frame);
    *frame = NULL;
}

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
int id3v2_decode_tag(FILE* input, struct id3v2_tag** tag) {
    *tag = (struct id3v2_tag*)malloc(sizeof(struct id3v2_tag));

    if (id3v2_decode_header(input, &(*tag)->header) != 0) {
        fprintf(stderr, "Failed to decode header\n");
        return 1;
    }

    if ((*tag)->header.flags & EXHEADER) {
        if (id3v2_decode_exheader(input, &(*tag)->exheader) != 0) {
            fprintf(stderr, "Failed to decode extended header\n");
            return 1;
        }
    }

    size_t frames_size = 0;
    size_t tag_size = id3v2_synchsafe_to_uint32((*tag)->header.size);
    struct id3v2_frame** current_frame = &(*tag)->frames_head;

    // TODO: Check validity of while condition    
    while (frames_size < tag_size) {
        if (id3v2_decode_frame(input, current_frame) != 0) {
            fprintf(stderr, "Failed to decode frame\n");
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
        fprintf(stderr, "Failed to encode header\n");
        return 1;
    }

    if (id3v2_encode_exheader(output, tag->exheader) != 0) {
        fprintf(stderr, "Failed to encode exheader\n");
        return 1;
    }
    
    struct id3v2_frame* current_frame = tag->frames_head;
    
    while (current_frame) {
        if (id3v2_encode_frame(output, current_frame) != 0) {
            fprintf(stderr, "Failed to encode frame\n");
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
        fprintf(stderr, "Failed to show header\n");
        return 1;
    }

    if (id3v2_show_exheader(output, tag->exheader) != 0) {
        fprintf(stderr, "Failed to show exheader\n");
        return 1;
    }
    
    struct id3v2_frame* current_frame = tag->frames_head;
    
    while (current_frame) {
        if (id3v2_show_frame(output, current_frame) != 0) {
            fprintf(stderr, "Failed to show frame\n");
            return 1;
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