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
#include <graphx.h>
#include <keypadc.h>
#include <fileioc.h>

// oiram stuffs
#include "tile_handlers.h"
#include "defines.h"
#include "powerups.h"
#include "enemies.h"
#include "events.h"
#include "images.h"
#include "lower.h"
#include "simple_mover.h"

bool pressed_left = false;
bool pressed_right = false;
bool pressed_up = false;
bool pressed_down = false;
bool pressed_alpha = false;
bool pressed_2nd = false;
bool allow_up_press = true;

void compute_oiram_start_location(void) {
    int new_x_left = oiram.x;
    int new_y_top = oiram.y;
    
    if (new_x_left > 155) {
        if ((oiram.scrollx = new_x_left - 155) > level_map.max_x_scroll) { oiram.scrollx = level_map.max_x_scroll; }
    }
    
    if (new_y_top > 80) {
        if ((oiram.scrolly = new_y_top - 80) > level_map.max_y_scroll) { oiram.scrolly = level_map.max_y_scroll; }
    }
}

// handle picking up of a shell
bool pickup_shell(void) {
    uint8_t j;
    
    // get absolute locations
    int abs_x = oiram.x;
    int abs_y = oiram.y;
    
    // don't recurse
    if (oiram.has_shell) { return false; }
    
    // locate shell location
    if (oiram.direction == FACE_LEFT) {
        abs_x -= 24;
    } else {
        abs_x += OIRAM_HITBOX_WIDTH;
    }
    
    // check if there is a shell near oiram
    for(j = 0; j < num_simple_movers; j++) {
        simple_move_t *chk = simple_mover[j];
        uint8_t chk_type = chk->type;
        
        if ((chk_type == KOOPA_RED_SHELL_TYPE || chk_type == KOOPA_GREEN_SHELL_TYPE) && !chk->vx) {
            if (gfx_CheckRectangleHotspot(abs_x, abs_y, 24, oiram.hitbox.height, chk->x, chk->y, 15, 15)) {
                remove_simple_mover(j);
                oiram.has_shell = true;
                oiram.has_red_shell = chk_type == KOOPA_RED_SHELL_TYPE;
                return true;
            }
        }
    }
    
    // return false if we can't pick up a shell
    return false;
}

// drops a shell if oiram is holding one
void drop_shell(void) {
    simple_move_t *shell = add_simple_mover(NULL);
    shell->y = ((oiram.flags & FLAG_OIRAM_BIG) ? oiram.y + 26/2 - 4 : oiram.y);
    if (oiram.direction == FACE_LEFT) {
        shell->vx = -3;
        shell->x = -16;
    } else {
        shell->vx = 3;
        shell->x = OIRAM_HITBOX_WIDTH;
    }
    shell->x += oiram.x;
    shell->hitbox.height = 15;
    shell->hitbox.width = 15;
    if (oiram.has_red_shell) {
        shell->sprite = koopa_red_shell_0;
        shell->type = KOOPA_RED_SHELL_TYPE;
    } else {
        shell->sprite = koopa_green_shell_0;
        shell->type = KOOPA_GREEN_SHELL_TYPE;
    }
    oiram.has_shell = false;
}
                
// crouching is fun
static bool crouch_oiram(void) {
    // check if we are big
    if (oiram.flags & (FLAG_OIRAM_RACOON | FLAG_OIRAM_FIRE | FLAG_OIRAM_BIG)) {
        gfx_image_t *img0;
        if (oiram.flags & FLAG_OIRAM_RACOON) {
            img0 = oiram_crouch_racoon;
        } else if (oiram.flags & FLAG_OIRAM_FIRE) {
            img0 = oiram_crouch_fire;
        } else {
            img0 = oiram_crouch_big;
        }
        memcpy(oiram_0_buffer_right, img0, OIRAM_BIG_SPRITE_SIZE);
        memcpy(oiram_1_buffer_right, img0, OIRAM_BIG_SPRITE_SIZE);
        set_left_oiram_sprites();
        
        oiram.hitbox.height = OIRAM_SMALL_HITBOX_HEIGHT;
        oiram.hitbox_height_half = OIRAM_SMALL_HITBOX_HEIGHT/2;
        oiram.crouched = true;
        return true;
    }
    return false;
}

