#ifndef DEFINES_H
#define DEFINES_H

#include <lib/ce/graphx.h>
#include <stdint.h>

extern unsigned int num_coins;
extern bool handling_events;
extern bool someone_died;
extern bool something_died;
extern bool force_align;

extern gfx_image_t *tileset_tiles[224];

typedef struct {
    uint8_t animation_counter;
    uint8_t animation_3_counter;
    uint8_t animation_4_counter;
    uint8_t current_tile;
    uint8_t current_tile_sel;
    gfx_image_t *current_image_ptr;
} tiles_struct_t;
extern tiles_struct_t tiles;

typedef struct map {
    uint8_t width, height;
    uint8_t *data;
    unsigned int max_x_offset, max_y_offset;
    int max_y, max_x;
} map_t;
extern map_t level_map;

extern gfx_tilemap_t tilemap;

typedef struct {
   uint8_t width, height;
} hitbox_t;

enum directions {
    FACE_LEFT=0,
    FACE_RIGHT=1
};

enum power_ups {
    PWR_NONE=0,
    PWR_MUSHROOM,
    PWR_FIRE,
};

#define FLAG_MARIO_BIG        1
#define FLAG_MARIO_FIRE       2
#define FLAG_MARIO_INVINCIBLE 4
#define FLAG_MARIO_SLIDE      8

#define ONE_UP_SCORE          0x10000

typedef struct {
    int x, y;
    int vy;
    uint8_t vx;
    unsigned int scrollx, scrolly;
    int rel_x, rel_y;
    gfx_image_t *curr_sprite;
    hitbox_t hitbox;
    uint8_t hitbox_height_half;
    bool direction;
    uint8_t flags;
    bool has_shell;
    bool has_red_shell;
    bool crouched;
    uint8_t fireballs;
    int8_t momentum;
    uint8_t sprite_index;
    uint8_t invincible;            // used to count how long invincibility should last
    bool less;
    bool on_vine;
    uint8_t lives;
    unsigned int score;
    uint8_t score_display_counter;
    uint8_t score_counter;
    uint8_t in_pipe;
    bool enter_pipe;
    bool exit_pipe;
    bool pipe_counter;
    uint8_t exit_pipe_dir;
    unsigned int exit_pipe_loc;
    int pipe_clip_x;
    int pipe_clip_y;
    bool on_slope;
} mario_t;
extern mario_t mario;

enum pipe_types {
    PIPE_DOWN=1,
    PIPE_LEFT,
    PIPE_RIGHT,
    PIPE_UP,
};

extern gfx_image_t *mario_left[];
extern gfx_image_t *mario_right[];

typedef struct {
    bool exit;
    bool fastexit;
    uint8_t end_counter;
    bool entered_end_pipe;
} game_t;

extern game_t game;

// -----------------------------
// Tilemap defines
// -----------------------------

#define TILE_DATA_SIZE 258

#define TILE_WIDTH  16
#define TILE_HEIGHT 16

#define TILEMAP_DRAW_WIDTH  21
#define TILEMAP_DRAW_HEIGHT 9

#define Y_OFFSET 0
#define X_OFFSET 0

#define Y_PXL_MAX ((TILEMAP_DRAW_HEIGHT-1) * TILE_HEIGHT)
#define X_PXL_MAX ((TILEMAP_DRAW_WIDTH-1) * TILE_WIDTH)

#define BACKGROUND_COLOR_INDEX 0
#define BLACK_INDEX   255
#define WHITE_INDEX   254
#define PURPLE_INDEX  253

#define MARIO_BIG_SPRITE_SIZE   ((27*16) + 2)
#define MARIO_SMALL_SPRITE_SIZE ((16*16) + 2)
#endif