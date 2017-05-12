#ifndef TILE_HANDLERS_H
#define TILE_HANDLERS_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "defines.h"
#include "simple_mover.h"

/* Tile index definitions */
#define TILE_QUESTION_BOX         0
#define TILE_BRICK                4
#define TILE_SOLID_BOX            11
#define TILE_SOLID_EMPTY          13
#define TILE_WATER                26
#define TILE_EMPTY                27
#define TILE_EMPTY_BLACK          46
#define TILE_LAVA                 55
#define TILE_END_PIPE_L           109
#define TILE_END_PIPE_R           110
#define TILE_SNOW_BL              119
#define TILE_SNOW_BM              120
#define TILE_SNOW_BR              121
#define TILE_LAVA_TOP             122
#define TILE_WATER_TOP            126
#define TILE_SNOW_TL              127
#define TILE_SNOW_TM              128
#define TILE_SNOW_TR              129
#define TILE_COIN                 150
#define TILE_WATER_COIN           151
#define TILE_ICE_COIN             152
#define TILE_VANISH               161

#define TILE_COIN_BOX             225
#define TILE_1UP_BOX              226
#define TILE_MUSHROOM_BOX         227
#define TILE_STAR_BOX             228
#define TILE_FIREFLOWER_BOX       229
#define TILE_LEAF_BOX             230

#define TILE_BLUE_BRICK           233
#define TILE_BLUE_COIN            234
#define TILE_BLUE_COIN_X          235
#define TILE_BLUE_BRICK_X         238
#define TILE_ICE                  239

#define TILE_E_ORIAM_START        240
#define TILE_E_RESWOB             241
#define TILE_E_SPIKE              242
#define TILE_E_FISH               243
#define TILE_E_GOOMBA             244
#define TILE_E_GREEN_KOOPA        245
#define TILE_E_RED_KOOPA          246
#define TILE_E_GREEN_FLY_KOOPA    247
#define TILE_E_RED_FLY_KOOPA      248
#define TILE_E_BONES_KOOPA        249
#define TILE_E_THWOMP             250
#define TILE_E_LAVA_FIREBALL      251
#define TILE_E_CHOMPER            252
#define TILE_E_FIRE_CHOMPER       253
#define TILE_E_BOO                254

typedef struct {
    int x,y;
    uint8_t *tile_ptr;
    uint8_t tile;
    uint8_t dir;
    uint8_t count;
} bumped_tile_t;

#define MAX_TILE_BUMPS 100

extern bumped_tile_t *bumped_tile[MAX_TILE_BUMPS];
bumped_tile_t *add_bumped(uint8_t *tile, uint8_t dir);
void remove_bumped_tile(uint8_t i);
extern uint8_t num_bumped_tiles;

extern int test_y;
extern int test_x;
extern int *test_y_ptr;
extern uint8_t on_slope;
extern int test_y_height;
extern uint8_t (*tile_handler[256])(uint8_t*);
extern bool force_jump;
extern bool in_quicksand;
extern bool in_water;
extern bool on_ice;
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
    TILE_TEST_DOOR_UP,
};

extern uint8_t move_side;

uint8_t moveable_tile(int x, int y);
uint8_t moveable_tile_left_bottom(int x, int y);
uint8_t moveable_tile_right_bottom(int x, int y);

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
    NO_FIREBALL,
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