void set_normal_oiram_sprites(void) {
    gfx_image_t *img0;
    gfx_image_t *img1;
    if (oiram.flags & FLAG_OIRAM_RACOON) {
        img0 = oiram_0_racoon;
        img1 = oiram_1_racoon;
    } else if (oiram.flags & FLAG_OIRAM_FIRE) {
        img0 = oiram_0_fire;
        img1 = oiram_1_fire;
    } else if (oiram.flags & FLAG_OIRAM_BIG) {
        img0 = oiram_0_big;
        img1 = oiram_1_big;
    } else {
        img0 = oiram_0_small;
        img1 = oiram_1_small;
    }
    if (oiram.flags & FLAG_OIRAM_BIG) {
        oiram.hitbox.height = OIRAM_BIG_HITBOX_HEIGHT;
        oiram.hitbox_height_half = OIRAM_BIG_HITBOX_HEIGHT/2;
    } else {
        oiram.hitbox.height = OIRAM_SMALL_HITBOX_HEIGHT;
        oiram.hitbox_height_half = OIRAM_SMALL_HITBOX_HEIGHT/2;
    }
    memcpy(oiram_0_buffer_right, img0, OIRAM_BIG_SPRITE_SIZE);
    memcpy(oiram_1_buffer_right, img1, OIRAM_BIG_SPRITE_SIZE);
    set_left_oiram_sprites();
}

void uncrouch_oiram(void) {
    if ((oiram.flags & FLAG_OIRAM_BIG)) {
        set_normal_oiram_sprites();
    }
    oiram.crouched = false;
}

void compute_oiram_location_from_offset(unsigned int offset) {
    unsigned int x, y;
    
    tile_to_abs_xy_pos(offset + tilemap.map, &x, &y);

    if (oiram.exit_pipe_dir == PIPE_DOWN || oiram.exit_pipe_dir == PIPE_UP) {
        x += 8;
    }
    if (oiram.exit_pipe_dir == DOOR_WARP && (oiram.flags & (FLAG_OIRAM_BIG | FLAG_OIRAM_FIRE))) {
        y += 3;
    }

    oiram.x = x;
    oiram.y = y;
    
    oiram.pipe_clip_x = x;
    oiram.pipe_clip_y = y;
}

static void spin_racoon_mario(int x, int y) {
    uint8_t j;
    
    for(j = 0; j < num_simple_movers; j++) {
        simple_move_t *hit = simple_mover[j];
        
        if (hit->type > HITABLE_TYPES) {
            if (gfx_CheckRectangleHotspot(x, y, 8, 14, hit->x, hit->y, hit->hitbox.width, hit->hitbox.height)) {
                add_score(1, x, y);
                add_poof(hit->x + 4, hit->y + 4);
                remove_simple_mover(j);
                j = -1;
            }
        }
    }
    
    for(j = 0; j < num_chompers; j++) {
        chomper_t *hit = chomper[j];
        if (y + 13 < hit->start_y) {
            if (gfx_CheckRectangleHotspot(x, y, 8, 14, hit->x, hit->y, 15, 30)) {
                add_score(1, x, y);
                add_poof(hit->x + 4, y);
                remove_chomper(j);
                break;
            }
        }
    }
    
    move_side = TILE_RACOON_POWER;
    moveable_tile(x, y);
}

static void spin(int x, int y) {
    int tycoon = y + 13;
    spin_racoon_mario(x - 8, tycoon);
    spin_racoon_mario(x + 16, tycoon);
    oiram.spin_count = 3;
}

