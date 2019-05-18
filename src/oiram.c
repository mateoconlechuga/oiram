#include <stdbool.h>
#include <stdint.h>

#include <graphx.h>
#include <keypadc.h>
#include <fileioc.h>

#include "tile_handlers.h"
#include "defines.h"
#include "powerups.h"
#include "enemies.h"
#include "events.h"
#include "images.h"
#include "lower.h"
#include "oiram.h"
#include "simple_mover.h"

bool pressed_left = false;
bool pressed_right = false;
bool pressed_up = false;
bool pressed_down = false;
bool pressed_alpha = false;
bool pressed_2nd = false;
bool allow_up_press = true;

// determine the scroll and offsets for the start posistion
void oiram_start_location(void) {
    if (oiram.x > 155 && (oiram.scrollx = oiram.x - 155) > level_map.max_x_scroll) {
        oiram.scrollx = level_map.max_x_scroll;
    }

    if (oiram.y > 80 && (oiram.scrolly = oiram.y - 80) > level_map.max_y_scroll) {
        oiram.scrolly = level_map.max_y_scroll;
    }
}

// sets the sprites
void set_normal_oiram_sprites(void) {
    gfx_sprite_t *img0;
    gfx_sprite_t *img1;
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

// handle picking up of a shell
static bool pickup_shell(void) {
    uint8_t j;

    // get absolute locations
    int abs_x = oiram.x;
    int abs_y = oiram.y;

    // don't recurse
    if (oiram.has_shell) {
        return false;
    }

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
static void drop_shell(void) {
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

static bool crouch_oiram(void) {
    if (oiram.flags & (FLAG_OIRAM_RACOON | FLAG_OIRAM_FIRE | FLAG_OIRAM_BIG)) {
        gfx_sprite_t *img0;
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

static void uncrouch_oiram(void) {
    if (oiram.flags & FLAG_OIRAM_BIG) {
        set_normal_oiram_sprites();
    }
    oiram.crouched = false;
}

static void oiram_location_from_offset(unsigned int offset) {
    unsigned int x, y;

    tile_to_abs_xy_pos(offset + tilemap.map, &x, &y);

    if (warp.exit_style == PIPE_DOWN || warp.exit_style == PIPE_UP) {
        x += 8;
    }
    if (warp.exit_style == DOOR_WARP && (oiram.flags & (FLAG_OIRAM_BIG | FLAG_OIRAM_FIRE))) {
        y += 3;
    }

    oiram.x = x;
    oiram.y = y;

    warp.clip_x = x;
    warp.clip_y = y;
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

static void scroll_map(void) {
    switch (level_map.scroll) {
        case SCROLL_RIGHT:
            oiram.scrollx++;
            if (oiram.scrollx > level_map.max_x_scroll) {
                oiram.scrollx = level_map.max_x_scroll;
            }
            break;
        case SCROLL_LEFT:
            if (oiram.scrollx) {
                oiram.scrollx--;
            }
            break;
        default:
            break;
    }
}

// move oiram around on the screen, checking for bounds
void move_oiram(void) {
    uint8_t new_vx, left_bottom_test, right_bottom_test;
    int8_t mm;

    int diff_y, prev_y;
    int new_y_top, new_y_bot, new_x_left, new_x_right;

    // scroll the tilemap if needed
    if (level_map.scroll) {
        scroll_map();
    }

    // load variables
    new_vx = oiram.vx;
    prev_y = oiram.y;
    new_y_top = prev_y;
    new_y_bot = new_y_top + oiram.hitbox.height;
    new_x_left = oiram.x;
    new_x_right = new_x_left + OIRAM_HITBOX_WIDTH;
    mm = oiram.momentum;

    test_y_ptr = &new_y_top;
    test_y_height = oiram.hitbox.height;

    oiram.rel_x = new_x_left - oiram.scrollx;
    oiram.rel_y = new_y_top - oiram.scrolly;

    // reset states
    on_slope = TEST_NONE;
    in_quicksand = false;
    in_water = false;
    on_ice = false;
    oiram.flags &= ~FLAG_OIRAM_SLIDE;

    // do something else if someone died
    if (oiram.failed) {
        static int fail_y;
        if (!oiram.started_fail) {
            oiram.vy = -9;
EXECUTE_FAIL:
            oiram.fail_x = oiram.x;
            oiram.fail_y = fail_y = oiram.y;
            oiram.y = -500;
            oiram.x = -500;
            oiram.has_shell = false;
            oiram.started_fail = true;
        }
        oiram.fail_y += oiram.vy;
        if (oiram.vy < 3) {oiram.vy++; }
        if (oiram.fail_y > fail_y + 65) {
            game.exit = true;
        }
        return;
    }

    if (warp.style != WARP_NONE) {
        oiram.vy = 0; mm = 0;
        switch (warp.style) {
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

        warp.count--;
        if (!warp.count) {
            if (warp.enter) {
                if (game.enter_end) {
                    game.exit = true;
                    return;
                }
                warp.enter = false;
                oiram_location_from_offset(warp.exit_loc);
                if (warp.exit_style == DOOR_WARP) {
                    warp.exit = false;
                    warp.style = WARP_NONE;
                    new_y_top = oiram.y;
                    new_x_left = oiram.x;
                    oiram.rel_x = new_x_left - oiram.scrollx;
                    oiram.rel_y = new_y_top - oiram.scrolly;
                    goto HANDLE_NO_X_SCROLL;
                }
                warp.exit = true;
                warp.style = warp.exit_style;
                if (warp.exit_style == PIPE_DOWN) {
                    warp.count = TILE_HEIGHT;
                    warp.clip_y += TILE_HEIGHT;
                } else
                if (warp.exit_style == PIPE_UP) {
                    warp.count = oiram.hitbox.height + 1;
                } else {
                    if (oiram.flags & (FLAG_OIRAM_BIG | FLAG_OIRAM_FIRE)) {
                        oiram.y += 2;
                    }
                    if (warp.exit_style == PIPE_LEFT) {
                        warp.clip_x += OIRAM_HITBOX_WIDTH;
                    }
                    warp.count = OIRAM_HITBOX_WIDTH + 1;
                }
            } else {
                warp.exit = false;
                warp.style = WARP_NONE;
            }
        }
        new_y_top = oiram.y;
        new_x_left = oiram.x;
        oiram.rel_x = new_x_left - oiram.scrollx;
        oiram.rel_y = new_y_top - oiram.scrolly;
    } else {
        // check the tops when falling
        move_side = TILE_TOP;

        right_bottom_test = moveable_tile_right_bottom(new_x_right, new_y_bot + 1);
        left_bottom_test  = moveable_tile_left_bottom(new_x_left, new_y_bot + 1);

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
            oiram.score_chain = 0;
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
            if (!oiram.fly_count) {
                pressed_2nd = false;
            }
        }

        // make sure we aren't crouching
        if (oiram.crouched) {
            pressed_right = false;
            pressed_left = false;
            goto HANDLE_REDUCED_SPEED;
        } else {
            if (pressed_left || pressed_right) {
                if (pressed_2nd) {
                    gfx_FillRectangle_NoClip(118, 146, abs(mm)*2, 2);
HANDLE_DOWN:
                    on_slope = TEST_NONE;

                    // increase momentum in the x direction
                    if (oiram.direction == FACE_RIGHT) {
                        if (mm < 0)   { mm = 0; add_poof(new_x_left, new_y_bot); }
                        if (mm < 5)   { new_vx = 2; } else
                        if (mm > 20)  { new_vx = 5; oiram.index ^= 1; } else
                        if (mm > 10)  { new_vx = 4; oiram.index ^= 1; } else
                        if (mm > 5)   { new_vx = 3; }
                        if (mm < 40)  { mm++; }
                    } else {
                        if (mm > 0)   { mm = 0; add_poof(new_x_right, new_y_bot); }
                        if (mm > -5)  { new_vx = 2; } else
                        if (mm < -20) { new_vx = 5; oiram.index ^= 1; } else
                        if (mm < -10) { new_vx = 4; oiram.index ^= 1; } else
                        if (mm < -5)  { new_vx = 3; }
                        if (mm > -40) { mm--; }
                    }
                } else {
                    if (oiram.direction == FACE_RIGHT) {
                        if (mm < 0)   { mm = 0; }
                        else if (mm < 20)  { mm += 3; }
                    } else {
                        if (mm > 0)   { mm = 0; }
                        else if (mm > -20) { mm -= 3; }
                    }
                    goto HANDLE_REDUCED_SPEED;
                }
            } else {
HANDLE_REDUCED_SPEED:
                if (mm < 0) {
                    if ((mm += 2-(uint8_t)on_ice) >= 0) { mm = 0; }
                } else {
                    if ((mm -= 2-(uint8_t)on_ice) < 0) { mm = 0; }
                }
            }

            gfx_BlitLines(gfx_buffer, 146, 2);
        }

        // check if no momentum and set sprites
        if (!mm) {
            if (oiram.direction == FACE_RIGHT) {
                oiram.sprite = oiram_0_buffer_right;
            } else {
                oiram.sprite = oiram_0_buffer_left;
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

                        add_fireball(new_x_left + add_x, new_y_top + oiram.hitbox.height/2 - 2, add_dir, OIRAM_FIREBALL);
                        oiram.fireballs++;
                    }
                }
            }
            pressed_alpha = false;
        }

        move_side = TILE_SOLID;

        // if inside a tile, force us out of it
        if (!moveable_tile(new_x_right, new_y_top) || !moveable_tile(new_x_left, new_y_top)) {
            if ((level_map.scroll == SCROLL_RIGHT && oiram.rel_x <= 0) || (level_map.scroll == SCROLL_LEFT && oiram.rel_x >= 305)) {
                oiram.vy = -9;
                goto TOTAL_FAIL;
            }
            if (!in_quicksand) {
                if (oiram.direction == FACE_LEFT) {
                    new_x_right--;
                    new_x_left--;
                } else {
                    new_x_right++;
                    new_x_left++;
                }
                goto HANDLE_SKIP_UP;
            }
        }

        // an up event was triggered
        if (pressed_up) {
            // check if there is a door we can go through
            move_side = TILE_TEST_DOOR_UP;
            moveable_tile(new_x_left + 4, new_y_top - 2);
            if (warp.style) {
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
                            goto HANDLE_NORMAL_JUMP;
                        }
                    }

                    goto HANDLE_SKIP_FORCE_UP;
                }

HANDLE_NORMAL_JUMP:
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
HANDLE_SKIP_FORCE_UP:
                pressed_up = false;
                allow_up_press = false;
            }
        }

HANDLE_SKIP_UP:
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
                oiram.index = 0;
                oiram.flags |= FLAG_OIRAM_SLIDE;
                goto HANDLE_DOWN;
            }
            if (!(oiram.flags & FLAG_OIRAM_SLIDE)) {
                if (oiram.on_vine) {
                    oiram.vy = 2;
                } else if (!oiram.crouched) {
                    move_side = TILE_TEST_PIPE_DOWN;
                    moveable_tile(new_x_left, new_y_bot + 1);
                    if (!warp.style) {
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
                new_y_top -= 12;
                new_y_bot -= 12;
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

                for(; (unsigned)oiram.vy > 0; oiram.vy--) {
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
            if (oiram.index) {
                oiram.sprite = oiram_0_buffer_right;
            } else {
                oiram.sprite = oiram_1_buffer_right;
            }

            // check the left of the tile
            move_side = TILE_LEFT;
            thalf = new_y_top + oiram.hitbox_height_half;

            for(; new_vx > 0; new_vx--) {
                int tx = new_x_right + new_vx;
                if (moveable_tile_right_bottom(tx, new_y_bot) && moveable_tile(tx, new_y_top) && moveable_tile(tx, thalf)) {
                    goto HANDLE_SET_RIGHT;
                }
            }
            mm = 0;
HANDLE_SET_RIGHT:
            new_x_left += new_vx;

            if (mm < 0) {
                goto HANDLE_SKIP_LEFT;
            }
        }

        if (pressed_left || mm < 0) {
            int thalf;

            oiram.direction = FACE_LEFT;
            if (oiram.index) {
                oiram.sprite = oiram_0_buffer_left;
            } else {
                oiram.sprite = oiram_1_buffer_left;
            }

            // check the right of the tile
            move_side = TILE_RIGHT;
            thalf = new_y_top + oiram.hitbox_height_half;

            for(; new_vx > 0; new_vx--) {
                int tx = new_x_left - new_vx;
                if (moveable_tile_left_bottom(tx, new_y_bot) && moveable_tile(tx, new_y_top) && moveable_tile(tx, thalf)) {
                    goto HANDLE_SET_LEFT;
                }
            }
            mm = 0;
HANDLE_SET_LEFT:
            new_x_left -= new_vx;
        }
    }

HANDLE_SKIP_LEFT:

    if (level_map.scroll == SCROLL_NONE || (level_map.scroll != SCROLL_LEFT && level_map.scroll != SCROLL_RIGHT)) {
HANDLE_NO_X_SCROLL:
        if (new_x_left > 155) {
            if ((oiram.scrollx = new_x_left - 155) > level_map.max_x_scroll) {
                oiram.scrollx = level_map.max_x_scroll;
            }
        } else {
            oiram.scrollx = 0;
        }
    } else {
        int xoff;
        if (warp.exit_style == DOOR_WARP || (warp.style != WARP_NONE && warp.exit)) {
            if (warp.exit_style == DOOR_WARP) {
                warp.exit_style = WARP_NONE;
            }
            goto HANDLE_NO_X_SCROLL;
        }

        xoff = new_x_left - (int)oiram.scrollx;
        if (xoff < 0) {
            oiram.rel_x = 0;
            new_x_left = oiram.scrollx;
        } else if (xoff > 305) {
            oiram.rel_x = 305;
            new_x_left = oiram.scrollx + 305;
        }
    }

    if ((diff_y = (new_y_top - prev_y)) >= 0) {
        if (oiram.rel_y >= 80 && (oiram.scrolly += diff_y) > level_map.max_y_scroll) {
            oiram.scrolly = level_map.max_y_scroll;
        }
    } else {
        if (oiram.rel_y <= 20 && (oiram.scrolly += diff_y) > level_map.max_y_scroll) {
            oiram.scrolly = 0;
        }
    }

    oiram.x = new_x_left;
    oiram.y = new_y_top;
    oiram.vx = new_vx;
    oiram.momentum = mm;

    if (new_y_top > level_map.max_y) {
TOTAL_FAIL:
        oiram.flags = FLAG_OIRAM_RESET;
        oiram.failed = true;
        goto EXECUTE_FAIL;
    }

    if (oiram.spin_count) {
        oiram.spin_count--;
        if (oiram.sprite == oiram_0_buffer_right) {
            oiram.sprite = oiram_0_buffer_left;
        } else {
            oiram.sprite = oiram_0_buffer_right;
        }
        if (!oiram.spin_count) {
            pressed_alpha = false;
        }
    }
}
