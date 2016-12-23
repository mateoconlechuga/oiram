#include <stdint.h>
#include <lib/ce/graphx.h>
#include <tice.h>
#include <debug.h>
#include <stdio.h>

#include "defines.h"
#include "powerups.h"
#include "tile_handlers.h"
#include "simple_mover.h"
#include "events.h"
#include "mario.h"
#include "images.h"
#include "lower.h"

#define tile_y_loc(x) ((((unsigned int)(x) - (unsigned int)tilemap.map) / tilemap.width) * TILE_HEIGHT)

uint8_t move_side;
bool force_jump;
int test_y;
int test_x;
int *test_y_ptr;
int test_y_height;
uint8_t testing_side;
bool handled_slope = false; 
uint8_t on_slope = 0;

#pragma asm "xref _gfx_TilePtr"
#pragma asm "xref __indcall"
#pragma asm "xdef _moveable_tile"
#pragma asm "xdef _moveable_tile_left_bottom"
#pragma asm "xdef _moveable_tile_right_bottom"

#pragma asm "_moveable_tile_right_bottom:"
#pragma asm "	xor	a,a ; TEST_RIGHT"
#pragma asm "	jr	_test_against"
#pragma asm "_moveable_tile_left_bottom:"
#pragma asm "	ld	a,1 ; TEST_LEFT"
#pragma asm "	jr	_test_against"
#pragma asm "_moveable_tile:"
#pragma asm "	ld	a,2 ; TEST_NONE"
#pragma asm "_test_against:"
#pragma asm "	ld	(_testing_side),a"
#pragma asm "	pop	bc"
#pragma asm "	pop	hl"
#pragma asm "	pop	de"
#pragma asm "	push	de"
#pragma asm "	push	hl"
#pragma asm "	push	bc"
#pragma asm "	ld	bc,0"
#pragma asm "	xor	a,a"
#pragma asm "	sbc	hl,bc"
#pragma asm "	jp	p,_l__2"
#pragma asm "	jp	pe,_l_2"
#pragma asm "	ret"
#pragma asm "_l__2:"
#pragma asm "	jp	po,_l_2"
#pragma asm "	ret"
#pragma asm "_l_2:"
#pragma asm "	ex	de,hl"
#pragma asm "	or	a,a"
#pragma asm "	sbc	hl,bc"
#pragma asm "	jp	p,_l__4"
#pragma asm "	jp	pe,_l_3"
#pragma asm "	jr	_l__5"
#pragma asm "_l__4:"
#pragma asm "	jp	po,_l_3"
#pragma asm "_l__5:"
#pragma asm "	ld	a,1"
#pragma asm "	ret"
#pragma asm "_l_3:"
#pragma asm "	ld	(_test_x),de"
#pragma asm "	ld	(_test_y),hl"
#pragma asm "	push	hl"
#pragma asm "	push	de"
#pragma asm "	ld	de,_tilemap"
#pragma asm "	push	de"
#pragma asm "	call	_gfx_TilePtr"
#pragma asm "	pop	de"
#pragma asm "	pop	de"
#pragma asm "	pop	de"
#pragma asm "	push	hl"
#pragma asm "	ld	l,(hl)"
#pragma asm "	ld	h,3"
#pragma asm "	mlt	hl"
#pragma asm "	ld	de,_tile_handler"
#pragma asm "	add	hl,de"
#pragma asm "	ld	iy,(hl)"
#pragma asm "	call	__indcall"
#pragma asm "	pop	de"
#pragma asm "	ret"
   
// handle collisions with different tiles
// function pointers are nice for this
/*
bool moveable_tile(int x, int y) {
    uint8_t *tile;
    testing_side = 2;
    
    if (x < 0) { return false; }
    if (y < 0) { return true; }
    tile = gfx_TilePtr(&tilemap, test_x = x, test_y = y);
    return (*tile_handler[*tile])(tile);
}
*/

/*
static uint8_t solid_tile_handler(uint8_t *tile) {
    return 0;
}
*/

static uint8_t solid_tile_handler(uint8_t *tile);

#pragma asm "_solid_tile_handler:"
#pragma asm "xor a,a"
#pragma asm "ret"

#pragma asm "xdef _solid_tile_handler"

/*
static uint8_t empty_tile_handler(uint8_t *tile) {
    return 1;
}
*/

static uint8_t empty_tile_handler(uint8_t *tile);

#pragma asm "_empty_tile_handler:"
#pragma asm "ld a,1"
#pragma asm "ret"

#pragma asm "xdef _empty_tile_handler"

uint8_t handle_warp_pipe(uint8_t *tile);

