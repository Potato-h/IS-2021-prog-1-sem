#ifndef HASH_SET_HEADER
#define HASH_SET_HEADER

#include <stdint.h>

#define HS_TABLE_SIZE 100000

struct list {
    uint64_t key;
    struct list* next;
};

static int list_exists(struct list* head, uint64_t key);

static void list_insert(struct list** head, uint64_t key);

static void list_erase(struct list** head, uint64_t key);

static void list_free(struct list** head);

struct hash_set {
    struct list* table[HS_TABLE_SIZE];  
};

static uint32_t hs_bucket(uint64_t key);

// Return 1 if key exists and 0 otherwise.
int hs_exists(struct hash_set* set, uint64_t key);

// Insert key into set.
void hs_insert(struct hash_set* set, uint64_t key);

// Erase key from set.
void hs_erase(struct hash_set* set, uint64_t key);

// Deallocate inner part of set
void hs_free(struct hash_set* set);

#endif
