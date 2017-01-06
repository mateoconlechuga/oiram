#ifndef POWERUPS_H
#define POWERUPS_H

#include <stdbool.h>
#include <stdint.h>

bool add_mushroom_1up(uint8_t*);

bool add_mushroom(uint8_t*);
void eat_mushroom(void);

bool add_fire_flower(uint8_t*);
void eat_fire_flower(void);

bool add_star(uint8_t*);
void eat_star(void);

void eat_leaf(void);

bool shrink_oiram(void);
void set_left_oiram_sprites(void);

#endif