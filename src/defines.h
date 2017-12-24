#ifndef DEFINES_H
#define DEFINES_H

#include <graphx.h>
#include <stdint.h>
#include <stdlib.h>

void interrupt isr_keyboard_alternate(void);
void interrupt isr_keyboard(void);

extern bool handling_events;
extern bool something_died;

extern bool easter_egg1;
extern bool easter_egg2;
extern bool easter_egg3;
extern bool easter_egg4;

typedef struct {
    uint8_t animation_count;
    uint8_t animation_3_count;
    uint8_t animation_4_count;
} tiles_struct_t;
extern tiles_struct_t tiles;

typedef struct map {
    uint8_t width, height;
    uint8_t *data;
    unsigned int max_x_scroll, max_y_scroll;
    int max_y, max_x;
    uint8_t scroll;
} map_t;
extern map_t level_map;

void missing_appvars(void);

extern gfx_tilemap_t tilemap;

typedef struct {
   uint8_t width, height;
} hitbox_t;

enum directions {
    FACE_LEFT=0,
    FACE_RIGHT=1
};

enum autoscroll {
    SCROLL_NONE=0,
    SCROLL_RIGHT,
    SCROLL_LEFT,
    SCROLL_DOWN,
    SCROLL_UP
};

#define FLAG_OIRAM_RESET          0
#define FLAG_OIRAM_BIG            1
#define FLAG_OIRAM_FIRE           2
#define FLAG_OIRAM_INVINCIBLE     4
#define FLAG_OIRAM_SLIDE          8
#define FLAG_OIRAM_RACOON         16

#define ONE_UP_SCORE              8

#define OIRAM_HITBOX_WIDTH        15
#define OIRAM_SMALL_HITBOX_HEIGHT 15
#define OIRAM_BIG_HITBOX_HEIGHT   26

typedef struct {
    bool enter;
    bool exit;
    bool count;
    int clip_x;
    int clip_y;
    uint8_t style;
    uint8_t exit_style;
    unsigned int exit_loc;
} warp_info_t;

extern warp_info_t warp;

enum warp_types {
    WARP_NONE,
    PIPE_DOWN=1,
    PIPE_LEFT,
    PIPE_RIGHT,
    PIPE_UP,
    DOOR_WARP,
};

extern gfx_sprite_t *oiram_left[];
extern gfx_sprite_t *oiram_right[];

typedef struct {
    bool exit;
    bool fastexit;
    unsigned int coins;
    unsigned int score;
    uint8_t end_count;
    bool enter_end;
    uint8_t level;
    uint8_t num_levels;
    uint8_t pack;
    uint24_t seconds;
    bool alternate_keypad;
    uint8_t blue_item_count;
} game_t;

extern game_t game;

/* Tilemap defines */

#define TILE_DATA_SIZE 258

#define TILE_WIDTH  16
#define TILE_HEIGHT 16

#define TILEMAP_DRAW_WIDTH  21
#define TILEMAP_DRAW_HEIGHT 9

#define Y_PXL_MAX ((TILEMAP_DRAW_HEIGHT-1) * TILE_HEIGHT)
#define X_PXL_MAX ((TILEMAP_DRAW_WIDTH-1) * TILE_WIDTH)

#define BACKGROUND_COLOR_INDEX 0
#define BLACK_INDEX            255
#define WHITE_INDEX            254
#define DARK_BLUE_INDEX        252

#define OIRAM_BIG_SPRITE_SIZE   ((27*16) + 2)
#define OIRAM_SMALL_SPRITE_SIZE ((16*16) + 2)

#define oiram_collision(a, b, c, d) gfx_CheckRectangleHotspot(oiram.x, oiram.y, OIRAM_HITBOX_WIDTH, oiram.hitbox.height, a, b, c, d)

#endif
