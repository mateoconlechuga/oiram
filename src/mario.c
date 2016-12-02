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
#include "defines.h"
#include "powerups.h"
#include "enemies.h"
#include "events.h"
#include "images.h"
#include "simple_mover.h"

bool pressed_left = false;
bool pressed_right = false;
bool pressed_up = false;
bool pressed_down = false;
bool pressed_2nd = false;
bool pressed_yequ = false;

// handle picking up of a shell
bool pickup_shell(void) {
    uint8_t j;
    
    // get absolute locations
    int abs_x = mario.x + mario.scrollx;
    int abs_y = mario.y + mario.scrolly;
    
    // don't recurse
    if (mario.has_shell) { return false; }
    
    // locate shell location
    if (mario.direction == FACE_LEFT) {
        abs_x -= 24;
    } else {
        abs_x += mario.hitbox.width;
    }
    
    // check if there is a shell near mario
    for(j = 0; j < num_simple_movers; j++) {
        simple_move_t *chk = simple_mover[j];
        
        if ((chk->type == KOOPA_RED_SHELL_TYPE || chk->type == KOOPA_GREEN_SHELL_TYPE) && !chk->vx) {
            if (gfx_CheckRectangleHotspot(abs_x, abs_y, 24, mario.hitbox.height, chk->x, chk->y, 15, 15)) {
                remove_simple_mover(j);
                mario.has_shell = true;
                mario.has_red_shell = chk->type == KOOPA_RED_SHELL_TYPE;
                return true;
            }
        }
    }
    
    // return false if we can't pick up a shell
    return false;
}

// drops a shell if mario is holding one
void drop_shell(void) {
    simple_move_t *shell = add_simple_mover(NULL);
    shell->x = ((mario.direction == FACE_LEFT) ? mario.x - 16 : mario.x + mario.hitbox.width) + mario.scrollx;
    shell->y = ((mario.flags & FLAG_MARIO_BIG) ? mario.y + 26/2 - 4 : mario.y) + mario.scrolly;
    shell->vx = (mario.direction == FACE_LEFT) ? -3 : 3;
    shell->hitbox.height = 15;
    shell->hitbox.width = 15;
    shell->sprite = mario.has_red_shell ? koopa_red_shell_0 : koopa_green_shell_0;
    shell->type = mario.has_red_shell ? KOOPA_RED_SHELL_TYPE : KOOPA_GREEN_SHELL_TYPE;
    mario.has_shell = false;
}

// crouching is fun
void crouch_mario(void) {
    
    // check if we are big
    if ( mario.flags & (FLAG_MARIO_FIRE | FLAG_MARIO_BIG)) {
        if (mario.flags & FLAG_MARIO_FIRE) {
            memcpy(mario_0_buffer_right, mario_crouch_fire, 16*16 + 2);
            memcpy(mario_1_buffer_right, mario_crouch_fire, 16*16 + 2);
        } else {
            memcpy(mario_0_buffer_right, mario_crouch_big, 16*16 + 2);
            memcpy(mario_1_buffer_right, mario_crouch_big, 16*16 + 2);
        }
        
        // register new sprites
        mario.hitbox.height = 15;
        set_left_mario_sprites();
        mario.crouched = true;
        return;
    }
    mario.crouched = false;
}

void uncrouch_mario(void) {
    if ( mario.flags & (FLAG_MARIO_FIRE | FLAG_MARIO_BIG)) {
        if (mario.flags & FLAG_MARIO_FIRE) {
            memcpy(mario_0_buffer_right, mario_0_fire, 27*16 + 2);
            memcpy(mario_1_buffer_right, mario_1_fire, 27*16 + 2);
        } else {
            memcpy(mario_0_buffer_right, mario_0_big, 27*16 + 2);
            memcpy(mario_1_buffer_right, mario_1_big, 27*16 + 2);
        }
        mario.hitbox.height = 26;
        set_left_mario_sprites();
        mario.crouched = false;
    }
}