static uint8_t brick_tile_handler(uint8_t *tile) {
    unsigned int x, y;
    if (move_side == TILE_BOTTOM) {
        if (!handling_events) {
            if (!mario.on_vine) {
                if (mario.flags & FLAG_MARIO_BIG) {
                    *tile = TILE_EMPTY;
                    add_bumped(tile, TILE_BOTTOM);
                    tile_to_abs_xy_pos(tile, &x, &y);
                    add_poof(x + 2, y + 2);
                } else {
                    add_bumped(tile, TILE_BOTTOM);
                    *tile = TILE_SOLID_EMPTY;
                }
            }
        }
    }
    if (move_side == TILE_LEFT || move_side == TILE_RIGHT) {
        if (handling_events) {
            if (simple_mover_type == KOOPA_RED_SHELL_TYPE || simple_mover_type == KOOPA_GREEN_SHELL_TYPE) {
                tile_to_abs_xy_pos(tile, &x, &y);
                add_poof(x + 2, y + 2);
                *tile = TILE_EMPTY;
            }
        }
    }
    return 0;
}

static uint8_t upspk_tile_handler(uint8_t *tile) {
    if (move_side == TILE_TOP) {
        if (!handling_events) {
            shrink_mario();
        }
    }
    return 0;
}

static uint8_t lavas_tile_handler(uint8_t *tile) {
    if (!handling_events) {
        if (!someone_died) {
            mario.vy = -11;
            mario.has_shell = false;
            mario.momentum = 8;
            mario.curr_sprite = mario_fail;
            someone_died = true;
        }
    }
    return 1;
}

static uint8_t lava_tile_handler(uint8_t *tile) {
    if (handling_events) {
        something_died = true;
    }
    return 1;
}

static uint8_t dnspk_tile_handler(uint8_t *tile) {
    if (move_side == TILE_BOTTOM) {
        if (!handling_events) {
            shrink_mario();
        }
    }
    return 0;
}

static uint8_t down_tile_handler(uint8_t *tile) {
    bumped_tile_t *bump_tile;
    if (move_side == TILE_BOTTOM) {
        if (!handling_events) {
            add_bumped(tile, TILE_BOTTOM);
            *tile = 61;
            if (game.end_counter) {
                game.end_counter--;
                bump_tile = add_bumped(tile - tilemap.width, TILE_BOTTOM);
                bump_tile->tile_ptr = NULL;
                bump_tile->tile = 150;
                bump_tile->count = 6;
                bump_tile->y -= TILE_HEIGHT/2;
                add_coin();
            }
        }
    }
    return 0;
}

static uint8_t quest_tile_handler(uint8_t *tile) {
    bumped_tile_t *bump_tile;
    if (move_side == TILE_BOTTOM) {
        if (!handling_events) {
            if (!mario.on_vine) {
handle_hit:         
                //if (rand() & 1) { add_mushroom(tile); } else { add_star(tile); }
                
                add_fire_flower(tile);
                
                *tile = TILE_SOLID_BOX;
                add_bumped(tile, TILE_BOTTOM);
                bump_tile = add_bumped(tile - tilemap.width, TILE_BOTTOM);
                bump_tile->tile_ptr = NULL;
                bump_tile->tile = 150;
                bump_tile->count = 6;
                bump_tile->y -= TILE_HEIGHT/2;
                
                *tile = TILE_SOLID_EMPTY;
                
                add_coin();
            }
        }
    } else if (move_side == TILE_LEFT || move_side == TILE_RIGHT) {
        if (handling_events) {
            if (simple_mover_type == KOOPA_RED_SHELL_TYPE) {
                goto handle_hit;
            }
        }
    }
    return 0;
}

static uint8_t jump_tile_handler(uint8_t *tile) {
    if (move_side == TILE_TOP) {
        if (!handling_events) {
            force_jump = true;
            mario.vy = -11;
            add_bumped(tile, TILE_TOP);
            *tile = TILE_SOLID_EMPTY;
        }
    }
    return 0;
}

static uint8_t coin_tile_handler(uint8_t *tile) {
    if (!handling_events) {
        add_coin();
        
        // delete the coin
        *tile = TILE_EMPTY;
    }
    return 1;
}

static uint8_t top_tile_handler(uint8_t *tile) {
    if (move_side == TILE_TOP) {
        return 0;
    }
    return 1;
}

