#include <stdint.h>
#include <tice.h>
#include <debug.h>
#include <stdio.h>
#include <graphx.h>

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

static uint8_t warp_tile_handler(uint8_t *tile);
static uint8_t door_tile_handler(uint8_t *tile);

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

static uint8_t ice_block_handler(uint8_t *tile) {
    if (handling_events) {
        if (simple_mover_type == FIREBALL_TYPE) {
            something_died = true;
            if (*tile == TILE_ICE_COIN) {
                *tile = TILE_COIN;
            } else {
                *tile = TILE_EMPTY;
            }
        }
    } else {
        if (move_side == TILE_TOP) {
            on_ice = true;
        }
    }
    return 0;
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

static uint8_t p_block_handler(uint8_t *tile) {
    if (!handling_events) {
        if (move_side == TILE_TOP) {
            unsigned int x, y;
            *tile = TILE_EMPTY;
            tile_to_abs_xy_pos(tile, &x, &y);
            add_poof(x + 2, y + 2);
            oiram.vy = -2;
            game.blue_item_count = 10;
            show_blue_items(true);
        }
    }
    return 0;
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
            *tile = TILE_EMPTY_BLACK;
            if (game.end_count) {
                bumped_tile_t *bump_tile = add_bumped(tile - tilemap.width, TILE_BOTTOM);
                bump_tile->tile_ptr = NULL;
                bump_tile->tile = TILE_COIN;
                bump_tile->count = 6;
                bump_tile->y -= TILE_HEIGHT/2;
                add_coin(bump_tile->x, bump_tile->y);
                game.end_count--;
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
        bump_coin->tile = TILE_COIN;
        bump_coin->count = 6;
        bump_coin->y -= TILE_HEIGHT/2;
        add_coin(bump_coin->x, bump_coin->y);
    }
    return 0;
}
static uint8_t up1_quest_handler(uint8_t *tile) {
    if (quest_tile_handler(tile)) {
        add_mushroom_1up(tile);
    }
    return 0;
}
static uint8_t mushroom_quest_handler(uint8_t *tile) {
    if (quest_tile_handler(tile)) {
        add_mushroom(tile);
    }
    return 0;
}
static uint8_t star_quest_handler(uint8_t *tile) {
    if (quest_tile_handler(tile)) {
        add_star(tile);
    }
    return 0;
}
static uint8_t fire_quest_handler(uint8_t *tile) {
    if (quest_tile_handler(tile)) {
        if ((oiram.flags & FLAG_OIRAM_BIG)) {
            add_fire_flower(tile);
        } else {
            add_mushroom(tile);
        }
    }
    return 0;
}
static uint8_t leaf_quest_handler(uint8_t *tile) {
    if (quest_tile_handler(tile)) {
        if ((oiram.flags & FLAG_OIRAM_BIG)) {
            add_simple_enemy(tile - tilemap.width, LEAF_TYPE);
        } else {
            add_mushroom(tile);
        }
    }
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
        if (*tile == TILE_WATER_COIN) {
            *tile = TILE_WATER;
        } else {
            *tile = TILE_EMPTY;
        }
    }
    return 1;
}

static uint8_t top_tile_handler(uint8_t *tile) {
    bool m = (test_y & 15) < 8;
    if (!handling_events) {
        if ((move_side == TILE_TOP) && m) {
            return 0;
        }
    } else {
        if ((move_side == TILE_TOP || move_side == TILE_X) && m) {
            return 0;
        }
    }
    return 1;
}

