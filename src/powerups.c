// standard headers
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>
#include <debug.h>

// shared libraries
#include <lib/ce/graphx.h>
#include <lib/ce/keypadc.h>
#include <lib/ce/fileioc.h>

// mario stuffs
#include "tilemapdata.h"
#include "tile_handlers.h"
#include "powerups.h"
#include "defines.h"
#include "lower.h"
#include "images.h"
#include "simple_mover.h"

bool add_mushroom_1up(uint8_t *spawing_tile) {
    // add a normal mushroom
    if(add_mushroom(spawing_tile)) {
        // change the type
        simple_mover[num_simple_movers-1]->type = MUSHROOM_1UP_TYPE;
    } else {
        return false;
    }
    return true;
}

bool add_mushroom(uint8_t *spawing_tile) {
    simple_move_t *shroom = add_simple_mover(spawing_tile);
    
    if (!shroom) {
        return false;
    }
    
    shroom->hitbox.width = 15;
    shroom->hitbox.height = 15;
    shroom->y -= TILE_HEIGHT;
    if (mario.x + mario.scrollx < shroom->x) {
        shroom->vx = 2;
    } else {
        shroom->vx = -2;
    }
    shroom->type = MUSHROOM_TYPE;
    return true;
}

bool add_star(uint8_t *spawing_tile) {
    simple_move_t *star = add_simple_mover(spawing_tile);
    
    if (!star) {
        return false;
    }
    
    star->hitbox.width = 15;
    star->hitbox.height = 15;
    star->y -= TILE_HEIGHT;
    if (mario.x + mario.scrollx < star->x) {
        star->vx = 2;
    } else {
        star->vx = -2;
    }
    star->type = STAR_TYPE;
    star->is_bouncer = true;
    return true;
}

bool add_fire_flower(uint8_t *spawing_tile) {
    simple_move_t *shroom = add_simple_mover(spawing_tile);
    
    if (!shroom) {
        return false;
    }
    
    shroom->hitbox.width = 15;
    shroom->hitbox.height = 15;
    shroom->y -= TILE_HEIGHT;
    shroom->type = FIRE_FLOWER_TYPE;
    return true;
}

void set_left_mario_sprites(void) {
    gfx_FlipSpriteY(mario_0_buffer_right, mario_0_buffer_left);
    gfx_FlipSpriteY(mario_1_buffer_right, mario_1_buffer_left);
}

void eat_mushroom_1up(void) {
    add_score(ONE_UP_SCORE);
}

void eat_mushroom(void) {
    if (!(mario.flags & FLAG_MARIO_BIG)) {
        memcpy(mario_0_buffer_right, mario_0_big, 27*16 + 2);
        memcpy(mario_1_buffer_right, mario_1_big, 27*16 + 2);
        set_left_mario_sprites();
        mario.y -= 12;
        mario.hitbox.height = 26;
        mario.hitbox.width = 15;
        mario.flags |= FLAG_MARIO_BIG;
    }
    add_score(1000);
}

void eat_fire_flower(void) {
    if (!(mario.flags & FLAG_MARIO_FIRE)) {
        memcpy(mario_0_buffer_right, mario_0_fire, 27*16 + 2);
        memcpy(mario_1_buffer_right, mario_1_fire, 27*16 + 2);
        set_left_mario_sprites();
        if (!(mario.flags & FLAG_MARIO_BIG)) {
            mario.y -= 12;
        }
        mario.hitbox.height = 26;
        mario.hitbox.width = 15;
        mario.flags |= FLAG_MARIO_BIG | FLAG_MARIO_FIRE;
    }
    add_score(1000);
}

void eat_star(void) {
    mario.invincible = 200;
    mario.flags |= FLAG_MARIO_INVINCIBLE;
    add_score(1000);
}

uint8_t shrink_counter = 0;

bool shrink_mario(void) {
    
    if (mario.flags & (FLAG_MARIO_INVINCIBLE | FLAG_MARIO_SLIDE)) {
        return false;
    }
    
    if (!shrink_counter) {
        shrink_counter = 40;
        if (mario.has_shell) { mario.has_shell = false; }
        
        if (mario.flags & FLAG_MARIO_FIRE) {
            memcpy(mario_0_buffer_right, mario_0_big, 27*16 + 2);
            memcpy(mario_1_buffer_right, mario_1_big, 27*16 + 2);
            set_left_mario_sprites();
            mario.flags &= ~FLAG_MARIO_FIRE;
        } else if (mario.flags & FLAG_MARIO_BIG) {
            memcpy(mario_0_buffer_right, mario_0_small, 27*16 + 2);
            memcpy(mario_1_buffer_right, mario_1_small, 27*16 + 2);
            set_left_mario_sprites();
            if (!mario.crouched) { mario.y += 11; }
            mario.hitbox.height = 15;
            mario.flags &= ~FLAG_MARIO_BIG;
        } else {
            if (!someone_died) {
                mario.vy = -11;
                shrink_counter = 0;
                mario.has_shell = false;
                mario.momentum = 8;
                mario.curr_sprite = mario_fail;
                someone_died = true;
            }
        }
    }
    mario.hitbox_height_half = mario.hitbox.height >> 1;
    
    return true;
}