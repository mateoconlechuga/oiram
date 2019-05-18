#ifndef LOADSCREEN_H
#define LOADSCREEN_H

#include <stdint.h>

typedef struct {
    uint8_t progress;
    uint8_t coins;
    uint8_t lives;
    uint8_t flags;
    uint24_t score;
    char name[9];
} pack_info_t;

extern pack_info_t pack_info[256];
extern uint8_t num_packs;
extern char *pack_author;

void save_progress(void);
void load_progress(void);
void set_load_screen(void);
void set_level(char *name, uint8_t level);

extern unsigned int *warp_info;
extern unsigned int warp_num;

#endif
