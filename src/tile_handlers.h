#ifndef TILE_HANDLERS_H
#define TILE_HANDLERS_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "defines.h"
 
#define MAX_TILE_BUMPS      100

typedef struct {
    int x,y;
    uint8_t *tile_ptr;
    uint8_t tile;
    uint8_t dir;
    uint8_t count;
} bumped_tile_t;

extern bumped_tile_t *bumped_tile[MAX_TILE_BUMPS];
bumped_tile_t *add_bumped(uint8_t *tile, uint8_t dir);
bumped_tile_t *add_bumped_sprite(uint8_t *tile, gfx_image_t *sprite, uint8_t dir);
bool remove_bumped_tile(uint8_t i);
extern uint8_t num_bumped_tiles;

extern int test_y;
extern int test_x;

extern uint8_t (*tile_handler[])(uint8_t*);
extern bool force_jump;

enum tile_interaction {
    TILE_TOP=0,
    TILE_LEFT,
    TILE_RIGHT,
    TILE_BOTTOM,
    TILE_X,
    TILE_TEST_PIPE_DOWN,
};
extern uint8_t move_side;

uint8_t moveable_tile(int x, int y);

#define TILE_QUESTIONBOX_0  0
#define TILE_QUESTIONBOX_1  1
#define TILE_QUESTIONBOX_2  2
#define TILE_QUESTIONBOX_3  3
#define TILE_COIN_0         150
#define TILE_BRICK_0        4
#define TILE_BRICK_1        5
#define TILE_BRICK_2        6
#define TILE_BRICK_3        7
#define TILE_SOLID_BOX      11
#define TILE_SOLID_EMPTY    13
#define TILE_WATER          26
#define TILE_EMPTY          27

// animation functions
void animate_tiles(void);
void draw_tile_grid(void);
void reset_3_animations(void);
void reset_4_animations(void);

void tile_to_abs_xy_pos(uint8_t *tile, unsigned int *x, unsigned int *y);

#endif