static uint8_t vine_tile_handler(uint8_t *tile) {
    if (!handling_events) {
        if (pressed_up) {
            if (!mario.on_vine) {
                mario.on_vine = true;
                if (mario.flags & FLAG_MARIO_FIRE) {
                    memcpy(mario_0_buffer_right, mario_up_fire_0, MARIO_BIG_SPRITE_SIZE);
                    memcpy(mario_1_buffer_right, mario_up_fire_1, MARIO_BIG_SPRITE_SIZE);
                } else if (mario.flags & FLAG_MARIO_BIG) {
                    memcpy(mario_0_buffer_right, mario_up_big_0, MARIO_BIG_SPRITE_SIZE);
                    memcpy(mario_1_buffer_right, mario_up_big_1, MARIO_BIG_SPRITE_SIZE);
                } else {
                    memcpy(mario_0_buffer_right, mario_up_small_0, MARIO_SMALL_SPRITE_SIZE);
                    memcpy(mario_1_buffer_right, mario_up_small_1, MARIO_SMALL_SPRITE_SIZE);
                }
                set_left_mario_sprites();
            }
            return 1;
        }
        if (move_side == TILE_TOP) {
            return 2;
        }
    }
    return 1;
}

bool in_quicksand;
unsigned int quicksand_clip_y;

static uint8_t quicksand_handler(uint8_t *tile) {
    static uint8_t adder = 0;
    static uint8_t test_sides = 0;
    unsigned int x, y;
    
    tile_to_abs_xy_pos(tile, &x, &y);
    
    if (move_side == TILE_TOP) {
        if (testing_side == TEST_RIGHT && test_sides == 0) {
            test_sides = 1;
        } else if (testing_side == TEST_LEFT && test_sides == 1) {
            if (!adder) {
                *test_y_ptr += 1;
            }
            adder = (adder+1) & 5;
            in_quicksand = true;
            pressed_right = false;
            pressed_left = false;
            test_sides = 0;
        } else {
            test_sides = 0;
        }
    }
    
    if (handling_events) { something_died = true;}
    
    return 0;
}

bool in_water;

static uint8_t water_tile_hander(uint8_t *tile) {
    static uint8_t adder = 0;
    static uint8_t test_sides = 0;
    
    if (move_side == TILE_TOP) {
        if (testing_side == TEST_RIGHT && test_sides == 0) {
            test_sides = 1;
        } else if (testing_side == TEST_LEFT && test_sides == 1) {
            if(mario.vy < 0) { mario.vy += 1; } else { if (mario.vy > 1) { mario.vy = 1; } }
            in_water = true;
            test_sides = 0;
        } else {
            test_sides = 0;
        }
    }
    
    return 1;
}

static uint8_t quicksandt_handler(const uint8_t *tile) {
    unsigned int x;
    
    tile_to_abs_xy_pos(tile, &x, &quicksand_clip_y);
    return quicksand_handler(tile);
}

static uint8_t handle_slope_tile(const uint8_t *heightmap, const uint8_t *tile, const uint8_t test_side) {
    if (!handling_events) {
        if (mario.vy < 0) { return 1; }
    }
    
    if (test_side == TEST_LEFT) {
        if (testing_side == TEST_LEFT) {
            goto resolve_slope;
        }
    } else {
        if (testing_side == TEST_RIGHT) {
resolve_slope:
            *test_y_ptr = tile_y_loc(tile) - test_y_height + heightmap[test_x & 15] - 1;
            on_slope = test_side;
        }
    }
    
    if (move_side == TILE_LEFT || move_side == TILE_RIGHT) {
        return 1;
    }
    return 0;
}

static uint8_t steepl_slope_handler(uint8_t *tile) {
    static const uint8_t heightmap[] = { 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0 };
    return handle_slope_tile(heightmap, tile, TEST_RIGHT);
}

static uint8_t semi1l_slope_handler(uint8_t *tile) {
    static const uint8_t heightmap[] = { 15,14,14,13,13,13,12,12,11,11,10,10,9,9,8,8 };
    return handle_slope_tile(heightmap, tile, TEST_RIGHT);
}

static uint8_t semi2l_slope_handler(uint8_t *tile) {
    static const uint8_t heightmap[] = { 7,6,6,5,5,5,4,4,3,3,2,2,1,1,0,0 };
    return handle_slope_tile(heightmap, tile, TEST_RIGHT);
}

