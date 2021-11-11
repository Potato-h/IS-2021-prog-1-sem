#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#define _XOPEN_SOURCE
#define __USE_XOPEN
#define _GNU_SOURCE
#include <time.h>

static uint32_t stoi(char* num) {
    size_t len = strlen(num);
    uint32_t res = 0;

    for (size_t i = 0; i < len; i++) {
        res *= 10;
        res += num[i] - '0'; 
    }

    return res;
} 

static uint32_t lines(char* filename) {
    char command[100];
    uint32_t lines;
    
    sprintf(command, "wc -l %s", filename);
    FILE* wc = popen(command, "r");
    fscanf(wc, "%u", &lines);
    fclose(wc);

    return lines;
}

// TODO: memset all vars to zero before parse
int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Invalid usage\n");
        return 0;
    }

    FILE*       file                = fopen(argv[1], "r");
    char        remote_addr[100];
    char        local_time[100];
    char        request_info[1000];
    char        request[1000];
    char        bytes_end[100]; // Number of bytes or '-'
    char**      request_failed      = (char**)malloc(lines(argv[1]) * sizeof(char*)); 
    time_t*     reqs                = (time_t*)malloc(lines(argv[1]) * sizeof(time_t));
    uint32_t    count               = 0;
    uint32_t    window              = stoi(argv[2]);
    uint32_t    failed              = 0;
    uint32_t    status;
    
    if (!reqs || !request_failed) {
        fprintf(stderr, "Failed to allocated memory\n");
        return 0;
    }

    while (
        fscanf(
            file, 
            "%s - - [%[^]]] \"%[^\n]", 
            remote_addr, local_time, request_info
        ) == 3
    ) {
        size_t last = 0, len = strlen(request_info);
        for (size_t i = 0; i < len; i++) {
            if (request_info[i] == '\"')
                last = i;
        }

        char format[100]; 
        snprintf(format, sizeof(format) / sizeof(char), "%%%uc\" %%u %%s", last);
        memset(request, '\0', sizeof(request));
        sscanf(request_info, format, request, &status, bytes_end);

        if (status / 100 == 5) {
            request_failed[failed] = (char*)malloc(1000 * sizeof(char));
            snprintf(request_failed[failed], 1000, "%s - - [%s] \"%s\" %u %s", remote_addr, local_time, request, status, bytes_end);
            failed++;
        }

        //printf("%s - - [%s] \"%s\" %u %s\n", remote_addr, local_time, request, status, bytes_end);

        struct tm tt_of_req;
        strptime(local_time, "%d/%b/%Y:%H:%M:%S %z", &tt_of_req);        
        time_t t_of_req = mktime(&tt_of_req);
        reqs[count++] = t_of_req;
    }
    
    printf("Total failed: %u\n", failed);
    for (size_t i = 0; i < failed; i++) {
        printf("%s\n", request_failed[i]);
        free(request_failed[i]);
    }

    uint32_t l = 0, r = 0, lwin = 0, rwin = 0;
    for (; l < count; l++) {
        // Move r to the end of current possiable window
        for (; reqs[r + 1] - reqs[l] <= window && r + 1 < count; r++);
        // Now reqs[r] - reqs[l] <= window and r - l + 1 -> max

        // Pick the biggest window from current and already picked
        if (r - l > rwin - lwin) {
            lwin = l;
            rwin = r;
        }
    }

    // Pointers shouldn't be deallocated and should be copyed immedeatly,
    // because pointer from localtime is shared static object
    struct tm tlwin = *localtime((const time_t*)&reqs[lwin]);
    struct tm trwin = *localtime((const time_t*)&reqs[rwin]);
    
    char stlwin[100];
    char strwin[100];
    strftime(stlwin, sizeof(stlwin), "%d/%b/%Y:%H:%M:%S", &tlwin);
    strftime(strwin, sizeof(strwin), "%d/%b/%Y:%H:%M:%S", &trwin);
    
    printf("Biggest load was beetwen %s and %s with max requests = %d\n", stlwin, strwin, rwin - lwin + 1);
    
    free(request_failed);
    free(reqs);

    return 0;
}