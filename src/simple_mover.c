// standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include "simple_mover.h"
#include "defines.h"
#include "tile_handlers.h"
#include "events.h"
#include "images.h"

simple_move_t *simple_mover[MAX_SIMPLE_MOVERS];
uint8_t num_simple_movers = 0;
uint8_t simple_mover_type;

simple_move_t *add_simple_mover(uint8_t *spawing_tile) {
    simple_move_t *mover;
    unsigned int x, y;
    
    if (num_simple_movers > MAX_SIMPLE_MOVERS - 1) {
        remove_simple_mover(0);
    }
    
    tile_to_abs_xy_pos(spawing_tile, &x, &y);
    
    mover = simple_mover[num_simple_movers] = safe_malloc(sizeof(simple_move_t));
    
    mover->vy = 0;
    mover->vx = 0;
    mover->x = x;
    mover->y = y;
    mover->bumped = false;
    mover->smart = false;
    mover->is_bouncer = false;
    mover->is_flyer = false;
    mover->counter = -1;
    mover->score_counter = 0;
    mover->fly_counter = 0;
    num_simple_movers++;
    return mover;
}

void remove_simple_mover(uint8_t i) {
    simple_move_t *mover = simple_mover[i];
    uint8_t num_simple_movers_less;
    
    if (!num_simple_movers) {
        return;
    }
    
    num_simple_movers_less = num_simple_movers--;
    
    for(; i < num_simple_movers_less; i++) {
        simple_mover[i] = simple_mover[i+1];
    }
    
    free(mover);
}

void simple_move_handler(simple_move_t *this) {
    int tmp_x, tmp_y;
    
    int new_y = this->y;
    int new_x = this->x;
    int tmp_vy = this->vy;
    int tmp_vx = this->vx;
    
    int add_right = this->hitbox.width;
    int add_bottom = this->hitbox.height;
    
    bool test_right_bottom, test_left_bottom;
    
    int tx, ty, tb, tvx;
    
    tmp_x = new_x;
    tmp_y = new_y + add_bottom + 1;
    
    // don't care side
    move_side = TILE_X;
    test_y_ptr = &new_y;
    test_y_height = add_bottom;
    
    simple_mover_type = this->type;
    
    test_left_bottom  = moveable_tile_left_bottom(tmp_x, tmp_y);
    test_right_bottom = moveable_tile_right_bottom(tmp_x + add_right, tmp_y);
    
    if (!this->is_flyer) {
        
        // if nothing below, start accelerating
        if (test_left_bottom || test_right_bottom) {
            if (test_left_bottom && test_right_bottom) {
                if (tmp_vy < 7) {
                    tmp_vy++;
                }
            } else if (this->smart) {
                if ((tmp_vx < 0 && test_left_bottom) || (tmp_vx >= 0 && test_right_bottom)) {
                    tmp_vx = -tmp_vx;
                }
            }
        } else if (this->is_bouncer) {
            tmp_vy = -6;
        }
        
        // check if velocity change
        if (tmp_vy) {
                
                // moving up
                if (tmp_vy < 0) {
                    
                    // check bottom of tile
                    move_side = TILE_BOTTOM;
                    
                    // binary test until we find the new thing -- bitwise or because need both sides
                    while(!(moveable_tile(tmp_x, new_y + tmp_vy) & moveable_tile(tmp_x + add_right, new_y + tmp_vy))) {
                        if ((int8_t)(tmp_vy /= 2) >= 0) { break; }
                    }
                    
                    new_y += tmp_vy;
                    
                // moving down
                } else {
                    
                    // test against top
                    tmp_y = new_y + add_bottom;
                    
                    // check top of tile
                    move_side = TILE_TOP;
                    
                    for(; (unsigned)tmp_vy > 0; tmp_vy--) {
                        ty = tmp_y + tmp_vy;
                        if (moveable_tile_right_bottom(tmp_x + add_right, ty) & moveable_tile_left_bottom(tmp_x, ty)) {
                            break;
                        }
                    }
                    
                    new_y += tmp_vy;
                }
        }
        
    // this is a flying simple one... which means we can pretty much go anywhere
    } else {
        if (this->fly_counter <= 0) {
            this->fly_counter--;
            if (this->fly_counter < -9) { this->fly_counter = 1; tmp_vy = -tmp_vy; }
        } else {
            new_y += tmp_vy;
            this->fly_counter++;
            if (this->fly_counter > 64) { this->fly_counter = 0; }
        }
    }
    
    if (tmp_vx > 0) {
        tb = new_y + add_bottom;
        tmp_x = new_x + add_right;
        tvx = tmp_vx;
        
        // check left of tile
        move_side = TILE_LEFT;
        
        for(; tvx != 0; tvx--) {
            tx = tmp_x + tvx;
            if (moveable_tile_right_bottom(tx, tb) && moveable_tile(tx, new_y)) {
                goto set_new_left_right;
            }
        }
        tmp_vx = -tmp_vx;
set_new_left_right:
        new_x += tvx;
    } else {
        tb = new_y + add_bottom;
        tmp_x = new_x;
        tvx = tmp_vx;
        
        // check right of tile
        move_side = TILE_RIGHT;
            
        for(; tvx != 0; tvx++) {
            tx = tmp_x + tvx;
            if (moveable_tile_left_bottom(tx, tb) && moveable_tile(tx, new_y)) {
                goto set_new_left_right;
            }
        }
        tmp_vx = -tmp_vx;
    }
    
    this->x = new_x;
    this->y = new_y;
    this->vy = tmp_vy;
    this->vx = tmp_vx;
}

