// Audio meta tag parser. See https://id3.org/id3v2.4.0-structure document
// to understand topic and comments style agreement.

#include "libid3v2.h"

//#define DEBUG 

#define BUF_SIZE (1 << 20)

#define SET_ARG         "--set="
#define VALUE_ARG       "--value="
#define GET_ARG         "--get="
#define SHOW_ARG        "--show"
#define FILEPATH_ARG    "--filepath="

struct id3v2_frame* unsupported_frame_converter(const char* id, const char* str) {
    return NULL;
}

struct id3v2_frame* text_frame_converter(const char* id, const char* str) {
    size_t text_size = strlen(str);
    struct id3v2_frame* frame = id3v2_allocate_frame();
    frame->content = id3v2_allocate_text_frame_content(text_size + 1);
    
    memmove(frame->id, id, 4);
    frame->size = id3v2_uint32_to_synchsafe(text_size + 1);
    memset(frame->flags, 0, sizeof(frame->flags));

    struct id3v2_text_frame_content* content = frame->content;
    content->encoding = ID3V2_UTF8;
    memmove(content->text, str, text_size);

    return frame;
}

struct id3v2_frame* url_frame_converter(const char* id, const char* str) {
    size_t url_size = strlen(str);
    struct id3v2_frame* frame = id3v2_allocate_frame();
    frame->content = id3v2_allocate_url_frame_content(url_size);

    memmove(frame->id, id, 4);
    frame->size = id3v2_uint32_to_synchsafe(url_size);
    memset(frame->flags, 0, sizeof(frame->flags));

    struct id3v2_url_frame_content* content = frame->content;
    memmove(content->url, str, url_size);

    return frame;
}

struct id3v2_frame* comment_frame_converter(const char* id, const char* str) {
    return NULL;
}

void rewrite_mp3(struct id3v2_tag* tag, FILE* from_mp3) {
    FILE* tmp = tmpfile();
    id3v2_encode_tag(tmp, tag);
    char buffer[BUF_SIZE]; 
    size_t read;

    while ((read = fread(buffer, sizeof(char), sizeof(buffer), from_mp3)) != 0) {
        fwrite(buffer, sizeof(char), read, tmp);
    }

    fseek(from_mp3, 0, SEEK_SET);
    fseek(tmp, 0, SEEK_SET);

    while ((read = fread(buffer, sizeof(char), sizeof(buffer), tmp)) != 0) {
        fwrite(buffer, sizeof(char), read, from_mp3);
    }

    fclose(tmp);
}

// TODO: Make handle of arguments in loop, so it will make this usage possiable
// ./mp3-meta --set=TAA1 --value=VAL1 --set=TAA2 --value=VAL2
int main(int argc, char* argv[]) {
    FILE*   file        = NULL;
    char*   set_prop    = NULL;
    char*   set_value   = NULL;
    char*   get_prop    = NULL;
    int     show_flag   = 0;

    static struct id3v2_frame* (*frame_converters[])(const char*, const char*) = {
        unsupported_frame_converter,
        text_frame_converter,
        url_frame_converter,
        comment_frame_converter,
    };

    for (int i = 0; i < argc; i++) {
        if (strncmp(argv[i], FILEPATH_ARG, strlen(FILEPATH_ARG)) == 0) {
            file = fopen(argv[i] + strlen(FILEPATH_ARG), "rwb+");

            if (!file) {
                fprintf(stderr, "Cannot open file with filename = %s\n", argv[i] + strlen(FILEPATH_ARG));
                return 0;
            }
        }
        
        if (strncmp(argv[i], SHOW_ARG, strlen(SHOW_ARG)) == 0) {
            show_flag = 1;
        }

        if (strncmp(argv[i], SET_ARG, strlen(SET_ARG)) == 0) {
            set_prop = argv[i] + strlen(SET_ARG);
        }

        if (strncmp(argv[i], VALUE_ARG, strlen(VALUE_ARG)) == 0) {
            set_value = argv[i] + strlen(VALUE_ARG);
        }

        if (strncmp(argv[i], GET_ARG, strlen(GET_ARG)) == 0) {
            get_prop = argv[i] + strlen(GET_ARG);
        }
    }

    if (!file) {
        fprintf(stderr, "Don't know file to parse\n");
        return 0;
    }

    struct id3v2_tag* tag = NULL;

#ifdef DEBUG
    id3v2_decode_tag(file, &tag);
    id3v2_show_tag(fopen("log.txt", "w"), tag);
   
    goto CLEANUP;
#endif

    if (id3v2_decode_tag(file, &tag) != 0) {
        fprintf(stderr, "Failed to decode file\n");
        goto CLEANUP;
    }

    if (show_flag) {
        if (id3v2_show_tag(stdout, tag) != 0) {
            fprintf(stderr, "Failed to write output in file\n");
        }

        goto CLEANUP;
    }

    if (set_prop && set_value) {
        struct id3v2_frame* frame = id3v2_find_first(tag->frames_head, set_prop);
        struct id3v2_frame* prev = NULL;

        while (frame) {
            id3v2_show_frame(stdout, frame);
            printf("Do you want to change it? ((y|Y)/(n|N))\n");
            char answer = getchar();

            if (answer == 'y' || answer == 'Y') {
                struct id3v2_frame* value = frame_converters[id3v2_get_frame_type(set_prop)](set_prop, set_value);
                value->next = frame->next;
                
                tag->header.size = id3v2_uint32_to_synchsafe(
                    id3v2_synchsafe_to_uint32(tag->header.size)
                    + id3v2_synchsafe_to_uint32(value->size)
                    - id3v2_synchsafe_to_uint32(frame->size)
                );

                if (prev) {
                    prev->next = value;
                } else {
                    tag->frames_head = value;
                }

                id3v2_free_frame(&frame);
                rewrite_mp3(tag, file);
                goto CLEANUP;
            }

            prev = frame;
            frame = id3v2_find_first(frame->next, set_prop);
        }

        struct id3v2_frame* value = frame_converters[id3v2_get_frame_type(set_prop)](set_prop, set_value);
        value->next = tag->frames_head;
        tag->frames_head = value;
        tag->header.size = id3v2_uint32_to_synchsafe(
            id3v2_synchsafe_to_uint32(tag->header.size)
            + id3v2_synchsafe_to_uint32(value->size)
            + 10
        );

        rewrite_mp3(tag, file);
        goto CLEANUP;
    }

    if (get_prop) {
        struct id3v2_frame* frame = id3v2_find_first(tag->frames_head, get_prop);

        if (frame == NULL) {
            printf("Didn't find any tag with given prop\n");
        }

        while (frame) {
            id3v2_show_frame(stdout, frame);
            frame = id3v2_find_first(frame->next, get_prop);
        }

        goto CLEANUP;
    }

CLEANUP:
    fclose(file);
    id3v2_free_tag(&tag);

    return 0;
}
