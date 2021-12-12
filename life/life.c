#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "libbmp.h"
#include "liblife.h"

#define INPUT_ARG           "--input"
#define OUTPUT_ARG          "--output"
#define MAX_ITER_ARG        "--max_iter"
#define DUMP_FREQ_ARG       "--dump_freq"

// Cross platform way to create directory
#if defined(WIN32) || defined(__WIN32) || defined(__WIN32__)
#include <windows.h>

#define mkdir(dir, mode) _mkdir(dir)
#endif

#if defined(__linux__)
#include <sys/stat.h>
#include <sys/types.h>
#endif

// There is some problem with label mark
int main(int argc, char* argv[]) {
    FILE*       input       = NULL;
    char*       output_dir  = NULL;
    uint32_t    max_iter    = 0;    // 0 represent inf value
    uint32_t    dump_freq   = 1;

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], INPUT_ARG) == 0) {
            if (i + 1 == argc) {
                fprintf(stderr, "Input file name wasn't provided\n");
                goto ARG_PARSE_CLEANUP;
            }

            input = fopen(argv[i + 1], "rwb+");

            if (!input) {
                fprintf(stderr, "Couldn't open input file: %s\n", argv[i + 1]);
                goto ARG_PARSE_CLEANUP;
            }
        }
        
        if (strcmp(argv[i], OUTPUT_ARG) == 0) {
            if (i + 1 == argc) {
                fprintf(stderr, "Argument output wasn't provided\n");
                goto ARG_PARSE_CLEANUP;
            }

            output_dir = argv[i + 1];
            mkdir(output_dir, 0700);
        }

        if (strcmp(argv[i], MAX_ITER_ARG) == 0) {
            if (i + 1 == argc) {
                fprintf(stderr, "Argument max_iter wasn't provided\n");
                goto ARG_PARSE_CLEANUP;
            }

            if ((max_iter = atoi(argv[i + 1])) == 0) {
                fprintf(stderr, "Invalid max_iter argument: %s %s\n", argv[i], argv[i + 1]);
                goto ARG_PARSE_CLEANUP;
            }
        }

        if (strcmp(argv[i], DUMP_FREQ_ARG) == 0) {
            if (i + 1 == argc) {
                fprintf(stderr, "Argument dump_freq wasn't provded\n");
                goto ARG_PARSE_CLEANUP;
            }

            if ((dump_freq = atoi(argv[i + 1])) == 0) {
                fprintf(stderr, "Invalid dump_freq argument: %s %s\n", argv[i], argv[i + 1]);
                goto ARG_PARSE_CLEANUP;
            }
        }
    }

    struct bmp_image* start = NULL;

    if (input) {
        bmp_decode_image(input, &start);
    }

    struct life_config config = {
        .start = start,
        .out_dir = output_dir,
        .max_iter = max_iter,
        .dump_freq = dump_freq
    };

    if (config.max_iter == 0) {
        printf("Are you sure, that you want infite game? [y/n]\n");

        if (getchar() != 'y') {
            goto ARG_PARSE_CLEANUP;
        }
    }

    struct life_game* game;

    if (life_game_init(&config, &game) != 0) {
        fprintf(stderr, "Failed to init game\n");
        goto GAME_CLEANUP;
    }

    while (life_game_step(game) == 0);

GAME_CLEANUP:
    bmp_free_image(&start);
    life_free_game(&game);

ARG_PARSE_CLEANUP:
    if (input) {
        fclose(input);
    }

    return 0;
}
