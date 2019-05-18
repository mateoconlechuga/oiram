#ifndef POWERUPS_H
#define POWERUPS_H

#include <stdbool.h>
#include <stdint.h>

void add_mushroom_1up(uint8_t*);

void add_mushroom(uint8_t*);
void eat_mushroom(void);

void add_fire_flower(uint8_t*);
void eat_fire_flower(void);

void add_star(uint8_t*);
void eat_star(void);

void eat_leaf(void);

void set_left_oiram_sprites(void);

void show_blue_items(bool);

bool shrink_oiram(void);

#endif