void compute_mario_location_from_offset(unsigned int offset) {
    unsigned int x, y;
    unsigned int new_y = 0, new_x = 0, new_scroll_y = 0, new_scroll_x = 0;
                    
    tile_to_abs_xy_pos(offset + tilemap.map, &x, &y);

    if (mario.exit_pipe_dir == PIPE_DOWN || mario.exit_pipe_dir == PIPE_UP) {
        x += 8;
    }
    
    // move down
    while (new_scroll_y + new_y != y) {
        if (new_y >= 90) {
            if (new_scroll_y < level_map.max_y_offset)  { new_scroll_y++; }
            else                                        { new_y++; }
        } else                                          { new_y++; }
    }
    
    if (new_scroll_y > level_map.max_y_offset) new_scroll_y = level_map.max_y_offset;
    
    // move right
    while (new_scroll_x + new_x != x) {
        if (new_x >= 150) {
            if (new_scroll_x < level_map.max_x_offset) { new_scroll_x++; }
            else                                       { new_x++; }
        } else                                         { new_x++; }
    }
    
    if (new_scroll_x > level_map.max_x_offset) { new_scroll_x = level_map.max_x_offset; }
    
    mario.scrollx = new_scroll_x;
    mario.scrolly = new_scroll_y;
    mario.x = new_x;
    mario.y = new_y;
    
    mario.pipe_clip_y = new_y;
    mario.pipe_clip_x = new_x;
}

