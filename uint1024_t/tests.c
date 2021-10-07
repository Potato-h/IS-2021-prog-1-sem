#include "uint1024.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

void test1() {
    char strx[] = "91239812938109230912391391839813";
    char stry[] = "8173981723981723102930123";
    
    uint1024_t x = stoi(strx), y = stoi(stry); 
    uint1024_t res = subtr_op(x, y);
    res = mult_op(res, y);
    res = add_op(res, x);
    char* strres = itos(res);
    
    char command[1000];
    snprintf(
        command, 
        sizeof(command) / sizeof(char), 
        "python -c \""
        "x = %s\n"
        "y = %s\n"
        "print((x - y) * y + x)\"",
        strx, stry 
    );    
    
    FILE* python = popen(command, "r");
    if (!python) {
        fprintf(stderr, "Failed to open python: %s\n", strerror(errno));
        return;
    }

    char panswer[1000];
    fscanf(python, "%s", panswer);
    
    assert(strlen(panswer) == strlen(strres));
    assert(memcmp(panswer, strres, strlen(panswer)) == 0);
    
    free(strres);
    fclose(python);
    printf("Test 1: \033[1;32mOK\033[0m\n");
}

void test2() {
    char strx[] = "8743928374981734987239847293847928374";
    char stry[] = "9138913091823098129381902839123898";
    
    uint1024_t x = stoi(strx), y = stoi(stry); 
    uint1024_t res = mult_op(x, y);
    res = add_op(res, from_uint(5));
    res = add_op(res, x);
    res = subtr_op(res, mult_op(x, from_uint(2)));
    char* strres = itos(res);
    
    char command[1000];
    snprintf(
        command, 
        sizeof(command) / sizeof(char), 
        "python -c \""
        "x = %s\n"
        "y = %s\n"
        "print(x * y + 5 + x - 2*x)\"",
        strx, stry 
    );    
    
    FILE* python = popen(command, "r");
    if (!python) {
        fprintf(stderr, "Failed to open python: %s\n", strerror(errno));
        return;
    }

    char panswer[1000];
    fscanf(python, "%s", panswer);
    
    assert(strlen(panswer) == strlen(strres));
    assert(memcmp(panswer, strres, strlen(panswer)) == 0);
    
    free(strres);
    fclose(python);
    printf("Test 2: \033[1;32mOK\033[0m\n");
}

void test3() {
    char strx[] = "000002";
    char stry[] = "00000009";
    
    uint1024_t x = stoi(strx), y = stoi(stry);
    uint1024_t res = mult_op(x, y);
    char* strres = itos(res);

    assert(memcmp(strres, "18", 2) == 0); 
    
    free(strres);
    printf("Test 3: \033[1;32mOK\033[0m\n");
}

int main() {
    test1();
    test2();
    test3();

    return 0;
}