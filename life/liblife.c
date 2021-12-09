#include "liblife.h"
#include <stdlib.h>
#include <string.h>

// TODO: Rewrite all logs and erros
// TODO: Hide logs in own library

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

void life_game_init(struct life_config* config, struct life_game** game) {
    *game = malloc(sizeof(struct life_game));
    
    // TODO: Just copy whole config
    (*game)->step = 0;
    (*game)->state = config->start;
    (*game)->config = *config;
}

int life_game_step(struct life_game* game) {
    if (game->step == game->config.max_iter) {
        // All iterations were done
        return 1;
    }

    game->step++;
    
    // TODO: Create new bmp structure and replace status with it after calc.
    
    // After state update
    if (game->step % game->config.dump_freq == 0) {
        FILE* file = NULL;
        char filepath[200];
        char filename[200];
        
        filename[0] = '\0';
        sprintf(filename, "/%d.bmp", game->step);
        strcat(filepath, game->config.out_dir);
        strcat(filepath, filename);

        if (!(file = fopen(filepath, "wb+"))) {
            log_error("Couldn't open to file\n");
            return -1;
        }

        if (bmp_encode_image(file, game->state) != 0) {
            log_error("Couldn't encode current state of game into file\n");
            return -1;
        }
    }

    return 0;
}

void life_free_game(struct life_game** game) {
    free(*game);
    *game = NULL;
}