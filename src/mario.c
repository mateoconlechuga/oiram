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
    int abs_x = mario.x;
    int abs_y = mario.y;
    
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
    shell->y = ((mario.flags & FLAG_MARIO_BIG) ? mario.y + 26/2 - 4 : mario.y) + mario.scrolly;
    if (mario.direction == FACE_LEFT) {
        shell->vx = -3;
        shell->x = -16;
    } else {
        shell->vx = 3;
        shell->x = mario.hitbox.width;
    }
    shell->x += mario.x;
    shell->hitbox.height = 15;
    shell->hitbox.width = 15;
    if (mario.has_red_shell) {
        shell->sprite = koopa_red_shell_0;
        shell->type = KOOPA_RED_SHELL_TYPE;
    } else {
        shell->sprite = koopa_green_shell_0;
        shell->type = KOOPA_GREEN_SHELL_TYPE;
    }
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
    while (mario.scrolly + new_y != y) {
        if (new_y >= 65) {
            if (mario.scrolly < level_map.max_y_offset)  { mario.scrolly++; }
            else                                        { new_y++; }
        } else                                          { new_y++; }
    }
    
    if (mario.scrolly > level_map.max_y_offset) mario.scrolly = level_map.max_y_offset;
    
    // move right
    while (new_scroll_x + new_x != x) {
        if (new_x >= 150) {
            if (new_scroll_x < level_map.max_x_offset) { new_scroll_x++; }
            else                                       { new_x++; }
        } else                                         { new_x++; }
    }
    
    if (new_scroll_x > level_map.max_x_offset) { new_scroll_x = level_map.max_x_offset; }
    
    mario.scrollx = new_scroll_x;
    mario.scrolly = mario.scrolly;
    mario.x = new_x;
    mario.y = new_y;
    
    mario.pipe_clip_y = new_y;
    mario.pipe_clip_x = new_x;
}