static uint8_t steepr_slope_handler(uint8_t *tile) {
    static const uint8_t heightmap[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
    return handle_slope_tile(heightmap, tile, TEST_LEFT);
}

static uint8_t semi1r_slope_handler(uint8_t *tile) {
    static const uint8_t heightmap[] = { 8,8,9,9,10,10,11,11,12,12,13,13,13,14,14,15 };
    return handle_slope_tile(heightmap, tile, TEST_LEFT);
}

static uint8_t semi2r_slope_handler(uint8_t *tile) {
    static const uint8_t heightmap[] = { 0,0,1,1,2,2,3,3,4,4,5,5,5,6,6,7 };
    return handle_slope_tile(heightmap, tile, TEST_LEFT);
}

static uint8_t jelly_tile_handler(uint8_t *tile) {
    if (!handling_events) {
        if(!shrink_mario()) {
            *tile = 26;
        }
    }
    return 0;
}

static uint8_t end_pipe_handler(uint8_t *tile) {
    if (move_side == TILE_TEST_PIPE_DOWN) {
        mario.in_pipe = PIPE_DOWN;
        mario.enter_pipe = true;
        mario.pipe_counter = mario.hitbox.height; 
        game.entered_end_pipe = true;
    }
    return 0;
}

uint8_t (*tile_handler[])(uint8_t*) = {
    quest_tile_handler, // 0   question box 0
    quest_tile_handler, // 1   question box 1
    quest_tile_handler, // 2   question box 2
    quest_tile_handler, // 3   question box 3
    brick_tile_handler, // 4   brick block 0
    brick_tile_handler, // 5   brick block 1
    brick_tile_handler, // 6   brick block 2
    brick_tile_handler, // 7   brick block 3
    jump_tile_handler,  // 8   jump block 0
    jump_tile_handler,  // 9   jump block 1
    jump_tile_handler,  // 10  jump block 2
    solid_tile_handler, // 11  solid box
    solid_tile_handler, // 12  solid wood
    solid_tile_handler, // 13  solid empty
    handle_warp_pipe,   // 14  pipe green up top left
    handle_warp_pipe,   // 15  pipe green up top right
    handle_warp_pipe,   // 16  pipe gray up top left
    handle_warp_pipe,   // 17  pipe gray up top right
    handle_warp_pipe,   // 18  pipe green left left top
    solid_tile_handler, // 19  pipe green left mid top
    handle_warp_pipe,   // 20  pipe gray left left top
    solid_tile_handler, // 21  pipe gray left mid top
    solid_tile_handler, // 22  solid grass
    solid_tile_handler, // 23  solid grass
    solid_tile_handler, // 24  solid grass
    solid_tile_handler, // 25  solid ball
    water_tile_hander,  // 26  water
    empty_tile_handler, // 27  empty
    solid_tile_handler, // 28  pipe green up left mid
    solid_tile_handler, // 29  pipe green up right mid
    solid_tile_handler, // 30  pipe gray up left mid
    solid_tile_handler, // 31  pipe gray up right mid
    handle_warp_pipe,   // 32  pipe green left left bottom
    solid_tile_handler, // 33  pipe green left right bottom
    handle_warp_pipe,   // 34  pipe gray left left bottom
    solid_tile_handler, // 35  pipe gray left right bottom
    solid_tile_handler, // 36  solid grass
    solid_tile_handler, // 37  solid grass
    solid_tile_handler, // 38  solid grass
    upspk_tile_handler, // 39  spikes up
    dnspk_tile_handler, // 40  spikes down
    solid_tile_handler, // 41  solid cloud
    solid_tile_handler, // 42  solid wood
    solid_tile_handler, // 43  solid wood
    solid_tile_handler, // 44  solid wood
    empty_tile_handler, // 45  half black
    empty_tile_handler, // 46  black
    down_tile_handler,  // 47  down block
    solid_tile_handler, // 48  solid wood
    solid_tile_handler, // 49  solid wood
    solid_tile_handler, // 50  solid wood
    solid_tile_handler, // 51  solid grass
    solid_tile_handler, // 52  solid grass
    solid_tile_handler, // 53  solid grass
    top_tile_handler,   // 54  solid top
    lava_tile_handler,  // 55  lava
    solid_tile_handler, // 56  solid wood
    solid_tile_handler, // 57  solid wood
    solid_tile_handler, // 58  solid wood
    end_pipe_handler,   // 59  end pipe 0
    solid_tile_handler, // 60  end pipe 1 -- solid :)
    solid_tile_handler, // 61  empty solid black
    solid_tile_handler, // 62  solid wood
    solid_tile_handler, // 63  solid wood
    solid_tile_handler, // 64  solid wood
    solid_tile_handler, // 65  solid ground
    solid_tile_handler, // 66  solid ground
    solid_tile_handler, // 67  solid ground
    solid_tile_handler, // 68  solid stone
    steepr_slope_handler, // 69  steep slope right
    solid_tile_handler, // 70  cannon up
    solid_tile_handler, // 71  solid sand
    solid_tile_handler, // 72  solid sand
    solid_tile_handler, // 73  solid sand
    solid_tile_handler, // 74  solid block
    solid_tile_handler, // 75  solid block
    solid_tile_handler, // 76  solid block
    solid_tile_handler, // 77  solid ground
    solid_tile_handler, // 78  solid ground
    solid_tile_handler, // 79  solid ground
    solid_tile_handler, // 80  solid grass
    solid_tile_handler, // 81  solid grass
    solid_tile_handler, // 82  solid grass
    solid_tile_handler, // 83  cannon up left
    solid_tile_handler, // 84  solid cannon part
    solid_tile_handler, // 85  solid sand
    solid_tile_handler, // 86  solid sand
    solid_tile_handler, // 87  solid sand
    solid_tile_handler, // 88  solid block
    solid_tile_handler, // 89  solid block
    solid_tile_handler, // 90  solid block
    solid_tile_handler, // 91  solid ground
    solid_tile_handler, // 92  solid ground
    solid_tile_handler, // 93  solid ground
    upspk_tile_handler,  // 94  yummy plant
    upspk_tile_handler,  // 95  yummy plant
    upspk_tile_handler,  // 96  yummy plant
    solid_tile_handler, // 97  cannon down left
    solid_tile_handler, // 98  cannon part
    solid_tile_handler, // 99  solid sand
    solid_tile_handler, // 100 solid sand
    solid_tile_handler, // 101 solid sand
    solid_tile_handler, // 102 solid block
    solid_tile_handler, // 103 solid block
    solid_tile_handler, // 104 solid block
    solid_tile_handler, // 105 solid ground
    solid_tile_handler, // 106 solid ground
    solid_tile_handler, // 107 solid ground
    vine_tile_handler,  // 108 vine 0
    vine_tile_handler,  // 109 vine 1
    vine_tile_handler,  // 110 vine 2
    steepl_slope_handler, // 111 steep slope left
    semi1l_slope_handler, // 112 semi slope left 0
    semi2l_slope_handler, // 113 semi slope left 1
    semi2r_slope_handler, // 114 semi slope right 0
    semi1r_slope_handler, // 115 semi slope right 1
    empty_tile_handler, // 116 empty landscape
    empty_tile_handler, // 117 empty landscape
    quicksandt_handler, // 118 quicksand top 0
    quicksandt_handler, // 119 quicksand top 1
    quicksandt_handler, // 120 quicksand top 2
    quicksandt_handler, // 121 quicksand top 2
    lavas_tile_handler, // 122 lave 0
    lavas_tile_handler, // 123 lava 1
    lavas_tile_handler, // 124 lava 2
    lavas_tile_handler, // 125 lava 3
    water_tile_hander, // 126 water 0
    water_tile_hander, // 127 water 1
    water_tile_hander, // 128 water 2
    water_tile_hander, // 129 water 3
    empty_tile_handler, // 130 empty landscape
    empty_tile_handler, // 131 empty landscape
    quicksand_handler,  // 132 quicksand 0
    quicksand_handler,  // 133 quicksand 1
    quicksand_handler,  // 134 quicksand 2
    quicksand_handler,  // 135 quicksand 3
    empty_tile_handler, // 136 empty landscape
    empty_tile_handler, // 137 empty landscape
    empty_tile_handler, // 138 empty landscape
    empty_tile_handler, // 139 empty landscape
    top_tile_handler,   // 140 box top
    top_tile_handler,   // 141 box top
    top_tile_handler,   // 142 box top
    empty_tile_handler, // 143 box shadow
    empty_tile_handler, // 144 empty landscape
    empty_tile_handler, // 145 empty landscape
    jelly_tile_handler, // 146 jelly
    jelly_tile_handler, // 147 jelly
    jelly_tile_handler, // 148 jelly
    jelly_tile_handler, // 149 jelly
    coin_tile_handler,  // 150 coin 0
    coin_tile_handler,  // 151 coin 1
    coin_tile_handler,  // 152 coin 2
    coin_tile_handler,  // 153 coin 3
    empty_tile_handler, // 154 box part
    empty_tile_handler, // 155 box part
    empty_tile_handler, // 156 box part
    empty_tile_handler, // 157 box shadow
    empty_tile_handler, // 158 landscape
    empty_tile_handler, // 159 landscape
    empty_tile_handler, // 160 rope left 0
    empty_tile_handler, // 161 rope right 0
    empty_tile_handler, // 162 landscape
    empty_tile_handler, // 163 landscape
    empty_tile_handler, // 164 landscape
    empty_tile_handler, // 165 cloud top
    empty_tile_handler, // 166 cloud top
    empty_tile_handler, // 167 cloud top
    empty_tile_handler, // 168 box part
    empty_tile_handler, // 169 box part
    empty_tile_handler, // 170 box part
    empty_tile_handler, // 171 box shadow
    empty_tile_handler, // 172 landscape
    empty_tile_handler, // 173 landscape
    empty_tile_handler, // 174 rope left 1
    empty_tile_handler, // 175 rope right 1
    empty_tile_handler, // 176 landscape
    empty_tile_handler, // 177 landscape
    empty_tile_handler, // 178 landscape
    empty_tile_handler, // 179 cloud bottom
    empty_tile_handler, // 180 cloud bottom
    empty_tile_handler, // 181 cloud bottom
    top_tile_handler,   // 182 box top
    top_tile_handler,   // 183 box top
    top_tile_handler,   // 184 box top
    empty_tile_handler, // 185 box shadow
    top_tile_handler,   // 186 box top
    top_tile_handler,   // 187 box top
    top_tile_handler,   // 188 box top
    empty_tile_handler, // 189 box shadow
    top_tile_handler,   // 190 box top
    top_tile_handler,   // 191 box top
    top_tile_handler,   // 192 box top
    empty_tile_handler, // 193 box shadow
    empty_tile_handler, // 194 landscape
    empty_tile_handler, // 195 the rest are box parts + shadows and landscapes
    empty_tile_handler, // 196 
    empty_tile_handler, // 197 
    empty_tile_handler, // 198
    empty_tile_handler, // 199
    empty_tile_handler, // 200
    empty_tile_handler, // 201
    empty_tile_handler, // 202
    empty_tile_handler, // 203
    empty_tile_handler, // 204
    empty_tile_handler, // 205
    empty_tile_handler, // 206
    empty_tile_handler, // 208
    empty_tile_handler, // 209
    empty_tile_handler, // 210
    empty_tile_handler, // 211
    empty_tile_handler, // 212
    empty_tile_handler, // 213
    empty_tile_handler, // 214
    empty_tile_handler, // 215
    empty_tile_handler, // 216
    empty_tile_handler, // 217
    empty_tile_handler, // 218
    empty_tile_handler, // 219
    empty_tile_handler, // 220
    empty_tile_handler, // 221
    empty_tile_handler, // 222
    empty_tile_handler, // 223
    empty_tile_handler, // 224
};

// animates animated tiles
// 0 (4 tiles):        mystery brick
// 4 (4 tiles):        bricks
// 8 (3 tiles):        music brick
// 94 (3 tiles):       yummy plant
// 108 (3 tiles):      normal vine
// 118 (4 tiles):      quicksand top
// 122 (4 tiles):      lava
// 126 (4 tiles):      water
// 132 (4 tiles):      quicksand
// 146 (4 tiles):      jelly
// 150 (4 tiles):      coins

#define animate_tile(x) ((x) = (gfx_image_t*)(((uint8_t*)(x)) + TILE_DATA_SIZE))
#define animate_reset_4(x) ((x) = (gfx_image_t*)(((uint8_t*)(x)) - 4*TILE_DATA_SIZE))
#define animate_reset_3(x) ((x) = (gfx_image_t*)(((uint8_t*)(x)) - 3*TILE_DATA_SIZE))

#define animate_sprite(x, y, z) if ((x) == (y)) { (x) = (z); } else { (x) = (y); }

void animate(void) {
    
    if (tiles.animation_counter++ == 3) {
        animate_sprite(goomba_sprite, goomba_0, goomba_1);
        animate_sprite(koopa_red_left_sprite, koopa_red_left_0, koopa_red_left_1);
        animate_sprite(koopa_red_right_sprite, koopa_red_right_0, koopa_red_right_1);
        animate_sprite(koopa_green_left_sprite, koopa_green_left_0, koopa_green_left_1);
        animate_sprite(koopa_green_right_sprite, koopa_green_right_0, koopa_green_right_1);
        animate_sprite(koopa_bones_left_sprite, koopa_bones_left_0, koopa_bones_left_1);
        animate_sprite(koopa_bones_right_sprite, koopa_bones_right_0, koopa_bones_right_1);
        animate_sprite(fish_right_sprite, fish_right_0, fish_right_1);
        animate_sprite(fish_left_sprite, fish_left_0, fish_left_1);
        animate_sprite(chomper_sprite, chomper_0, chomper_1);
        animate_sprite(fireball_sprite, fire_0, fire_1);
        animate_sprite(flame_sprite_up, flame_fire_up_0, flame_fire_up_1);
        animate_sprite(flame_sprite_down, flame_fire_down_0, flame_fire_down_1);
        animate_sprite(wing_right_sprite, wing_right_0, wing_right_1);
        animate_sprite(wing_left_sprite, wing_left_0, wing_left_1);
        mario.sprite_index ^= 1;
        
        animate_tile(tileset_tiles[TILE_QUESTIONBOX_0]);
        animate_tile(tileset_tiles[TILE_BRICK_0]);
        animate_tile(tileset_tiles[8]);
        animate_tile(tileset_tiles[94]);
        animate_tile(tileset_tiles[108]);
        animate_tile(tileset_tiles[126]);
        animate_tile(tileset_tiles[118]);
        animate_tile(tileset_tiles[122]);
        animate_tile(tileset_tiles[132]);
        animate_tile(tileset_tiles[146]);
        animate_tile(tileset_tiles[TILE_COIN_0]);
        
        if (tiles.animation_4_counter == 3) {
            animate_reset_4(tileset_tiles[TILE_QUESTIONBOX_0]);
            animate_reset_4(tileset_tiles[TILE_BRICK_0]);
            animate_reset_4(tileset_tiles[126]);
            animate_reset_4(tileset_tiles[118]);
            animate_reset_4(tileset_tiles[122]);
            animate_reset_4(tileset_tiles[132]);
            animate_reset_4(tileset_tiles[TILE_COIN_0]);
            animate_reset_4(tileset_tiles[146]);
            tiles.animation_4_counter = 255;
        }
        
        if (tiles.animation_3_counter == 2) {
            animate_reset_3(tileset_tiles[8]);
            animate_reset_3(tileset_tiles[94]);
            animate_reset_3(tileset_tiles[108]);
            tiles.animation_3_counter = 255;
        }
        
        tiles.animation_3_counter++;
        tiles.animation_4_counter++;
        tiles.animation_counter = 0;
        if (mario.score_display_counter) {
            if (!--mario.score_display_counter) {
                gfx_SetColor(252);
                gfx_FillRectangle_NoClip(140, 161, 45, 14);
                gfx_BlitLines(gfx_buffer, 161, 14);
            }
        }
    }
}

void tile_to_abs_xy_pos(uint8_t *tile, unsigned int *x, unsigned int *y) {
    unsigned int offset = (unsigned int)tile - (unsigned int)tilemap.map;
    *y = (offset / tilemap.width) * TILE_HEIGHT;
    *x = (offset % tilemap.width) * TILE_WIDTH;
}

bumped_tile_t *bumped_tile[MAX_TILE_BUMPS];
uint8_t num_bumped_tiles = 0;

bumped_tile_t *add_bumped(uint8_t *tile, uint8_t dir) {
    bumped_tile_t *bump;
    unsigned int x, y;
    uint8_t i;
    
    if (num_bumped_tiles > MAX_TILE_BUMPS - 1) {
        return NULL;
    }
    
    tile_to_abs_xy_pos(tile, &x, &y);
    bump = bumped_tile[num_bumped_tiles++] = malloc(sizeof(bumped_tile_t));
    
    switch(dir) {
        case TILE_BOTTOM:
            bump->x = x;
            bump->y = y - TILE_HEIGHT/2;
            break;
        case TILE_TOP:
            bump->x = x;
            bump->y = y + TILE_HEIGHT/2;
            break;
        case TILE_RIGHT:
            bump->x = x - TILE_HEIGHT/2;
            bump->y = y;
            break;
        case TILE_LEFT:
            bump->x = x - TILE_HEIGHT/2;
            bump->y = y;
            break;
        default:
            abort();
    }
    
    bump->dir = dir;
    bump->tile = *tile;
    bump->tile_ptr = tile;
    bump->count = 2;
    
    for(i = 0; i < num_simple_movers; i++) {
        simple_move_t *cur = simple_mover[i];

        // check if we should delete it
        if (gfx_CheckRectangleHotspot(bump->x, bump->y, 15, 15, cur->x, cur->y, cur->hitbox.width, cur->hitbox.height)) {
            cur->vy -= 4;
            cur->bumped = true;
        }
    }
    
    return bump;
}

bool remove_bumped_tile(uint8_t i) {
    bumped_tile_t *free_me = bumped_tile[i];
    
    if (!num_bumped_tiles) {
        return false;
    }
    
    if (free_me->tile_ptr) {
        *free_me->tile_ptr = free_me->tile;
    }
    
    for(; i < num_bumped_tiles - 1; i++) {
        bumped_tile[i] = bumped_tile[i+1];
    }
    
    free(free_me);
    
    num_bumped_tiles--;
    
    return true;
}

#define MASK_PIPE_UP     (1<<23)
#define MASK_PIPE_DOWN   (0)
#define MASK_PIPE_LEFT   (1<<22)
#define MASK_PIPE_RIGHT  (1<<21)

const unsigned int warp_pipe_info[] = {
    4,                            // number of pipe warp places
    14 + (7*50) | MASK_PIPE_RIGHT,  // pipe location data -- 1<<23 = up, 1<<22 = left, 1<<21 = right
    45 + (5*50) | MASK_PIPE_RIGHT, // exits to the right
    
    14 + (10*50) | MASK_PIPE_LEFT, // enters to the left
    48 + (5*50)  | MASK_PIPE_LEFT, // exits to the left
    
    20 + (11*50) | MASK_PIPE_DOWN, // enters to the left
    46 + (1*50)  | MASK_PIPE_UP, // exits to the left
    
    23 + (10*50) | MASK_PIPE_UP, // enters to the left
    46 + (6*50)  | MASK_PIPE_DOWN // exits to the left
    
};

uint8_t handle_warp_pipe(uint8_t *tile) {
    uint8_t i, max_tests;
    unsigned int offset, x, y;
    
    if (handling_events) {
        return 0;
    }
    
    tile_to_abs_xy_pos(tile, &x, &y);
    
    max_tests = (*warp_pipe_info)*2;
    offset = tile - tilemap.map;
    
    mario.enter_pipe = false;
    
    for(i = 1; i <= max_tests; i += 2) {
        unsigned int pipe_enter = warp_pipe_info[i];
        unsigned int pipe_enter_masked = pipe_enter & ~(MASK_PIPE_UP | MASK_PIPE_LEFT | MASK_PIPE_RIGHT /* | MASK_PIPE_DOWN */ );
        
        switch (move_side) {
            case TILE_LEFT:
                if (pipe_enter & MASK_PIPE_LEFT) {
                    if (offset == pipe_enter_masked) {
                        mario.in_pipe = PIPE_LEFT;
                        mario.enter_pipe = true;
                        mario.pipe_counter = mario.hitbox.width;
                    }
                }
                break;
            case TILE_RIGHT:
                if (pipe_enter & MASK_PIPE_RIGHT) {
                    if (offset == pipe_enter_masked) {
                        mario.in_pipe = PIPE_RIGHT;
                        mario.enter_pipe = true;
                        mario.pipe_counter = mario.hitbox.width;
                    }
                }
                break;
            case TILE_TEST_PIPE_DOWN:
                if (offset == pipe_enter_masked) {
                    if (mario.x < x + 2) {
                        return 0;
                    }
                    mario.in_pipe = PIPE_DOWN;
                    mario.enter_pipe = true;
                    mario.pipe_counter = mario.hitbox.height;
                }
                break;
            case TILE_BOTTOM:
                if (pipe_enter & MASK_PIPE_UP) {
                    if (offset == pipe_enter_masked) {
                        if (mario.x < x + 2) {
                            return 0;
                        }
                        mario.in_pipe = PIPE_UP;
                        mario.enter_pipe = true;
                        mario.pipe_counter = mario.hitbox.height;
                    }
                }
                break;
            default:
                break;
        }
        
        if (mario.enter_pipe) {
            unsigned int not_masked = warp_pipe_info[i+1];
            mario.exit_pipe_loc = not_masked & ~(MASK_PIPE_UP | MASK_PIPE_LEFT | MASK_PIPE_RIGHT /* | MASK_PIPE_DOWN */ );
            
            if (not_masked & MASK_PIPE_UP) {
                mario.exit_pipe_dir = PIPE_DOWN;
                if (mario.flags & (FLAG_MARIO_BIG | FLAG_MARIO_FIRE)) {
                    mario.exit_pipe_loc -= tilemap.width;
                }
            } else
            if (not_masked & MASK_PIPE_LEFT) {
                mario.exit_pipe_dir = PIPE_RIGHT;
                if (mario.flags & (FLAG_MARIO_BIG | FLAG_MARIO_FIRE)) {
                    mario.exit_pipe_loc -= tilemap.width;
                }
            } else
            if (not_masked & MASK_PIPE_RIGHT) {
                mario.exit_pipe_dir = PIPE_LEFT;
                if (mario.flags & (FLAG_MARIO_BIG | FLAG_MARIO_FIRE)) {
                    mario.exit_pipe_loc -= tilemap.width;
                }
            } else {
                mario.exit_pipe_dir = PIPE_UP;
            }
            break;
        }
    }
    
    return 0;
}
