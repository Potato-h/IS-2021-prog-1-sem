// Audio meta tag parser. See https://id3.org/id3v2.4.0-structure document
// to understand topic and comments style agreement.

#include "libid3v2.h"

#define DEBUG 

#define SET_ARG         "--set="
#define VALUE_ARG       "--value="
#define GET_ARG         "--get="
#define SHOW_ARG        "--show"
#define FILEPATH_ARG    "--filepath="

// TODO: Make handle of arguments in loop, so it will make this usage possiable
// ./mp3-meta --set=TAA1 --value=VAL1 --set=TAA2 --value=VAL2
int main(int argc, char* argv[]) {
    FILE*   file        = NULL;
    char*   set_prop    = NULL;
    char*   set_value   = NULL;
    char*   get_prop    = NULL;
    int     show_flag   = 0;

    for (int i = 0; i < argc; i++) {
        if (strncmp(argv[i], FILEPATH_ARG, strlen(FILEPATH_ARG)) == 0) {
            file = fopen(argv[i] + strlen(FILEPATH_ARG), "rwb");

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

    struct id3v2_tag* tag;

#ifdef DEBUG
    id3v2_decode_tag(file, &tag);
    id3v2_show_tag(stdout, tag);
    id3v2_free_tag(&tag);

    return 0;
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
        if (id3v2_set(tag, set_prop, set_value) != 0) {
            fprintf(stderr, "Failed set property = %s to value = %s\n", set_prop, set_value);
        }

        if (id3v2_encode_tag(file, tag) != 0) {
            fprintf(stderr, "Failed write tag into file\n");
        }

        goto CLEANUP;
    }

    if (get_prop) {
        char* get_val = id3v2_get(tag, get_prop);

        if (get_val) {
            printf("%s\n", get_val);
        }
        else {
            fprintf(stderr, "Property name not found\n");
        }

        goto CLEANUP;
    }

CLEANUP:
    fclose(file);
    id3v2_free_tag(&tag);

    return 0;
}