// move oiram around on the screen, checking for bounds
void move_oiram(void) {
    uint8_t new_vx = oiram.vx;
    uint8_t left_bottom_test,right_bottom_test;
    
    static int start_y;
	
    int diff_y;
    int prev_y = oiram.y;
    
    int new_y_top = prev_y;
    int new_y_bot = new_y_top + oiram.hitbox.height;
    int new_x_left = oiram.x;
    int new_x_right = new_x_left + OIRAM_HITBOX_WIDTH;
    
    int tmp_y;
    int8_t mm = oiram.momentum;
    
    test_y_ptr = &new_y_top;
    test_y_height = oiram.hitbox.height;
    
    oiram.rel_x = new_x_left - oiram.scrollx;
    oiram.rel_y = new_y_top - oiram.scrolly;
    
    on_slope = TEST_NONE;
    in_quicksand = false;
    in_water = false;
    
    // do something else if someone died
    if (oiram.failed) {
        if (!oiram.started_fail) {
            oiram.fail_x = oiram.x;
            oiram.fail_y = start_y = oiram.y;
            oiram.has_shell = false;
            oiram.y = -500;
            oiram.x = -500;
            oiram.vy = -9;
            oiram.started_fail = true;
        }
        oiram.fail_y += oiram.vy;
        if (oiram.vy < 3) {oiram.vy++; }
        if (oiram.fail_y > start_y + 50) {
            game.exit = true;
        }
        return;
    }
    
    if (oiram.in_pipe) {
        unsigned int x, y;
        oiram.vy = 0;
        switch (oiram.in_pipe) {
            case PIPE_DOWN:
                oiram.y++;
                break;
            case PIPE_LEFT:
                oiram.x++;
                break;
            case PIPE_RIGHT:
                oiram.x--;
                break;
            case PIPE_UP:
                oiram.y--;
                break;
            default:
                break;
        }
        
        oiram.pipe_counter--;
        if (!oiram.pipe_counter) {
            if (oiram.enter_pipe) {
                if (game.entered_end_pipe) {
                    game.exit = true;
                    return;
                }
                oiram.enter_pipe = false;
                oiram.exit_pipe = true;
                oiram.in_pipe = oiram.exit_pipe_dir;
                compute_oiram_location_from_offset(oiram.exit_pipe_loc);
                if (oiram.exit_pipe_dir == DOOR_WARP) {
                    oiram.exit_pipe = false;
                    oiram.in_pipe = WARP_NONE;
                    oiram.vy = 0;
                }
                if (oiram.exit_pipe_dir == PIPE_DOWN) {
                    oiram.pipe_counter = TILE_HEIGHT;
                    oiram.pipe_clip_y += TILE_HEIGHT;
                } else
                if (oiram.exit_pipe_dir == PIPE_UP) {
                    oiram.pipe_counter = oiram.hitbox.height + 1;
                } else {
                    if (oiram.flags & (FLAG_OIRAM_BIG | FLAG_OIRAM_FIRE)) {
                        oiram.y+=2;
                    }
                    if (oiram.exit_pipe_dir == PIPE_LEFT) {
                        oiram.pipe_clip_x += OIRAM_HITBOX_WIDTH;
                    }
                    oiram.pipe_counter = OIRAM_HITBOX_WIDTH + 1;
                }
            // exiting a pipe
            } else {
                oiram.exit_pipe = false;
                oiram.in_pipe = WARP_NONE;
            }
        }
        new_y_top = oiram.y;
        new_x_left = oiram.x;
        oiram.rel_x = new_x_left - oiram.scrollx;
        oiram.rel_y = new_y_top - oiram.scrolly;
    } else {
        // check the tops when falling
        move_side = TILE_TOP;

        tmp_y = new_y_bot + 1;
        
        right_bottom_test = moveable_tile_right_bottom(new_x_right, tmp_y);
        left_bottom_test  = moveable_tile_left_bottom(new_x_left, tmp_y);
        
        // if nothing below, start accelerating
        if (left_bottom_test || right_bottom_test) {
            if (left_bottom_test != 3 && right_bottom_test != 3) {
                if (oiram.on_vine) {
                    oiram.on_vine = false;
                    set_normal_oiram_sprites();
                }
            }
            if (oiram.vy < 8) { oiram.vy++; }
        } else {
            oiram.score_counter = 0;
            oiram.is_flying = false;
            oiram.fly_count = 9;
        }
        
        if (oiram.on_vine) {
            oiram.vy = 0;
        }
        
        if (oiram.is_flying) {
            gfx_SetColor(WHITE_INDEX);
            gfx_FillRectangle_NoClip(118, 146, 79, 2);
            new_vx = 5;
        } else {
            gfx_SetColor(252);
            gfx_FillRectangle_NoClip(119, 146, 79, 2);
            gfx_SetColor(WHITE_INDEX);
            new_vx = 2;
            if (!oiram.fly_count) { pressed_2nd = false; }
        }
        
        // make sure we aren't crouching
        if (oiram.crouched) {
            pressed_right = pressed_left = false;
            goto handle_other_reduced_speed;
        } else { 
            if (pressed_left || pressed_right) {
                if (pressed_2nd) {
                    gfx_FillRectangle_NoClip(118, 146, abs(mm)*2, 2);
    handle_down:
                    on_slope = TEST_NONE;
                    
                    // increase momentum in the x direction
                    if (oiram.direction == FACE_RIGHT) {
                        if (mm < 0)   { mm = 0; add_poof(new_x_left, new_y_bot); }
                        if (mm < 5)   { new_vx = 2; } else
                        if (mm > 20)  { new_vx = 5; oiram.sprite_index ^= 1; } else
                        if (mm > 10)  { new_vx = 4; oiram.sprite_index ^= 1; } else
                        if (mm > 5)   { new_vx = 3; }
                        if (mm < 40)  { mm++; }
                    } else {
                        if (mm > 0)   { mm = 0; add_poof(new_x_right, new_y_bot); }
                        if (mm > -5)  { new_vx = 2; } else
                        if (mm < -20) { new_vx = 5; oiram.sprite_index ^= 1; } else
                        if (mm < -10) { new_vx = 4; oiram.sprite_index ^= 1; } else
                        if (mm < -5)  { new_vx = 3; }
                        if (mm > -40) { mm--; }
                    }
                } else {
                    if (oiram.direction == FACE_RIGHT) {
                        if (mm < 0)   { mm = 0; } else
                        if (mm < 20)  { mm += 3; }
                    } else {
                        if (mm > -20) { mm -= 3; } else
                        if (mm > 0)   { mm = 0; }
                    }
    handle_other_reduced_speed:
                    goto handle_reduced_speed;
                }
            } else {
    handle_reduced_speed:
                if (mm < 0) {
                    if ((mm += 2) > 0) { mm = 0; }
                } else {
                    if ((mm -= 2) < 0) { mm = 0; }
                }
            }
            
            gfx_BlitLines(gfx_buffer, 146, 2);
        }
        
        // check if no momentum and set sprites
        if (!mm) {
            if (oiram.direction == FACE_RIGHT) {
                oiram.curr_sprite = oiram_0_buffer_right; 
            } else {
                oiram.curr_sprite = oiram_0_buffer_left;
            }
        }
        
        // drop a shell or throw a fireball
        if (pressed_alpha) {
            if (oiram.has_shell) {
                drop_shell();
            } else
            if (!pickup_shell()) {
                if ((oiram.flags & FLAG_OIRAM_RACOON) && !oiram.on_vine) {
                    if (!oiram.spin_count) {
                        spin(new_x_left, new_y_top);
                    }
                } else {
                    if ((oiram.flags & FLAG_OIRAM_FIRE) && oiram.fireballs < 2) {
                        int add_x;
                        uint8_t add_dir;
                        
                        if (oiram.direction == FACE_LEFT) {
                            add_x = 0;
                            add_dir = DOWN_LEFT;
                        } else {
                            add_x = OIRAM_HITBOX_WIDTH - 1;
                            add_dir = DOWN_RIGHT;
                        }
                        
                        add_fireball(new_x_left + add_x, new_y_top + oiram.hitbox.height/2, add_dir, OIRAM_FIREBALL);
                        oiram.fireballs++;
                    }
                }
            }
            pressed_alpha = false;
        }
        
        move_side = TILE_SOLID;

        // if inside a tile, force us out of it
        if (!moveable_tile(new_x_right, new_y_top) || !moveable_tile(new_x_left, new_y_top)) {
            if (!in_quicksand) {
                if (oiram.direction == FACE_LEFT) {
                    new_x_right--;
                    new_x_left--;
                } else {
                    new_x_right++;
                    new_x_left++;
                }
                goto skip_up;
            }
        }
        
        // an up event was triggered
        if (pressed_up) {
            // check if there is a door we can go through
            move_side = TILE_TEST_DOOR_UP;
            moveable_tile(new_x_left + 4, new_y_top - 1);
            if (oiram.in_pipe) {
                oiram.vy = 0;
            } else
            if (oiram.on_vine) {
                oiram.vy = -2;
            } else  {
                if ((oiram.flags & FLAG_OIRAM_RACOON)) {
                    if (oiram.vy > 1) {
                        spin(new_x_left, new_y_top);
                        oiram.vy = 1;
                    }
                    if (oiram.is_flying) {
                        oiram.vy = -7;
                        if (oiram.fly_count) {
                            oiram.fly_count--;
                        } else {
                            oiram.is_flying = false;
                        }
                    } else {
                        if (abs(mm) > 38) {
                            oiram.is_flying = true;
                            oiram.fly_count = 9;
                            oiram.vy = -9;
                        } else {
                            goto normal_jump;
                        }
                    }
                    
                    goto skip_force_up;
                }
                
        normal_jump:
                // if forced jump added from bouncing on music blocks
                if (force_jump) {
                    if (oiram.vy <= 1) {
                        oiram.vy = -11;
                    } else {
                        oiram.vy += -5;
                    }
                    force_jump = false;
                // check if there is something below before jumping
                } else if (!(left_bottom_test && right_bottom_test) || in_water) {
                    if (in_quicksand || in_water) { 
                        oiram.vy = -6;
                    } else if (oiram.vy <= 1) {
                        oiram.vy = -11;
                    }
                }
        skip_force_up:
                pressed_up = false;
                allow_up_press = false;
            }
        }
	    
        skip_up:
        
        if (pressed_down) {
            if (on_slope != TEST_NONE) {
                if (on_slope == TEST_RIGHT) {
                    pressed_left = true;
                    oiram.direction = FACE_LEFT;
                    add_poof(new_x_right, new_y_bot);
                } else {
                    pressed_right = true;
                    oiram.direction = FACE_RIGHT;
                    add_poof(new_x_left, new_y_bot);
                }
                oiram.sprite_index = 0;
                oiram.flags |= FLAG_OIRAM_SLIDE;
                goto handle_down;
            } else {
                oiram.flags &= ~FLAG_OIRAM_SLIDE;
            }
            if (!(oiram.flags & FLAG_OIRAM_SLIDE)) {
                if (oiram.on_vine) {
                    oiram.vy = 2;
                } else if (!oiram.crouched) {
                    move_side = TILE_TEST_PIPE_DOWN;
                    moveable_tile(new_x_left, new_y_bot + 1);
                    if (!oiram.in_pipe) {
                        crouch_oiram();
                        if ((oiram.flags & FLAG_OIRAM_BIG)) {
                            new_y_top += 11;
                        }
                    }
                }
            }
        } else {
            oiram.flags &= ~FLAG_OIRAM_SLIDE;
            if (oiram.crouched) {
                uncrouch_oiram();
                new_y_top -= 12; new_y_bot -= 12;
            }
        }
        
        // check if velocity change
        if (oiram.vy) {
            int ty;
            
            // moving up
            if (oiram.vy < 0) {
                
                // check the bottom of the tile
                move_side = TILE_BOTTOM;
                
                for(; oiram.vy < 0; oiram.vy++) {
                    ty = new_y_top + oiram.vy;
                    if (moveable_tile(new_x_right, ty) & moveable_tile(new_x_left, ty)) {
                        break;
                    }
                }

            // moving down
            } else {
                
                // check the top of the tile
                move_side = TILE_TOP;
                
                for(; oiram.vy > 0; oiram.vy--) {
                    ty = new_y_bot + oiram.vy;
                    if (moveable_tile_right_bottom(new_x_right, ty) & moveable_tile_left_bottom(new_x_left, ty)) {
                        break;
                    }
                }
            }
	    
            new_y_top += oiram.vy;
            new_y_bot += oiram.vy;
        }
        
        if (pressed_right || mm > 0) {
            int thalf;
            
            oiram.direction = FACE_RIGHT;
            if (oiram.sprite_index) {
                oiram.curr_sprite = oiram_0_buffer_right;
            } else {
                oiram.curr_sprite = oiram_1_buffer_right;
            }
            
            // check the left of the tile
            move_side = TILE_LEFT;
            thalf = new_y_top + oiram.hitbox_height_half;
            
            for(; new_vx > 0; new_vx--) {
                int tx = new_x_right + new_vx;
                if (moveable_tile_right_bottom(tx, new_y_bot) && moveable_tile(tx, new_y_top) && moveable_tile(tx, thalf)) {
                    goto set_new_right;
                }
            }
            mm = 0;
    set_new_right:
            new_x_left += new_vx;
            
            if (mm < 0) {
                goto skip_left;
            }
        }
        
        if (pressed_left || mm < 0) {
            int thalf;
            
            oiram.direction = FACE_LEFT;
            if (oiram.sprite_index) {
                oiram.curr_sprite = oiram_0_buffer_left;
            } else {
                oiram.curr_sprite = oiram_1_buffer_left;
            }
            
            // check the right of the tile
            move_side = TILE_RIGHT;
            thalf = new_y_top + oiram.hitbox_height_half;
            
            for(; new_vx > 0; new_vx--) {
                int tx = new_x_left - new_vx;
                if (moveable_tile_left_bottom(tx, new_y_bot) && moveable_tile(tx, new_y_top) && moveable_tile(tx, thalf)) {
                    goto set_new_left;
                }
            }
            mm = 0;
    set_new_left:
            new_x_left -= new_vx;
        }
    
    }
    
    skip_left:

    if (new_x_left > 155) {
        if ((oiram.scrollx = new_x_left - 155) > level_map.max_x_scroll) { oiram.scrollx = level_map.max_x_scroll; }
    } else {
        oiram.scrollx = 0;
    }
    
    if ((diff_y = (new_y_top - prev_y)) < 0) {
        if (oiram.rel_y <= 20) {
            if ((oiram.scrolly += diff_y) > level_map.max_y_scroll) { oiram.scrolly = 0; }
        }
    } else {
        if (oiram.rel_y >= 80) {
            if ((oiram.scrolly += diff_y) > level_map.max_y_scroll) { oiram.scrolly = level_map.max_y_scroll; }
        }
    }
    
    oiram.x = new_x_left;
    oiram.y = new_y_top;
    oiram.vx = new_vx;
    oiram.momentum = mm;
    
    if (new_y_top > level_map.max_y) {
        if (!oiram.started_fail) {
            oiram.fail_x = oiram.x;
            oiram.fail_y = start_y = oiram.y;
            oiram.has_shell = false;
            oiram.y = -500;
            oiram.x = -500;
            oiram.vy = 1;
            oiram.started_fail = true;
            oiram.failed = true;
            oiram.flags = FLAG_OIRAM_RESET;
        }
    }

    if (oiram.spin_count) {
        oiram.spin_count--;
        if (oiram.curr_sprite == oiram_0_buffer_right) {
            oiram.curr_sprite = oiram_0_buffer_left;
        } else {
            oiram.curr_sprite = oiram_0_buffer_right;
        }
        if (!oiram.spin_count) {
            pressed_alpha = false;
        }
    }
}

