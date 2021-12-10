#ifndef LIBLIFE_HEADER
#define LIBLIFE_HEADER

#include "libbmp.h"
#include <stdint.h>

struct life_config {
    struct bmp_image*   start;
    char*               out_dir;
    uint32_t            max_iter;
    uint32_t            dump_freq;
};

// Can hold all possiable position in hash_set or hash_map
// If I need in hash_set or hash_map, then hash() is needed
struct life_game {
    uint32_t            step;
    struct bmp_image*   state;
    struct life_config  config;  
};

void life_game_init(struct life_config* config, struct life_game** game);

int life_game_step(struct life_game* game);

void life_free_game(struct life_game** game);

#endif