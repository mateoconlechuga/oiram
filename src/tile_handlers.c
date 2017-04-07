#include <stdint.h>
#include <tice.h>
#include <debug.h>
#include <stdio.h>
#include <lib/graphx.h>

#include "defines.h"
#include "powerups.h"
#include "tile_handlers.h"
#include "loadscreen.h"
#include "simple_mover.h"
#include "events.h"
#include "enemies.h"
#include "oiram.h"
#include "images.h"
#include "lower.h"

#define tile_y_loc(x) ((((unsigned int)(x) - (unsigned int)(tilemap.map)) / tilemap.width) * TILE_HEIGHT)

uint8_t move_side;
bool force_jump;
int test_y;
int test_x;
int *test_y_ptr;
int test_y_height;
uint8_t testing_side;
uint8_t on_slope = 0;

uint8_t handle_warp_pipe(uint8_t *tile);

static uint8_t brick_tile_handler(uint8_t *tile) {
    unsigned int x, y;
    if (!handling_events) {
        if (move_side == TILE_BOTTOM) {
            if (!oiram.on_vine) {
                if (oiram.flags & FLAG_OIRAM_BIG) {
                    goto destroy_block;
                } else {
                    add_bumped(tile, TILE_BOTTOM);
                    *tile = TILE_SOLID_EMPTY;
                }
            }
        } else
        if (move_side == TILE_RACOON_POWER) {
            goto destroy_block;
        }
    } else {
        if (move_side == TILE_LEFT || move_side == TILE_RIGHT) {
            if (simple_mover_type > SHELL_TYPES) {
    destroy_block:
                *tile = TILE_EMPTY;
                add_bumped(tile, TILE_BOTTOM);
                tile_to_abs_xy_pos(tile, &x, &y);
                add_poof(x + 2, y + 2);
                *tile = TILE_SOLID_EMPTY;
            }
        } else if (move_side == TILE_RESWOB_DOWN) {
            *tile = TILE_EMPTY;
            tile_to_abs_xy_pos(tile, &x, &y);
            add_poof(x + 2, y + 2);
        }
    }
    
    return 0;
}

static uint8_t upspk_tile_handler(uint8_t *tile) {
    if (!handling_events) {
        if (move_side == TILE_TOP) {
            static uint8_t test_sides = 0;
            if (testing_side == TEST_RIGHT && test_sides == 0) {
                test_sides = 1;
            } else if (testing_side == TEST_LEFT && test_sides == 1) {
                shrink_oiram();
                test_sides = 0;
            } else {
                test_sides = 0;
            }
        }
    }
    return 0;
}

static uint8_t plant_tile_handler(uint8_t *tile) {
    if (!handling_events) {
        if (move_side == TILE_TOP) {
            shrink_oiram();
        }
        return 0;
    }
    return 1;
}

static uint8_t lavas_tile_handler(uint8_t *tile) {
    if (!handling_events) {
        if (move_side == TILE_TOP) {
            static uint8_t test_sides = 0;
            
            if (testing_side == TEST_RIGHT && test_sides == 0) {
                test_sides = 1;
            } else if (testing_side == TEST_LEFT && test_sides == 1) {
                oiram.failed = true;
            } else {
                test_sides = 0;
            }
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
            shrink_oiram();
        }
    }
    return 0;
}

static uint8_t down_tile_handler(uint8_t *tile) {
    if (move_side == TILE_BOTTOM) {
        if (!handling_events) {
            add_bumped(tile, TILE_BOTTOM);
            *tile = 46;
            if (game.end_counter) {
                bumped_tile_t *bump_tile = add_bumped(tile - tilemap.width, TILE_BOTTOM);
                bump_tile->tile_ptr = NULL;
                bump_tile->tile = 150;
                bump_tile->count = 6;
                bump_tile->y -= TILE_HEIGHT/2;
                add_coin(bump_tile->x, bump_tile->y);
                game.end_counter--;
            }
        }
    }
    return 0;
}

