#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#define STRSIZE (1 << 10)
#define BUFSIZE (1 << 20)

void help(const char* name) {
    printf(
        "Usage: %s [OPTION] <FILENAME>\n"
        "Options: \n"
        "   -l, --lines         print newlines counts\n"
        "   -c, --bytes         print bytes counts\n"
        "   -w, --words         print words counts\n"
        "   -h, --help          display this help and exit\n"
    , name);
}

int getopt(int argc, char** argv, const char* arg) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], arg) == 0) {
            return i;
        }
    }

    return 0;
}

char* getfilename(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            return argv[i];
        }
    }

    return NULL;
}

void print_output(int outc, char** outv, int* outm) {
    int have_option = 0;

    for (int i = 0; i < outc; i++)
        have_option |= outm[i];

    for (int i = 0; i < outc; i++) {
        if (outm[i] || !have_option) {
            printf("%s", outv[i]);
        }
    }
}

int main(int argc, char** argv) {
    if (getopt(argc, argv, "-h") || getopt(argc, argv, "--help")) {
        help(argv[0]);
        return 0;
    }
    
    char* filename = getfilename(argc, argv);
    if (!filename) {
        fprintf(stderr, "Invalid usage: filename not found");
        return 0;
    }

    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "ERROR: failed to open file\n");
        return 0;
    }

    char* buf = malloc(BUFSIZE);
    if (!buf) {
        fprintf(stderr, "ERROR: failed to allocate memory");
        return 0;
    }
    
    int start_word = 1;
    size_t lines = 0, words = 0, bytes = 0;
    while (1) {
        size_t n = fread(buf, sizeof(char), BUFSIZE, file);
        bytes += n;

        if (n == 0)
            break;

        for (size_t i = 0; i < n; i++) {
            if (!isspace(buf[i])) {
                words += start_word;
                start_word = 0;
            }
            else
                start_word = 1;
                
            if (buf[i] == '\n') 
                lines++;
        }
    }

    free(buf);
    fclose(file);

#ifdef DEBUG
    char command[100];
    sprintf(command, "wc %s", filename);
    FILE* wc = popen(command, "r");
    int wclines, wcbytes, wcwords;
    fscanf(wc, "%d %d %d", &wclines, &wcwords, &wcbytes);

    if (wclines != lines || wcwords != words || wcbytes != bytes) {
        printf("FAIL\n");
        printf(
            "wc answer: \n"
            "%d lines\n"
            "%d words\n"
            "%d bytes\n\n",
            wclines, wcwords, wcbytes);
    }
#endif

    const int outc = 3;
    char* outv[outc];
    for (int i = 0; i < outc; i++) {
        outv[i] = malloc(STRSIZE);

        if (!outv[i]) {
            fprintf(stderr, "ERROR: fail to allocate memory\n");
            return 0;
        }
    } 

    int outm[3] = { 
        getopt(argc, argv, "-l") | getopt(argc, argv, "--lines"),
        getopt(argc, argv, "-w") | getopt(argc, argv, "--words"),
        getopt(argc, argv, "-c") | getopt(argc, argv, "--bytes"),
    };

    snprintf(outv[0], STRSIZE, "%zu lines\n", lines);
    snprintf(outv[1], STRSIZE, "%zu words\n", words);
    snprintf(outv[2], STRSIZE, "%zu bytes\n", bytes);

    print_output(outc, outv, outm);
    
    return 0;
}