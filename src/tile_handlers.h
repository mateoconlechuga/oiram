#ifndef TILE_HANDLERS_H
#define TILE_HANDLERS_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "defines.h"
#include "simple_mover.h"
 
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
void remove_bumped_tile(uint8_t i);
extern uint8_t num_bumped_tiles;

extern int test_y;
extern int test_x;
extern int *test_y_ptr;
extern uint8_t on_slope;
extern int test_y_height;
extern uint8_t (*tile_handler[])(uint8_t*);
extern bool force_jump;
extern bool in_quicksand;
extern bool in_water;
extern unsigned int quicksand_clip_y;

enum testing_side_enum { TEST_RIGHT=0, TEST_LEFT=1, TEST_NONE=2 };

enum tile_interaction {
    TILE_TOP=0,
    TILE_LEFT,
    TILE_RIGHT,
    TILE_BOTTOM,
    TILE_X,
    TILE_TEST_PIPE_DOWN,
    TILE_RACOON_POWER,
    TILE_RESWOB_DOWN,
    TILE_SOLID,
};
extern uint8_t move_side;

uint8_t moveable_tile(int x, int y);
uint8_t moveable_tile_left_bottom(int x, int y);
uint8_t moveable_tile_right_bottom(int x, int y);

#define TILE_QUESTIONBOX_0  0
#define TILE_QUESTIONBOX_1  1
#define TILE_QUESTIONBOX_2  2
#define TILE_QUESTIONBOX_3  3
#define TILE_BRICK_0        4
#define TILE_BRICK_1        5
#define TILE_BRICK_2        6
#define TILE_BRICK_3        7
#define TILE_SOLID_BOX      11
#define TILE_SOLID_EMPTY    13
#define TILE_WATER          26
#define TILE_EMPTY          27
#define TILE_COIN_0         150

#define TILE_COIN_QUESTIONBOX          225
#define TILE_1UP_QUESTIONBOX           226
#define TILE_MUSHROOM_QUESTIONBOX      227
#define TILE_STAR_QUESTIONBOX          228
#define TILE_FIREFLOWER_QUESTIONBOX    229
#define TILE_LEAF_QUESTIONBOX          230

// animation function
void animate(void);

void tile_to_abs_xy_pos(uint8_t *tile, unsigned int *x, unsigned int *y);

// misc
#define MAX_POOFS     11
#define MAX_FIREBALLS 25

typedef struct {
    int x, y;
    uint8_t count;
    bool second;
} poof_t;

extern poof_t *poof[MAX_POOFS];
extern uint8_t num_poofs;

typedef struct {
    uint8_t count;
    simple_move_t *mover;
} fireball_t;

extern fireball_t *fireball[MAX_FIREBALLS];
extern uint8_t num_fireballs;

enum fireball_directions {
    UP_LEFT=0,
    UP_RIGHT,
    DOWN_LEFT,
    DOWN_RIGHT
};

enum fireball_type {
    OIRAM_FIREBALL=1,
    CHOMPER_FIREBALL,
};

void remove_poof(uint8_t i);
void add_poof(int x, int y);
void remove_fireball(uint8_t i);
void add_fireball(int x, int y, uint8_t dir, uint8_t type);

// common.asm
uint8_t empty_tile_handler(uint8_t *tile);
uint8_t solid_tile_handler(uint8_t *tile);

#endif