static uint8_t quest_tile_handler(uint8_t *tile) {
    if (handling_events) {
        if ((move_side == TILE_LEFT || move_side == TILE_RIGHT) && (simple_mover_type > SHELL_TYPES)) {
            goto handle_hit;
        }
    } else {
        if (move_side == TILE_RACOON_POWER) {
            goto handle_hit;
        }
        if (move_side == TILE_BOTTOM  && !oiram.on_vine) {
handle_hit:     
            *tile = TILE_SOLID_BOX;
            add_bumped(tile, TILE_BOTTOM);
            *tile = TILE_SOLID_EMPTY;
            return 1;
        }
    }
    return 0;
}

static uint8_t coin_quest_handler(uint8_t *tile) {
    if (quest_tile_handler(tile)) {
        bumped_tile_t *bump_coin = add_bumped(tile - tilemap.width, TILE_BOTTOM);
        bump_coin->tile_ptr = NULL;
        bump_coin->tile = 150;
        bump_coin->count = 6;
        bump_coin->y -= TILE_HEIGHT/2;
        add_coin(bump_coin->x, bump_coin->y);
    }
    return 0;
}
static uint8_t up1_quest_handler(uint8_t *tile) {
    if (quest_tile_handler(tile)) { add_mushroom_1up(tile); }
    return 0;
}
static uint8_t mushroom_quest_handler(uint8_t *tile) {
    if (quest_tile_handler(tile)) { add_mushroom(tile); }
    return 0;
}
static uint8_t star_quest_handler(uint8_t *tile) {
    if (quest_tile_handler(tile)) { add_star(tile); }
    return 0;
}
static uint8_t fireflower_quest_handler(uint8_t *tile) {
    if (quest_tile_handler(tile)) { if ((oiram.flags & FLAG_OIRAM_BIG)) { add_fire_flower(tile); } else { add_mushroom(tile); } }
    return 0;
}
static uint8_t leaf_quest_handler(uint8_t *tile) {
    if (quest_tile_handler(tile)) { if ((oiram.flags & FLAG_OIRAM_BIG)) { add_simple_enemy(tile - tilemap.width, LEAF_TYPE); } else { add_mushroom(tile); } }
    return 0;
}

static uint8_t jump_tile_handler(uint8_t *tile) {
    if (!handling_events) {
        if (move_side == TILE_TOP) {
            force_jump = true;
            oiram.vy = -6;
            add_bumped(tile, TILE_TOP);
            *tile = TILE_SOLID_EMPTY;
        }
    }
    return 0;
}

static uint8_t coin_tile_handler(uint8_t *tile) {
    if (!handling_events) {
        unsigned int x, y;
        tile_to_abs_xy_pos(tile, &x, &y);
        add_coin(x, y);
        *tile = TILE_EMPTY;
    }
    return 1;
}

static uint8_t top_tile_handler(uint8_t *tile) {
    if (move_side == TILE_TOP) {
        if (!handling_events) {
            if ((test_y & 15) < 8) { return 0; }
        } else {
            if ((test_y & 15) < 5) { return 0; }
        }
    }
    return 1;
}

static uint8_t vine_tile_handler(uint8_t *tile) {
    if (!handling_events) {
        if (pressed_up) {
            if (!oiram.on_vine) {
                gfx_image_t *img0;
                gfx_image_t *img1;
                oiram.on_vine = true;
                if (oiram.flags & FLAG_OIRAM_RACOON) {
                    img0 = oiram_up_racoon_0;
                    img1 = oiram_up_racoon_1;
                } else if (oiram.flags & FLAG_OIRAM_FIRE) {
                    img0 = oiram_up_fire_0;
                    img1 = oiram_up_fire_1;
                } else if (oiram.flags & FLAG_OIRAM_BIG) {
                    img0 = oiram_up_big_0;
                    img1 = oiram_up_big_1;
                } else {
                    img0 = oiram_up_small_0;
                    img1 = oiram_up_small_1;
                }
                memcpy(oiram_0_buffer_right, img0, OIRAM_BIG_SPRITE_SIZE);
                memcpy(oiram_1_buffer_right, img1, OIRAM_BIG_SPRITE_SIZE);
                set_left_oiram_sprites();
            }
            return 1;
        }
        if (oiram.on_vine) {
            if (move_side == TILE_TOP) {
                return 3;
            }
        }
    }
    return 1;
}

