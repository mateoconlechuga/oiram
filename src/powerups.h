#ifndef POWERUPS_H
#define POWERUPS_H

// standard headers
#include <stdbool.h>
#include <stdint.h>

extern uint8_t shrink_counter;

bool add_mushroom(uint8_t*);
void eat_mushroom(void);

bool add_fire_flower(uint8_t*);
void eat_fire_flower(void);

bool add_star(uint8_t*);
void eat_star(void);

bool shrink_mario(void);
void set_left_mario_sprites(void);

#endif