// move mario around on the screen, checking for bounds
void move_mario(void) {
    int new_x = mario.x;
    int new_y = mario.y;
    int prev_scroll_x = (int)mario.scrollx;
    int new_scroll_y = (int)mario.scrolly;
    
    int add_bottom = mario.hitbox.height;
    int add_right = mario.hitbox.width;
    
    int tmp_x = prev_scroll_x + new_x;
    int tmp_y = new_scroll_y + new_y + add_bottom;
    
    int tx;
    uint8_t vx = mario.vx;
    uint8_t l,r;
    
    // do something else if someone died
    if (someone_died) {
        if (mario.vy < 9) { mario.vy += 1; }
        mario.y += mario.vy;
        
        if (mario.y > level_map.max_y + 150) {
            exit_game = true;
            return;
        }
        return;
    }
    
    if (mario.in_pipe) {
        switch (mario.in_pipe) {
            case PIPE_DOWN:
                mario.y++;
                break;
            case PIPE_LEFT:
                mario.x++;
                break;
            case PIPE_RIGHT:
                mario.x--;
                break;
            case PIPE_UP:
                mario.y--;
                break;
        }
        
        mario.pipe_counter--;
        if (!mario.pipe_counter) {
            if (mario.enter_pipe) {
                mario.enter_pipe = false;
                mario.exit_pipe = true;
                mario.in_pipe = mario.exit_pipe_dir;
                compute_mario_location_from_offset(mario.exit_pipe_loc);
                if (mario.exit_pipe_dir == PIPE_DOWN) {
                    mario.in_pipe = 0;
                    mario.y += TILE_HEIGHT;
                    if (mario.flags & (FLAG_MARIO_BIG | FLAG_MARIO_FIRE)) {
                        mario.y += 11;
                    }
                } else
                if (mario.exit_pipe_dir == PIPE_UP) {
                    mario.pipe_counter = mario.hitbox.height + 1;
                } else {
                    if (mario.exit_pipe_dir == PIPE_LEFT) {
                        mario.curr_sprite = mario_right[mario.sprite_index & 1];
                    } else {
                        mario.curr_sprite = mario_left[mario.sprite_index & 1];
                    }
                    mario.pipe_counter = mario.hitbox.width + 1;
                    if (mario.flags & (FLAG_MARIO_BIG | FLAG_MARIO_FIRE)) {
                        mario.y += 2;
                    }
                }
                
            // exiting a pipe
            } else {
                mario.exit_pipe = false;
                mario.in_pipe = 0;
            }
        }
        return;
    }
    
    // make sure we aren't crouching
    if (mario.crouched) {
        pressed_right = pressed_left = false;
        goto handle_reduced_speed;
    } else {
        if (pressed_left || pressed_right) {
            if (pressed_yequ) {
                
                // increase momentum in the x direction
                if (mario.direction == FACE_RIGHT) {
                    if (mario.momentum < 0) { mario.momentum = 0; add_poof(tmp_x, tmp_y); }
                    if (mario.momentum < 5) { mario.vx = 2; } else
                    if (mario.momentum > 20) { mario.vx = 5; mario.sprite_index++; } else
                    if (mario.momentum > 10) { mario.vx = 4; mario.sprite_index++; } else
                    if (mario.momentum > 5) { mario.vx = 3; }
                    if (mario.momentum < 40) { mario.momentum++; }
                } else {
                    if (mario.momentum > 0) { mario.momentum = 0; add_poof(tmp_x + add_right, tmp_y); }
                    if (mario.momentum > -5) { mario.vx = 2; } else
                    if (mario.momentum < -20) { mario.vx = 5; mario.sprite_index++; } else
                    if (mario.momentum < -10) { mario.vx = 4; mario.sprite_index++; } else
                    if (mario.momentum < -5) { mario.vx = 3; }
                    if (mario.momentum > -40) { mario.momentum--; }
                }
            } else {
            
                if (mario.direction == FACE_RIGHT) {
                    if (mario.momentum < 0) { mario.momentum = 0; } else
                    if (mario.momentum < 20) { mario.momentum += 3; }
                } else {
                    if (mario.momentum > -20) { mario.momentum -= 3; } else
                    if (mario.momentum > 0) { mario.momentum = 0; }
                }
                goto handle_reduced_speed;
            }
        } else {
handle_reduced_speed:
            mario.vx = 2;
            
            if (mario.momentum < 0) {
                if ((mario.momentum += 2) > 0) { mario.momentum = 0; }
            } else {
                if ((mario.momentum -= 2) < 0) { mario.momentum = 0; }
            }
        }
    }
    
    // drop a shell or throw a fireball
    if (pressed_2nd) {
        if (mario.has_shell) {
            drop_shell();
        } else {
            if (!pickup_shell() && (mario.flags & FLAG_MARIO_FIRE)) {
                bool facing_left = (mario.direction == FACE_LEFT);
                fireball_t *ball;
                
                if (mario.fireballs < 2) {
                    ball = add_fireball(mario.x + mario.scrollx + ((facing_left) ? 0 : 15), mario.y + mario.scrolly + mario.hitbox.height/2, (facing_left) ? DOWN_LEFT : DOWN_RIGHT);
                    ball->type = MARIO_FIREBALL;
                    mario.fireballs++;
                }
            }
        }
        pressed_2nd = false;
    }
    
    // check the tops when falling
    move_side = TILE_X;
    
    // if nothing below, start accelerating
    if ((l = moveable_tile(tmp_x, tmp_y + 1)) && (r = moveable_tile(tmp_x + add_right, tmp_y + 1))) {
        if (l != 2 && r != 2) {
            if (mario.on_vine) {
                mario.on_vine = false;
                if (mario.flags & FLAG_MARIO_FIRE) {
                    memcpy(mario_0_buffer_right, mario_0_fire, 27*16 + 2);
                    memcpy(mario_1_buffer_right, mario_1_fire, 27*16 + 2);
                } else if (mario.flags & FLAG_MARIO_BIG) {
                    memcpy(mario_0_buffer_right, mario_0_big, 27*16 + 2);
                    memcpy(mario_1_buffer_right, mario_1_big, 27*16 + 2);
                } else {
                    memcpy(mario_0_buffer_right, mario_0_small, 16*16 + 2);
                    memcpy(mario_1_buffer_right, mario_1_small, 16*16 + 2);
                }
                set_left_mario_sprites();
            }
        }
        if (mario.vy < 9) { mario.vy++; }
    } else {
        mario.score_counter = 0;
    }
        
    if (mario.on_vine) {
        mario.vy = 0;
    }
    
    if (pressed_up) {
        test_y = tmp_y = new_scroll_y + new_y + add_bottom + 1;
        
        // check the top of the tile
        move_side = TILE_TOP;
        
        if (force_jump) {
            mario.vy += -9;
            force_jump = false;
        } else if (!(moveable_tile(tmp_x, tmp_y) && moveable_tile(tmp_x + add_right, tmp_y))) {
            mario.vy += -11;
        }
        if (!mario.on_vine) {
            pressed_up = false;
        }
    }
    
    if (pressed_down) {
        if (!mario.crouched) {
            move_side = TILE_TEST_PIPE_DOWN;
            test_y = tmp_y = new_scroll_y + new_y + add_bottom + 1;
            moveable_tile(tmp_x, tmp_y);
            if (mario.in_pipe) {
                goto no_crouch_allowed;
            }
        }
        if (mario.on_vine) {
            mario.vy = 2;
        } else if (!mario.crouched) {
            crouch_mario();
            if (mario.crouched) {
                new_y += 11;
            }
        }
    } else {
        if (mario.crouched) {
            uncrouch_mario();
            new_y -= 11;
        }
    }
no_crouch_allowed:

    // check if velocity change
    if (mario.vy) {
        
        // recalculate y offeset
        tmp_y = new_scroll_y + new_y;
        
        // moving up
        if (mario.vy < 0) {
            
            // check the bottom of the tile
            move_side = TILE_BOTTOM;
            
            // binary test until we find the new thing -- bitwise or because need both sides
            while(!(moveable_tile(tmp_x, tmp_y + mario.vy) & moveable_tile(tmp_x + add_right, tmp_y + mario.vy))) {
                if ((int8_t)(mario.vy /= 2) >= 0) { break; }
            }
            
            // check if we should move the viewwindow
            if (new_y <= 30) {
                if (new_scroll_y) { new_scroll_y += mario.vy; }
                else              { new_y += mario.vy; }
            } else                { new_y += mario.vy; }
            
            // edge case handling
            if (new_scroll_y < 0) new_scroll_y = 0;
            
        // moving down
        } else {
            
            // test against bottom
            test_y = tmp_y += add_bottom;
            
            // check the top of the tile
            move_side = TILE_TOP;
            
            // binary test until we find the new thing
            while(!(moveable_tile(tmp_x, tmp_y + mario.vy) && moveable_tile(tmp_x + add_right, tmp_y + mario.vy))) {
                if ((int8_t)(mario.vy /= 2) <= 0) { break; }
            }
            
            // check if we should move the viewwindow
            if (new_y >= 90) {
                if (new_scroll_y < level_map.max_y_offset)  { new_scroll_y += mario.vy; }
                else                                        { new_y += mario.vy; }
            } else                                          { new_y += mario.vy; }
            
            // edge case handling
            if (new_scroll_y > level_map.max_y_offset) new_scroll_y = level_map.max_y_offset;
        }
    } else {
        if (!moveable_tile(tmp_x + add_right, new_scroll_y + new_y)) {
            goto force_left;
        }
    }
    
    if (pressed_right || mario.momentum > 0) {
        
        mario.direction = FACE_RIGHT;
        mario.curr_sprite = mario_right[mario.sprite_index & 1];
        
        tmp_x = prev_scroll_x + new_x + add_right;
        tmp_y = new_scroll_y + new_y;
        
        // check the left of the tile
        move_side = TILE_LEFT;
        
        for(vx = mario.vx; vx > 0; vx--) {
            tx = tmp_x + vx;
            if (moveable_tile(tx, tmp_y + add_bottom) && moveable_tile(tx, tmp_y) && moveable_tile(tx, tmp_y + mario.hitbox_height_half)) {
                break;
            }
        }
        
        if (vx) {
            if (new_x >= 150) {
                if (prev_scroll_x < level_map.max_x_offset) { prev_scroll_x += vx; }
                else                                        { new_x += vx; }
            } else                                          { new_x += vx; }
            
            if (prev_scroll_x > level_map.max_x_offset) { prev_scroll_x = level_map.max_x_offset; }
        }
        
        if (mario.momentum < 0) {
            goto skip_all;
        }
    }
    
    if (pressed_left || mario.momentum < 0) {
        
        mario.direction = FACE_LEFT;
        mario.curr_sprite = mario_left[mario.sprite_index & 1];
        
        tmp_x = prev_scroll_x + new_x;
        tmp_y = new_scroll_y + new_y;
    
        // check the right of the tile
        move_side = TILE_RIGHT;
        
        for(vx = mario.vx; vx > 0; vx--) {
            tx = tmp_x - vx;
            if (moveable_tile(tx, tmp_y + add_bottom) && moveable_tile(tx, tmp_y) && moveable_tile(tx, tmp_y + mario.hitbox_height_half)) {
                break;
            }
        }
        
        if ((mario.vx = vx)) {
force_left:
            if (new_x <= 170) {
                if (prev_scroll_x) { prev_scroll_x -= mario.vx; }
                else               { new_x -= mario.vx; }
            } else                 { new_x -= mario.vx; }
            
            if (prev_scroll_x < 0) { prev_scroll_x = 0; }
        }
    }
    
    if (!mario.momentum) {
        mario.curr_sprite = (mario.direction == FACE_RIGHT) ? mario_0_buffer_right : mario_0_buffer_left;
    }

skip_all:
    mario.y = new_y;
    mario.x = new_x;
    mario.scrollx = prev_scroll_x;
    mario.scrolly = new_scroll_y;
}