// move mario around on the screen, checking for bounds
void move_mario(void) {
    uint8_t new_vx = mario.vx;
    uint8_t left_bottom_test,right_bottom_test;
    
    int diff_y;
    int prev_y = mario.y;
    
    int new_y_top = prev_y;
    int new_y_bot = new_y_top + mario.hitbox.height;
    int new_x_left = mario.x;
    int new_x_right = new_x_left + mario.hitbox.width;
    
    int tmp_y;
    int8_t mm = mario.momentum;
    
    test_y_ptr = &new_y_top;
    test_y_height = mario.hitbox.height;
    
    mario.rel_x = new_x_left - mario.scrollx;
    mario.rel_y = new_y_top - mario.scrolly;
    
    on_slope = TEST_NONE;
    in_quicksand = false;
    in_water = false;
    mario.flags &= ~FLAG_MARIO_SLIDE;
    
    // do something else if someone died
    if (someone_died) {
        game.exit = true;
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
                if (game.entered_end_pipe) {
                    game.exit = true;
                    return;
                }
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
                        mario.curr_sprite = mario_right[mario.sprite_index];
                    } else {
                        mario.curr_sprite = mario_left[mario.sprite_index];
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
        if (pressed_left || pressed_right || pressed_down) {
            if (pressed_yequ || (pressed_down && on_slope)) {
                
                // increase momentum in the x direction
                if (mario.direction == FACE_RIGHT) {
                    if (mm < 0)   { mm = 0; add_poof(new_x_left, new_y_bot); }
                    if (mm < 5)   { new_vx = 2; } else
                    if (mm > 20)  { new_vx = 5; mario.sprite_index ^= 1; } else
                    if (mm > 10)  { new_vx = 4; mario.sprite_index ^= 1; } else
                    if (mm > 5)   { new_vx = 3; }
                    if (mm < 40)  { mm++; }
                } else {
                    if (mm > 0)   { mm = 0; add_poof(new_x_right, new_y_bot); }
                    if (mm > -5)  { new_vx = 2; } else
                    if (mm < -20) { new_vx = 5; mario.sprite_index ^= 1; } else
                    if (mm < -10) { new_vx = 4; mario.sprite_index ^= 1; } else
                    if (mm < -5)  { new_vx = 3; }
                    if (mm > -40) { mm--; }
                }
            } else {
            
                if (mario.direction == FACE_RIGHT) {
                    if (mm < 0)   { mm = 0; } else
                    if (mm < 20)  { mm += 3; }
                } else {
                    if (mm > -20) { mm -= 3; } else
                    if (mm > 0)   { mm = 0; }
                }
                goto handle_reduced_speed;
            }
        } else {
handle_reduced_speed:
            new_vx = 2;
            
            if (mm < 0) {
                if ((mm += 2) > 0) { mm = 0; }
            } else {
                if ((mm -= 2) < 0) { mm = 0; }
            }
        }
        mario.momentum = mm;
    }
    
    // drop a shell or throw a fireball
    if (pressed_2nd) {
        if (mario.has_shell) {
            drop_shell();
        } else if ((mario.flags & FLAG_MARIO_FIRE) && !pickup_shell() && mario.fireballs < 2) {
            int add_x, add_y;
            uint8_t add_dir;
            fireball_t *ball;
            
            if (mario.direction == FACE_LEFT) {
                add_x = 0;
                add_dir = DOWN_LEFT;
            } else {
                add_x = mario.hitbox.width - 1;
                add_dir = DOWN_RIGHT;
            }
            
            ball = add_fireball(new_x_left + add_x, new_y_top + mario.hitbox.height/2, add_dir);
            ball->type = MARIO_FIREBALL;
            mario.fireballs++;
        }
        pressed_2nd = false;
    }
    
    // check the tops when falling
    move_side = TILE_TOP;

    tmp_y = new_y_bot + 1;
    
    right_bottom_test = moveable_tile_right_bottom(new_x_right, tmp_y);
    left_bottom_test  = moveable_tile_left_bottom(new_x_left, tmp_y);
    
    // if nothing below, start accelerating
    if (left_bottom_test || right_bottom_test) {
        if (left_bottom_test != 2 && right_bottom_test != 2) {
            if (mario.on_vine) {
                mario.on_vine = false;
                if (mario.flags & FLAG_MARIO_FIRE) {
                    memcpy(mario_0_buffer_right, mario_0_fire, MARIO_BIG_SPRITE_SIZE);
                    memcpy(mario_1_buffer_right, mario_1_fire, MARIO_BIG_SPRITE_SIZE);
                } else if (mario.flags & FLAG_MARIO_BIG) {
                    memcpy(mario_0_buffer_right, mario_0_big, MARIO_BIG_SPRITE_SIZE);
                    memcpy(mario_1_buffer_right, mario_1_big, MARIO_BIG_SPRITE_SIZE);
                } else {
                    memcpy(mario_0_buffer_right, mario_0_small, MARIO_SMALL_SPRITE_SIZE);
                    memcpy(mario_1_buffer_right, mario_1_small, MARIO_SMALL_SPRITE_SIZE);
                }
                set_left_mario_sprites();
            }
        }
        if (mario.vy < 9) { mario.vy++; }
    } else {
        mario.score_counter = 0;
    }
    
    if (mario.on_vine) { mario.vy = 0; }
    
    // pressing up triggers a jump
    if (pressed_up) {
        if (mario.on_vine) {
            mario.vy = -2;
        } else {
            pressed_up = false;
            // if forced jump added from bouncing on music blocks
            if (force_jump) {
                mario.vy += -9;
                force_jump = false;
            // check if there is something below before jumping
            } else if (!(left_bottom_test && right_bottom_test) || in_water) {
                if (in_quicksand || in_water) { 
                    mario.vy = -6;
                } else {
                    mario.vy = -11;
                }
            }
        }
    }
    
    if (pressed_down) {
        if (on_slope != TEST_NONE) {
            if (on_slope == TEST_RIGHT) {
                pressed_left = true;
                add_poof(new_x_right, new_y_bot);
            } else {
                pressed_right = true;
                add_poof(new_x_left, new_y_bot);
            }
            mario.sprite_index = 0;
            mario.flags |= FLAG_MARIO_SLIDE;
        }
        if (!mario.crouched) {
            move_side = TILE_TEST_PIPE_DOWN;
            moveable_tile(new_x_left, new_y_bot + 1);
            if (mario.in_pipe) {
                goto no_crouch_allowed;
            }
        }
        if (mario.on_vine) {
            mario.vy = 2;
        } else if (!mario.crouched) {
            crouch_mario();
            if (mario.crouched) {
                new_y_top += 11; new_y_bot += 11;
            }
        }
    } else {
        if (mario.crouched) {
            uncrouch_mario();
            new_y_top -= 11; new_y_bot -= 11;
        }
    }
no_crouch_allowed:

    // check if velocity change
    if (mario.vy) {
        int ty;
        
        // moving up
        if (mario.vy < 0) {
            
            // check the bottom of the tile
            move_side = TILE_BOTTOM;
            
            // binary test until we find the new thing -- bitwise or because need both sides
            while(!(moveable_tile(new_x_right, new_y_top + mario.vy) & moveable_tile(new_x_left, new_y_top + mario.vy))) {
                if ((int8_t)(mario.vy /= 2) >= 0) { break; }
            }

        // moving down
        } else {
            
            // check the top of the tile
            move_side = TILE_TOP;
            
            for(; mario.vy > 0; mario.vy--) {
                ty = new_y_bot + mario.vy;
                if (moveable_tile_right_bottom(new_x_right, ty) & moveable_tile_left_bottom(new_x_left, ty)) {
                    break;
                }
            }
        }
        new_y_top += mario.vy;
        new_y_bot += mario.vy;
    }
	
    if (pressed_right || mario.momentum > 0) {
        
        mario.direction = FACE_RIGHT;
        if (mario.sprite_index) {
            mario.curr_sprite = mario_0_buffer_right;
        } else {
            mario.curr_sprite = mario_1_buffer_right;
        }
        
        // check the left of the tile
        move_side = TILE_LEFT;
        
        for(; new_vx > 0; new_vx--) {
            int tx = new_x_right + new_vx;
            if (moveable_tile_right_bottom(tx, new_y_bot) && moveable_tile(tx, new_y_top) && moveable_tile(tx, new_y_top + mario.hitbox_height_half)) {
                break;
            }
        }
        
        new_x_left += new_vx;
        
        if (mario.momentum < 0) {
            goto skip_left;
        }
    }
    
    if (pressed_left || mario.momentum < 0) {
        
        mario.direction = FACE_LEFT;
        if (mario.sprite_index) {
            mario.curr_sprite = mario_0_buffer_left;
        } else {
            mario.curr_sprite = mario_1_buffer_left;
        }
    
        // check the right of the tile
        move_side = TILE_RIGHT;

        for(; new_vx > 0; new_vx--) {
            int tx = new_x_left - new_vx;
            if (moveable_tile_left_bottom(tx, new_y_bot) && moveable_tile(tx, new_y_top) && moveable_tile(tx, new_y_top + mario.hitbox_height_half)) {
                break;
            }
        }
        
        new_x_left -= new_vx;
    }
    
skip_left:

    if (!mario.momentum) {
		if (mario.direction == FACE_RIGHT) {
			mario.curr_sprite = mario_0_buffer_right; 
		} else {
			mario.curr_sprite = mario_0_buffer_left;
		}
    }
    
    if (new_x_left > 155) {
        if ((mario.scrollx = new_x_left - 155) > level_map.max_x_offset) { mario.scrollx = level_map.max_x_offset; }
    } else {
        mario.scrollx = 0;
    }
    
    if ((diff_y = (new_y_top - prev_y)) < 0) {
        if (mario.rel_y <= 20) {
            if ((mario.scrolly += diff_y) > level_map.max_y_offset) { mario.scrolly = 0; }
        }
    } else {
        if (mario.rel_y >= 80) {
            if ((mario.scrolly += diff_y) > level_map.max_y_offset) { mario.scrolly = level_map.max_y_offset; }
        }
    }
    
    mario.x = new_x_left;
    mario.y = new_y_top;
    mario.vx = new_vx;
}
