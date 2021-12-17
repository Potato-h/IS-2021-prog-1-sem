#include "hash_set.h"
#include <stdlib.h>
#include <string.h>

static int list_exists(struct list* head, uint64_t key) {
    while (head) {
        if (head->key == key)
            return 1;

        head = head->next;
    }

    return 0;
}

static void list_insert(struct list** head, uint64_t key) {
    struct list* node = malloc(sizeof(struct list));
    node->key = key;
    node->next = *head;
    *head = node;
}

static void list_erase(struct list** head, uint64_t key) {
    if ((*head) == NULL)
        return;

    if ((*head)->key == key) {
        *head = (*head)->next;
        return;
    }

    struct list* current = (*head);
    while (current->next) {
        if (current->next->key == key) {
            current->next = current->next->next;
            return;
        }

        current = current->next;
    }
}

static void list_free(struct list** head) {
    struct list* current = *head;
    
    while (current) {
        struct list* next = current->next;
        free(current);
        current = next;    
    }
    
    (*head) = NULL;
}

static uint32_t hs_bucket(uint64_t key) {
    return (138 * key + 827138) % HS_TABLE_SIZE;
}

void hs_create(struct hash_set* set) {
    memset(set->table, 0, sizeof(struct list*) * HS_TABLE_SIZE);
    return;
}


int hs_exists(struct hash_set* set, uint64_t key) {
    return list_exists(set->table[hs_bucket(key)], key);
}

void hs_insert(struct hash_set* set, uint64_t key) {
    list_insert(&set->table[hs_bucket(key)], key);
}

void hs_erase(struct hash_set* set, uint64_t key) {
    list_erase(&set->table[hs_bucket(key)], key);
}

void hs_free(struct hash_set* set) {
    for (size_t i = 0; i < HS_TABLE_SIZE; i++) {
        list_free(&set->table[i]);
    }
}