bool in_quicksand;
unsigned int quicksand_clip_y;

static uint8_t quicksand_handler(uint8_t *tile) {
    
    if (handling_events) {
        something_died = true;
    } else {
        unsigned int x, y;
    
        tile_to_abs_xy_pos(tile, &x, &y);
        
        if (move_side == TILE_TOP) {
            static uint8_t test_sides = 0;
            static uint8_t adder = 0;
            
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
        
        if (move_side == TILE_SOLID) {
            in_quicksand = true;
            pressed_right = false;
            pressed_left = false;
            return 1;
        }
    }
    
    return 0;
}

bool in_water;

static uint8_t water_tile_hander(uint8_t *tile) {
    if (move_side == TILE_TOP) {
        static uint8_t test_sides = 0;
        
        if (testing_side == TEST_RIGHT && test_sides == 0) {
            test_sides = 1;
        } else if (testing_side == TEST_LEFT && test_sides == 1) {
            if(oiram.vy < 0) { oiram.vy += 1; } else { if (oiram.vy > 1) { oiram.vy = 1; } }
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
        if (oiram.vy < 0) { return 1; }
        if (oiram.vy > 2) oiram.vy = 2;
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
        if(!shrink_oiram()) {
            *tile = 26;
        }
    }
    return 1;
}

static uint8_t end_pipe_handler(uint8_t *tile) {
    if (move_side == TILE_TEST_PIPE_DOWN) {
        
        oiram.in_pipe = PIPE_DOWN;
        oiram.enter_pipe = true;
        oiram.pipe_counter = oiram.hitbox.height + 2;
        oiram.pipe_clip_y = tile_y_loc(tile);
        game.entered_end_pipe = true;
    }
    return 0;
}

static uint8_t vanish_tile_handler(uint8_t *tile) {
    if (!handling_events) {
        if (move_side == TILE_TOP) {
            bumped_tile_t *bump_tile = add_bumped(tile, TILE_BOTTOM);
            bump_tile->count = 15;
            bump_tile->y += TILE_HEIGHT/2 - 2;
            *tile = TILE_SOLID_EMPTY;
        }
    }
    return 0;
}

fireball_t *fireball[MAX_FIREBALLS];
uint8_t num_fireballs = 0;

poof_t *poof[MAX_POOFS];
uint8_t num_poofs = 0;

void remove_poof(uint8_t i) {
    poof_t *free_me = poof[i];
    uint8_t num_poofs_less;
    
    if (!num_poofs) { return; }
    
    num_poofs_less = num_poofs--;
    
    for(; i < num_poofs_less; i++) {
        poof[i] = poof[i+1];
    }
    
    free(free_me);
}

void add_poof(int x, int y) {
    poof_t *fluff;
    
   if (num_poofs >= MAX_POOFS - 1) {
        remove_poof(0);
    }
    
    fluff = poof[num_poofs] = safe_malloc(sizeof(poof_t));
    num_poofs++;
    fluff->x = x;
    fluff->y = y;
    fluff->count = 3;
    fluff->second = false;
}

void remove_fireball(uint8_t i) {
    fireball_t *free_me = fireball[i];
    uint8_t num_fireballs_less;
    
    if (!num_fireballs) { return; }
    
    num_fireballs_less = num_fireballs--;
     
    if (free_me->mover->type == OIRAM_FIREBALL) {
        oiram.fireballs--;
    }
    
    for(; i < num_fireballs_less; i++) {
        fireball[i] = fireball[i+1];
    }
    
    free(free_me->mover);
    free(free_me);
}

// add a fireball
void add_fireball(int x, int y, uint8_t dir, uint8_t type) {
    fireball_t *ball;
    simple_move_t *mover;
    
    if (num_fireballs >= MAX_FIREBALLS - 1) {
        return;
    }
    
    ball = fireball[num_fireballs] = safe_malloc(sizeof(fireball_t));
    mover = ball->mover = safe_malloc(sizeof(simple_move_t));
    mover->x = x;
    mover->y = y;
    
    // handle DOWN_RIGHT by default
    mover->vy = 3;
    mover->vx = 3;
    
    switch(dir) {
        case UP_LEFT:
            mover->vx = -3;
            mover->vy = -3;
            break;
        case UP_RIGHT:
            mover->vy = -3;
            break;
        case DOWN_LEFT:
            mover->vx = -3;
            break;
        default:
            break;
    }
    mover->is_bouncer = true;
    mover->hitbox.height = 7;
    mover->hitbox.width = 7;
    mover->type = type;
    mover->is_flyer = false;
    ball->count = 127;
    num_fireballs++;
}

uint8_t (*tile_handler[])(uint8_t*) = {
    solid_tile_handler, // 0   question box 0
    solid_tile_handler, // 1   question box 1
    solid_tile_handler, // 2   question box 2
    solid_tile_handler, // 3   question box 3
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
    solid_tile_handler,   // 18  pipe green left left top
    solid_tile_handler, // 19  pipe green left mid top
    solid_tile_handler,   // 20  pipe gray left left top
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
    solid_tile_handler, // 46  solid black
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
    empty_tile_handler, // 61  empty black
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
    plant_tile_handler,  // 94  yummy plant
    plant_tile_handler,  // 95  yummy plant
    plant_tile_handler,  // 96  yummy plant
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
    empty_tile_handler, // 160 rope
    vanish_tile_handler, // 161 vanishing tile
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
    empty_tile_handler, // 174 rope end
    solid_tile_handler, // 175 reswob clear block
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
    empty_tile_handler, // 195 the rest are (mostly) box parts + shadows and landscapes
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
    empty_tile_handler, // 223
    coin_quest_handler, // 225
    up1_quest_handler,  // 226
    mushroom_quest_handler, // 227
    star_quest_handler, // 228
    fireflower_quest_handler, // 229
    leaf_quest_handler, // 230
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler,
    empty_tile_handler, // prevent a crash caused by jumping to invalid code -- lol totes forgot about that
};

bumped_tile_t *bumped_tile[MAX_TILE_BUMPS];
uint8_t num_bumped_tiles = 0;

bumped_tile_t *add_bumped(uint8_t *tile, uint8_t dir) {
    bumped_tile_t *bump;
    unsigned int x, y;
    uint8_t i;
    
    if (num_bumped_tiles > MAX_TILE_BUMPS - 1) {
        remove_bumped_tile(0);
    }
    
    tile_to_abs_xy_pos(tile, &x, &y);
    bump = bumped_tile[num_bumped_tiles] = safe_malloc(sizeof(bumped_tile_t));
    
    switch(dir) {
        case TILE_BOTTOM:
            bump->x = x;
            bump->y = y - TILE_HEIGHT/2;
            break;
        case TILE_TOP:
            bump->x = x;
            bump->y = y + TILE_HEIGHT/2;
            break;
        default:
            break;
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
    
    num_bumped_tiles++;
    
    return bump;
}

void remove_bumped_tile(uint8_t i) {
    bumped_tile_t *free_me = bumped_tile[i];
    uint8_t num_bumped_tiles_less;
    
    if (!num_bumped_tiles) { return; }
    
    num_bumped_tiles_less = num_bumped_tiles--;
    
    if (free_me->tile_ptr) {
        *free_me->tile_ptr = free_me->tile;
    }
    
    if (free_me->tile == 161) {
        *free_me->tile_ptr = TILE_EMPTY;
    }
    
    for(; i < num_bumped_tiles_less; i++) {
        bumped_tile[i] = bumped_tile[i+1];
    }
    
    free(free_me);
}

#define MASK_PIPE_DOWN   (0)
#define MASK_PIPE_UP     (1<<23)
#define MASK_PIPE_LEFT   (1<<22)
#define MASK_PIPE_RIGHT  (1<<21)

uint8_t handle_warp_pipe(uint8_t *tile) {
    unsigned int i;
    unsigned int offset, x, y;
    
    if (handling_events || oiram.vy > 0) {
        return 0;
    }
    
    tile_to_abs_xy_pos(tile, &x, &y);
    
    offset = tile - tilemap.map;
    oiram.enter_pipe = false;
    
    for(i = 0; i < pipe_max_tests; i += 2) {
        unsigned int pipe_enter = warp_pipe_info[i];
        unsigned int pipe_enter_masked = pipe_enter & ~(MASK_PIPE_UP | MASK_PIPE_LEFT | MASK_PIPE_RIGHT /* | MASK_PIPE_DOWN */ );

        if (offset == pipe_enter_masked) {
            switch (move_side) {
                case TILE_LEFT:
                    if (pipe_enter & MASK_PIPE_RIGHT) {
                        oiram.in_pipe = PIPE_LEFT;
                        oiram.enter_pipe = true;
                        oiram.pipe_counter = OIRAM_HITBOX_WIDTH;
                        oiram.pipe_clip_x = x;
                    }
                    break;
                case TILE_RIGHT:
                    if (pipe_enter & MASK_PIPE_LEFT) {
                        oiram.in_pipe = PIPE_RIGHT;
                        oiram.enter_pipe = true;
                        oiram.pipe_counter = OIRAM_HITBOX_WIDTH;
                        oiram.pipe_clip_x = x + TILE_WIDTH;
                    }
                    break;
                case TILE_TEST_PIPE_DOWN:
                    if (oiram.x < x + 2) {
                        return 0;
                    }
                    oiram.in_pipe = PIPE_DOWN;
                    oiram.enter_pipe = true;
                    oiram.pipe_counter = oiram.hitbox.height;
                    oiram.pipe_clip_y = y;
                    break;
                case TILE_BOTTOM:
                    if (pipe_enter & MASK_PIPE_UP) {
                        if (oiram.x < x + 2) {
                            return 0;
                        }
                        oiram.in_pipe = PIPE_UP;
                        oiram.enter_pipe = true;
                        oiram.pipe_clip_y = y + TILE_HEIGHT;
                        oiram.pipe_counter = oiram.hitbox.height;
                    }
                    break;
                default:
                    break;
            }
        }
        
        if (oiram.enter_pipe) {
            unsigned int not_masked = warp_pipe_info[i+1];
            oiram.exit_pipe_loc = not_masked & ~(MASK_PIPE_UP | MASK_PIPE_LEFT | MASK_PIPE_RIGHT /* | MASK_PIPE_DOWN */ );
            
            if (not_masked & MASK_PIPE_UP) {
                oiram.exit_pipe_dir = PIPE_UP;
            } else
            if (not_masked & MASK_PIPE_LEFT) {
                oiram.exit_pipe_dir = PIPE_RIGHT;
                if (oiram.flags & (FLAG_OIRAM_BIG | FLAG_OIRAM_FIRE)) {
                    oiram.exit_pipe_loc -= tilemap.width;
                }
            } else
            if (not_masked & MASK_PIPE_RIGHT) {
                oiram.exit_pipe_dir = PIPE_LEFT;
                if (oiram.flags & (FLAG_OIRAM_BIG | FLAG_OIRAM_FIRE)) {
                    oiram.exit_pipe_loc -= tilemap.width;
                }
            } else {
                oiram.exit_pipe_dir = PIPE_DOWN;
            }
            break;
        }
    }
    
    return 0;
}

/**
 * Functions rewritten in common.asm for speedz
 */
 
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
uint8_t solid_tile_handler(uint8_t *tile) {
    return 0;
}
*/

/*
uint8_t empty_tile_handler(uint8_t *tile) {
    return 1;
}
*/

/*
void tile_to_abs_xy_pos(uint8_t *tile, unsigned int *x, unsigned int *y) {
    unsigned int offset = (unsigned int)tile - (unsigned int)tilemap.map;
    *y = (offset / tilemap.width) * TILE_HEIGHT;
    *x = (offset % tilemap.width) * TILE_WIDTH;
}
*/
