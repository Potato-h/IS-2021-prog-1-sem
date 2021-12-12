#ifndef LIBLIFE_HEADER
#define LIBLIFE_HEADER

#include "libbmp.h"
#include <stdint.h>

// Config of life game. All fields are requaried
struct life_config {
    struct bmp_image*   start;
    char*               out_dir;
    uint32_t            max_iter;
    uint32_t            dump_freq;
};

// State of current game. All fields will be
// automaticly filled by life_game_init call.
struct life_game {
    uint32_t            step;
    struct bmp_image*   state;
    struct life_config  config;  
};

// Initialization of game.
void life_game_init(struct life_config* config, struct life_game** game);

// One step of game. life_game_init call before first step is required
int life_game_step(struct life_game* game);

// Clean up of all resources
void life_free_game(struct life_game** game);

#endif