static uint8_t vine_tile_handler(uint8_t *tile) {
    if (!handling_events) {
        if (pressed_up) {
            if (!oiram.on_vine) {
                gfx_sprite_t *img0;
                gfx_sprite_t *img1;
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
        if (oiram.on_vine && move_side == TILE_TOP) {
            return 3;
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
bool on_ice;

static uint8_t water_tile_handler(uint8_t *tile) {
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
        
        warp.style = PIPE_DOWN;
        warp.enter = true;
        warp.count = oiram.hitbox.height + 2;
        warp.clip_y = tile_y_loc(tile);
        game.enter_end = true;
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
    
    fluff = poof[num_poofs] = malloc(sizeof(poof_t));
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
    
    ball = fireball[num_fireballs] = malloc(sizeof(fireball_t));
    mover = ball->mover = malloc(sizeof(simple_move_t));
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
    mover->is_flyer = false;
    mover->is_bouncer = true;
    mover->hitbox.height = 7;
    mover->hitbox.width = 7;
    mover->type = type;
    ball->count = 127;
    num_fireballs++;
}

/*
 * this array specifies what tile should do what
 * Indexes defined as ** unused are available for collisions. Note that graphics must be manually configured though.
 */
uint8_t (*tile_handler[256])(uint8_t*) = {
    solid_tile_handler,      // 0   question box
    empty_tile_handler,      // 1   ** unused
    empty_tile_handler,      // 2   ** unused
    empty_tile_handler,      // 3   ** unused
    brick_tile_handler,      // 4   brick block
    empty_tile_handler,      // 5   ** unused
    empty_tile_handler,      // 6   ** unused
    empty_tile_handler,      // 7   ** unused
    jump_tile_handler,       // 8   jump block
    empty_tile_handler,      // 9   ** unused
    empty_tile_handler,      // 10  ** unused
    solid_tile_handler,      // 11  solid box
    solid_tile_handler,      // 12  solid wood
    solid_tile_handler,      // 13  solid empty
    warp_tile_handler,       // 14  pipe green up top left
    warp_tile_handler,       // 15  pipe green up top right
    warp_tile_handler,       // 16  pipe gray up top left
    warp_tile_handler,       // 17  pipe gray up top right
    solid_tile_handler,      // 18  pipe green left left top
    solid_tile_handler,      // 19  pipe green left mid top
    solid_tile_handler,      // 20  pipe gray left left top
    solid_tile_handler,      // 21  pipe gray left mid top
    solid_tile_handler,      // 22  solid grass
    solid_tile_handler,      // 23  solid grass
    solid_tile_handler,      // 24  solid grass
    solid_tile_handler,      // 25  solid ball
    water_tile_handler,      // 26  water
    empty_tile_handler,      // 27  empty
    solid_tile_handler,      // 28  pipe green up left mid
    solid_tile_handler,      // 29  pipe green up right mid
    solid_tile_handler,      // 30  pipe gray up left mid
    solid_tile_handler,      // 31  pipe gray up right mid
    warp_tile_handler,       // 32  pipe green left left bottom
    solid_tile_handler,      // 33  pipe green left right bottom
    warp_tile_handler,       // 34  pipe gray left left bottom
    solid_tile_handler,      // 35  pipe gray left right bottom
    solid_tile_handler,      // 36  solid grass
    solid_tile_handler,      // 37  solid grass
    solid_tile_handler,      // 38  solid grass
    upspk_tile_handler,      // 39  spikes up
    dnspk_tile_handler,      // 40  spikes down
    solid_tile_handler,      // 41  solid cloud
    solid_tile_handler,      // 42  solid wood
    solid_tile_handler,      // 43  solid wood
    solid_tile_handler,      // 44  solid wood
    empty_tile_handler,      // 45  half black
    solid_tile_handler,      // 46  solid black
    down_tile_handler,       // 47  down block
    solid_tile_handler,      // 48  solid wood
    solid_tile_handler,      // 49  solid wood
    solid_tile_handler,      // 50  solid wood
    solid_tile_handler,      // 51  solid grass
    solid_tile_handler,      // 52  solid grass
    solid_tile_handler,      // 53  solid grass
    top_tile_handler,        // 54  solid top
    lava_tile_handler,       // 55  lava
    solid_tile_handler,      // 56  solid wood
    solid_tile_handler,      // 57  solid wood
    solid_tile_handler,      // 58  solid wood
    end_pipe_handler,        // 59  end pipe
    solid_tile_handler,      // 60  end pipe
    empty_tile_handler,      // 61  empty black
    solid_tile_handler,      // 62  solid wood pillar
    solid_tile_handler,      // 63  solid wood
    solid_tile_handler,      // 64  solid wood
    solid_tile_handler,      // 65  solid ground
    solid_tile_handler,      // 66  solid ground
    solid_tile_handler,      // 67  solid ground
    solid_tile_handler,      // 68  solid stone
    steepr_slope_handler,    // 69  steep slope right
    solid_tile_handler,      // 70  cannon up
    solid_tile_handler,      // 71  solid sand
    solid_tile_handler,      // 72  solid sand
    solid_tile_handler,      // 73  solid sand
    solid_tile_handler,      // 74  solid block
    solid_tile_handler,      // 75  solid block
    solid_tile_handler,      // 76  solid block
    solid_tile_handler,      // 77  solid ground
    solid_tile_handler,      // 78  solid ground
    solid_tile_handler,      // 79  solid ground
    solid_tile_handler,      // 80  solid grass
    solid_tile_handler,      // 81  solid grass
    solid_tile_handler,      // 82  solid grass
    solid_tile_handler,      // 83  cannon up left
    solid_tile_handler,      // 84  solid cannon part
    solid_tile_handler,      // 85  solid sand
    solid_tile_handler,      // 86  solid sand
    solid_tile_handler,      // 87  solid sand
    solid_tile_handler,      // 88  solid block
    solid_tile_handler,      // 89  solid block
    solid_tile_handler,      // 90  solid block
    solid_tile_handler,      // 91  solid ground
    solid_tile_handler,      // 92  solid ground
    solid_tile_handler,      // 93  solid ground
    plant_tile_handler,      // 94  yummy plant
    empty_tile_handler,      // 95  ** unused
    empty_tile_handler,      // 96  ** unused
    solid_tile_handler,      // 97  cannon down left
    solid_tile_handler,      // 98  cannon part
    solid_tile_handler,      // 99  solid sand
    solid_tile_handler,      // 100 solid sand
    solid_tile_handler,      // 101 solid sand
    solid_tile_handler,      // 102 solid block
    solid_tile_handler,      // 103 solid block
    solid_tile_handler,      // 104 solid block
    solid_tile_handler,      // 105 solid ground
    solid_tile_handler,      // 106 solid ground
    solid_tile_handler,      // 107 solid ground
    vine_tile_handler,       // 108 vine
    solid_tile_handler,      // 109 end pipe left
    solid_tile_handler,      // 110 end pipe right
    steepl_slope_handler,    // 111 steep slope left
    semi1l_slope_handler,    // 112 semi slope left 0
    semi2l_slope_handler,    // 113 semi slope left 1
    semi2r_slope_handler,    // 114 semi slope right 0
    semi1r_slope_handler,    // 115 semi slope right 1
    empty_tile_handler,      // 116 empty landscape
    empty_tile_handler,      // 117 empty landscape
    quicksandt_handler,      // 118 quicksand top
    solid_tile_handler,      // 119 snow bottom left
    solid_tile_handler,      // 120 snow bottom middle
    solid_tile_handler,      // 121 snow bottom right
    lavas_tile_handler,      // 122 lava top
    empty_tile_handler,      // 123 ** unused
    empty_tile_handler,      // 124 ** unused
    empty_tile_handler,      // 125 ** unused
    water_tile_handler,      // 126 water top
    solid_tile_handler,      // 127 snow left
    solid_tile_handler,      // 128 snow middle
    solid_tile_handler,      // 129 snow right
    empty_tile_handler,      // 130 empty landscape
    empty_tile_handler,      // 131 empty landscape
    quicksand_handler,       // 132 quicksand
    empty_tile_handler,      // 133 ** unused
    empty_tile_handler,      // 134 ** unused
    empty_tile_handler,      // 135 ** unused
    empty_tile_handler,      // 136 empty landscape
    empty_tile_handler,      // 137 empty landscape
    empty_tile_handler,      // 138 empty landscape
    empty_tile_handler,      // 139 empty landscape
    top_tile_handler,        // 140 box top
    top_tile_handler,        // 141 box top
    top_tile_handler,        // 142 box top
    empty_tile_handler,      // 143 box shadow
    empty_tile_handler,      // 144 empty landscape
    empty_tile_handler,      // 145 empty landscape
    jelly_tile_handler,      // 146 jelly
    jelly_tile_handler,      // 147 ** unused
    jelly_tile_handler,      // 148 ** unused
    jelly_tile_handler,      // 149 jelly
    coin_tile_handler,       // 150 coin
    coin_tile_handler,       // 151 water coin
    ice_block_handler,       // 152 ice coin
    empty_tile_handler,      // 153 ** unused
    empty_tile_handler,      // 154 box part
    empty_tile_handler,      // 155 box part
    empty_tile_handler,      // 156 box part
    empty_tile_handler,      // 157 box shadow
    empty_tile_handler,      // 158 landscape
    empty_tile_handler,      // 159 landscape
    empty_tile_handler,      // 160 rope
    vanish_tile_handler,     // 161 vanishing tile
    empty_tile_handler,      // 162 landscape
    empty_tile_handler,      // 163 landscape
    empty_tile_handler,      // 164 landscape
    empty_tile_handler,      // 165 cloud top
    empty_tile_handler,      // 166 cloud top
    empty_tile_handler,      // 167 cloud top
    empty_tile_handler,      // 168 box part
    empty_tile_handler,      // 169 box part
    empty_tile_handler,      // 170 box part
    empty_tile_handler,      // 171 box shadow
    empty_tile_handler,      // 172 landscape
    empty_tile_handler,      // 173 landscape
    empty_tile_handler,      // 174 rope end
    solid_tile_handler,      // 175 reswob clear block
    empty_tile_handler,      // 176 landscape
    empty_tile_handler,      // 177 landscape
    empty_tile_handler,      // 178 landscape
    empty_tile_handler,      // 179 cloud bottom
    empty_tile_handler,      // 180 cloud bottom
    empty_tile_handler,      // 181 cloud bottom
    top_tile_handler,        // 182 box top
    top_tile_handler,        // 183 box top
    top_tile_handler,        // 184 box top
    empty_tile_handler,      // 185 box shadow
    top_tile_handler,        // 186 box top
    top_tile_handler,        // 187 box top
    top_tile_handler,        // 188 box top
    empty_tile_handler,      // 189 box shadow
    top_tile_handler,        // 190 box top
    top_tile_handler,        // 191 box top
    top_tile_handler,        // 192 box top
    empty_tile_handler,      // 193 box shadow
    empty_tile_handler,      // 194 landscape
    empty_tile_handler,      // 195 landscape 
    empty_tile_handler,      // 196 landscape
    empty_tile_handler,      // 197 landscape 
    empty_tile_handler,      // 198 landscape
    empty_tile_handler,      // 199 landscape
    empty_tile_handler,      // 200 landscape
    empty_tile_handler,      // 201 landscape
    empty_tile_handler,      // 202 landscape
    empty_tile_handler,      // 203 landscape
    empty_tile_handler,      // 204 landscape
    empty_tile_handler,      // 205 landscape
    empty_tile_handler,      // 206 landscape
    empty_tile_handler,      // 207 landscape
    empty_tile_handler,      // 208 landscape
    empty_tile_handler,      // 209 landscape
    empty_tile_handler,      // 210 landscape
    empty_tile_handler,      // 211 landscape
    empty_tile_handler,      // 212 landscape
    empty_tile_handler,      // 213 landscape
    empty_tile_handler,      // 214 landscape
    empty_tile_handler,      // 215 landscape
    empty_tile_handler,      // 216 landscape
    empty_tile_handler,      // 217 landscape
    empty_tile_handler,      // 218 landscape
    empty_tile_handler,      // 219 landscape
    empty_tile_handler,      // 220 landscape
    empty_tile_handler,      // 221 landscape
    empty_tile_handler,      // 222 landscape
    empty_tile_handler,      // 223 landscape
    coin_tile_handler,       // 224 water coin
    coin_quest_handler,      // 225 coin question box
    up1_quest_handler,       // 226 1 up mushroom question box
    mushroom_quest_handler,  // 227 mushroom question box
    star_quest_handler,      // 228 star question box
    fire_quest_handler,      // 229 fire question box
    leaf_quest_handler,      // 230 leaf question box
    door_tile_handler,       // 231 door upper
    door_tile_handler,       // 232 door lower
    brick_tile_handler,      // 233 blue brick
    coin_tile_handler,       // 234 blue coin
    empty_tile_handler,      // 235 empty block for blue coins
    empty_tile_handler,      // 236 ** unused
    p_block_handler,         // 237 p switch
    empty_tile_handler,      // 238 empty block for blue blocks
    ice_block_handler,       // 239 ice block
    empty_tile_handler,      // 240 ** oiram start
    empty_tile_handler,      // 241 ** enemy reswob
    empty_tile_handler,      // 242 ** enemy spike
    empty_tile_handler,      // 243 ** enemy fish
    empty_tile_handler,      // 244 ** enemy goomba
    empty_tile_handler,      // 245 ** enemy green koopa
    empty_tile_handler,      // 246 ** enemy red koopa
    empty_tile_handler,      // 247 ** enemy green flying koopa
    empty_tile_handler,      // 248 ** enemy red flying koopa
    empty_tile_handler,      // 249 ** enemy bones koopa
    empty_tile_handler,      // 250 ** enemy thwomp
    empty_tile_handler,      // 251 ** enemy lava fireball
    empty_tile_handler,      // 252 ** enemy chomper plant
    empty_tile_handler,      // 253 ** enemy fire chomper plant
    empty_tile_handler,      // 254 ** enemy boo
    empty_tile_handler,      // 255 ** enemy unused
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
    bump = bumped_tile[num_bumped_tiles] = malloc(sizeof(bumped_tile_t));
    
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

        // check if we should bump it
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
    
    if (!num_bumped_tiles) {
        return;
    }
    
    num_bumped_tiles_less = num_bumped_tiles--;
    
    if (free_me->tile_ptr) {
        if (free_me->tile == TILE_VANISH) {
            *free_me->tile_ptr = TILE_EMPTY;
        } else {
            *free_me->tile_ptr = free_me->tile;
        }
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
#define MASK_DOOR_E      (1<<20)
#define MASK_DOOR_X      (1<<19)
#define MASK_PIPE_DOOR   (MASK_PIPE_UP | MASK_PIPE_DOWN | MASK_PIPE_LEFT | MASK_PIPE_RIGHT | MASK_DOOR_E | MASK_DOOR_X)

warp_info_t warp;

uint8_t door_tile_handler(uint8_t *tile) {
    warp_tile_handler(tile);
    return 1;
}

uint8_t warp_tile_handler(uint8_t *tile) {
    unsigned int i;
    unsigned int offset, x, y;

    if (handling_events || warp.style || oiram.vy > 0) {
        return 0;
    }
    
    tile_to_abs_xy_pos(tile, &x, &y);
    
    offset = tile - tilemap.map;
    warp.enter = false;
    
    for(i = 0; i < warp_num; i += 2) {
        unsigned int warp_enter = warp_info[i];
        unsigned int warp_enter_masked = warp_enter & ~MASK_PIPE_DOOR;

        if (offset == warp_enter_masked) {
            switch (move_side) {
                case TILE_LEFT:
                    if (warp_enter & MASK_PIPE_RIGHT) {
                        warp.style = PIPE_LEFT;
                        warp.enter = true;
                        warp.count = OIRAM_HITBOX_WIDTH;
                        warp.clip_x = x;
                    }
                    break;
                case TILE_RIGHT:
                    if (warp_enter & MASK_PIPE_LEFT) {
                        warp.style = PIPE_RIGHT;
                        warp.enter = true;
                        warp.count = OIRAM_HITBOX_WIDTH;
                        warp.clip_x = x + TILE_WIDTH;
                    }
                    break;
                case TILE_TEST_PIPE_DOWN:
                    if (oiram.x < (int)x + 2) {
                        return 0;
                    }
                    warp.style = PIPE_DOWN;
                    warp.enter = true;
                    warp.count = oiram.hitbox.height;
                    warp.clip_y = y;
                    break;
                case TILE_BOTTOM:
                    if (warp_enter & MASK_PIPE_UP) {
                        if (oiram.x < (int)x + 2) {
                            return 0;
                        }
                        warp.style = PIPE_UP;
                        warp.enter = true;
                        warp.clip_y = y + TILE_HEIGHT;
                        warp.count = oiram.hitbox.height;
                    }
                    break;
                case TILE_TEST_DOOR_UP:
                    if (warp_enter & MASK_DOOR_E) {
                        warp.style = DOOR_WARP;
                        warp.enter = true;
                        warp.count = 4;
                        oiram.door_x = x - oiram.scrollx;
                        oiram.door_y = y - oiram.scrolly;
                    }
                    break;
                default:
                    break;
            }
        }
        
        if (warp.enter) {
            unsigned int not_masked = warp_info[i+1];
            warp.exit_loc = not_masked & ~MASK_PIPE_DOOR;
            
            if (not_masked & MASK_PIPE_UP) {
                warp.exit_style = PIPE_UP;
            } else
            if (not_masked & MASK_PIPE_LEFT) {
                warp.exit_style = PIPE_RIGHT;
                if (oiram.flags & (FLAG_OIRAM_BIG | FLAG_OIRAM_FIRE)) {
                    warp.exit_loc -= tilemap.width;
                }
            } else
            if (not_masked & MASK_PIPE_RIGHT) {
                warp.exit_style = PIPE_LEFT;
                if (oiram.flags & (FLAG_OIRAM_BIG | FLAG_OIRAM_FIRE)) {
                    warp.exit_loc -= tilemap.width;
                }
            } else
            if (not_masked & MASK_DOOR_X) {
                warp.exit_style = DOOR_WARP;
                if (!(oiram.flags & (FLAG_OIRAM_BIG | FLAG_OIRAM_FIRE))) {
                    warp.exit_loc += tilemap.width;
                }
            } else {
                warp.exit_style = PIPE_DOWN;
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

