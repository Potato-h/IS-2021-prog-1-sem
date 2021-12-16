#include "liblife.h"
#include <stdlib.h>
#include <string.h>

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */

#define log(format, ...)            fprintf(stderr, "%s(): " format ": %s:%d\n", __func__, ##__VA_ARGS__, __FILE__, __LINE__)
#define log_warning(format, ...)    fprintf(stderr, YELLOW "%s(): " format ": %s:%d\n" RESET, __func__, ##__VA_ARGS__, __FILE__, __LINE__)
#define log_error(format, ...)      fprintf(stderr, RED "%s(): " format ": %s:%d\n" RESET, __func__, ##__VA_ARGS__, __FILE__, __LINE__)

#define INPLACE_LIMIT 100

// TODO: Hash previous steps and stop game, when previous 
// state was reached or all cells died

int life_game_init(struct life_config* config, struct life_game** game) {
    *game = malloc(sizeof(struct life_game));
    
    (*game)->step = 0;
    (*game)->config = *config;

    if (config->start) {
        (*game)->state = bmp_copy_image(config->start);
    } else {
        return -1;
    }

    return 0;
}

int life_game_step(struct life_game* game) {
    if ((game->config.max_iter != 0 && game->step == game->config.max_iter)
    || game->step == INPLACE_LIMIT) {
        log("All iteration done");
        return 1;
    }

    game->step++;
    
    struct bmp_image* next = bmp_copy_image(game->state);

    for (size_t i = 0; i < next->bmih.height; i++) {
        for (size_t j = 0; j < next->bmih.width; j++) {
            uint8_t neibo = 0; 
            uint8_t alive = bmp_get_pixel(game->state, i, j) == 0;

            for (int32_t di = -1; di <= 1; di++) {
                for (int32_t dj = -1; dj <= 1; dj++) {
                    if ((di != 0 || dj != 0) 
                    && 0 <= (int32_t)i + di && i + di < next->bmih.height
                    && 0 <= (int32_t)j + dj && j + dj < next->bmih.width
                    ) {
                        neibo += bmp_get_pixel(game->state, i + di, j + dj) == 0;
                    } 
                }
            }

            if (alive && (neibo < 2 || neibo > 3)) {
                // Alive cell dies by under or over population
                bmp_set_pixel(next, i, j, 1);
                continue;
            }

            if (!alive && neibo == 3) {
                // Dead cell becomes alive cell by reprodaction
                bmp_set_pixel(next, i, j, 0);
                continue;
            }
        }
    }

    // Replace old state by new calculated
    bmp_free_image(&game->state);
    game->state = next;

    // Upload image of new state in output directory
    if (game->step % game->config.dump_freq == 0) {
        FILE* file = NULL;
        char filepath[200];
        char filename[200];
        
        filename[0] = '\0';
        sprintf(filename, "/%d.bmp", game->step);
        strcpy(filepath, game->config.out_dir);
        strcat(filepath, filename);

        if (!(file = fopen(filepath, "wb+"))) {
            log_error("Couldn't open to file\n");
            return -1;
        }

        if (bmp_encode_image(file, game->state) != 0) {
            log_error("Couldn't encode current state of game into file\n");
            fclose(file);
            return -1;
        }

        fclose(file);
    }

    return 0;
}

void life_free_game(struct life_game** game) {
    bmp_free_image(&(*game)->state);
    free(*game);
    *game = NULL;
}
