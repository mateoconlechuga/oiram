#include "events.h"
#include "loadscreen.h"
#include "tile_handlers.h"
#include "defines.h"
#include "images.h"
#include "lower.h"
#include "oiram.h"
#include "powerups.h"
#include "enemies.h"
#include "simple_mover.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <graphx.h>
#include <keypadc.h>
#include <fileioc.h>

bool something_died = false;

// only handle if somewhat within view; otherwise we can just ignore it
bool in_viewport(int x, int y) {
    int t_x, t_y;

    if (oiram.failed) {
        t_x = oiram.fail_x;
        t_y = oiram.fail_y;
    } else {
        t_x = oiram.x;
        t_y = oiram.y;
    }
    if (x - 360 >= t_x) {
        return false;
    } else
    if (x + 360 <= t_x) {
        return false;
    }
    if (y - 160 >= t_y) {
        return false;
    } else
    if (y + 160 <= t_y) {
        return false;
    }
    return true;
}

// gloabl used to handle events that aren't oiram
bool handling_events;

void handle_pending_events(void) {
    uint8_t i;
    int x, y;
    int rel_x, rel_y;

    handling_events = true;

    oiram.rel_x = oiram.x - oiram.scrollx;
    oiram.rel_y = oiram.y - oiram.scrolly;

    if (num_thwomps) {
        for(i = 0; i < num_thwomps; i++) {
            thwomp_t *cur = thwomp[i];
            int8_t tmp_vy;

            x = cur->x;
            y = cur->y;

            if (!in_viewport(x, y)) {
                continue;
            }

            tmp_vy = cur->vy;
            rel_x = x - oiram.scrollx;
            rel_y = y - oiram.scrolly;

            gfx_TransparentSprite(thwomp_0, rel_x, rel_y);

            if (y == cur->start_y) {
                if (oiram.x >= x - 20 && oiram.x <= x + 20 + OIRAM_HITBOX_WIDTH) {
                    tmp_vy = 4;
                } else {
                    tmp_vy = 0;
                }
            } else {
                if (!tmp_vy) {
                    if (cur->count) {
                        cur->count--;
                    } else {
                        tmp_vy = -1;
                    }
                }
            }

            if (tmp_vy > 0) {
                int tmp_y;

                if (tmp_vy < 9) { tmp_vy++; }

                // test against bottom
                tmp_y = y + 31;

                // check bottom of tile
                move_side = TILE_TOP;

                // binary test until we find the new thing
                while(!(moveable_tile(x, tmp_y + tmp_vy) && moveable_tile(x + 23, tmp_y + tmp_vy))) {
                    if ((tmp_vy /= 2) <= 0) { break; }
                }

                if (!tmp_vy) {
                    cur->count = 10;
                    add_poof(x + 12, y + 25);
                    add_poof(x, y + 25);
                }
            }

            y += tmp_vy;

            if (oiram_collision(x, y, 23, 31)) {
                if (!shrink_oiram()) {
                    add_poof(oiram.x, oiram.y + 2);
                    remove_thwomp(i--);
                    continue;
                }
            }

            cur->vy = tmp_vy;
            cur->y = y;
        }
    }

    if (num_simple_movers) {
        for(i = 0; i < num_simple_movers; i++) {
            simple_move_t *cur = simple_mover[i];
            uint8_t type;

            x = cur->x;
            y = cur->y;

            if (!in_viewport(x, y)) {
                continue;
            }

            if (y > level_map.max_y) {
               if (cur->type == RESWOB_TYPE) {
                   unsigned int chk, max = tilemap.width * tilemap.height;
                   for (chk = 0; chk < max; chk++) {
                       if (tilemap.map[chk] == TILE_RESWOB_VANISH) {
                           tilemap.map[chk] = TILE_EMPTY;
                       }
                   }
               }
HANDLE_LOOP_FAIL:
               remove_simple_mover(i--);
               continue;
            }

            simple_move_handler(cur);

            x = cur->x;
            y = cur->y;

            if (something_died) {
                something_died = false;
                add_poof(x + 4, y + 4);
                goto HANDLE_LOOP_FAIL;
            }

            rel_x = x - oiram.scrollx;
            rel_y = y - oiram.scrolly;
            type = cur->type;

            if (cur->bumped) {
                switch(type) {
                    case GOOMBA_TYPE:
                        add_poof(x + 4, y + 4);
                        add_score(0, x, y);
                        goto HANDLE_LOOP_FAIL;
                    case KOOPA_RED_TYPE:
                    case KOOPA_GREEN_TYPE:
                    case KOOPA_BONES_TYPE:
                    case SPIKE_TYPE:
                        add_score(0, x, y);
                        goto HANDLE_CREATE_SHELL;
                    default:
                        cur->bumped = false;
                        break;
                }
            }

            if (!oiram_collision(x, y, cur->hitbox.width, cur->hitbox.height)) {
                gfx_sprite_t *img;
                gfx_rletsprite_t *rletimg;
                static uint8_t reswob_sprite_count = 0;
                static gfx_rletsprite_t *reswob_sprite;
                static bool reswob_is_jumping = false;
                static uint8_t reswob_force_fall = 0;
HANDLE_DRAW_SPRITE:
                switch(type) {
                    case MUSHROOM_TYPE:
                        img = mushroom;
                        break;
                    case MUSHROOM_1UP_TYPE:
                        img = mushroom_1up;
                        break;
                    case FIRE_FLOWER_TYPE:
                        img = fire_flower;
                        break;
                    case STAR_TYPE:
                        img = star_0;
                        break;
                    case RESWOB_TYPE:
                        if (!reswob_is_jumping) {
                            if (!reswob_sprite_count) {
                                if (cur->vx < 0) {
                                    if (reswob_sprite == reswob_left_0) {
                                        rletimg = reswob_left_1;
                                    } else {
                                        rletimg = reswob_left_0;
                                    }
                                } else {
                                    if (reswob_sprite == reswob_right_0) {
                                        rletimg = reswob_right_1;
                                    } else {
                                        rletimg = reswob_right_0;
                                    }
                                }
                                reswob_sprite = rletimg;
                            }
                            if (reswob_sprite_count++ == 5) { reswob_sprite_count = 0; }
                            if ((oiram.x >= x - 24 && oiram.x <= x + 40)) {
                                cur->vy = -11;
                                reswob_is_jumping = true;
                                reswob_sprite = reswob_down;
                            }
                        } else {
                            if (cur->vy == 0) {
                                reswob_force_fall = !reswob_force_fall;
                                if (!reswob_force_fall) {
                                    reswob_is_jumping = false;
                                    if (oiram.x < x) {
                                        cur->vx = -1;
                                        rletimg = reswob_left_0;
                                    } else {
                                        cur->vx = 1;
                                        rletimg = reswob_right_0;
                                    }
                                    move_side = TILE_RESWOB_DOWN;
                                    moveable_tile(x, y + 45);
                                    moveable_tile(x + 16, y + 45);
                                }
                            }
                        }
                        rletimg = reswob_sprite;
                        gfx_RLETSprite(rletimg, rel_x, rel_y);
                        goto HANDLE_SKIP_DRAW;
                    case GOOMBA_TYPE:
HANDLE_DRAW_SPRITE_GOOMBA:
                        img = goomba_sprite;
                        break;
                    case FLAT_GOOMBA_TYPE:
HANDLE_DRAW_GOOMBA_FLAT:
                        img = goomba_flat;
                        break;
                    case KOOPA_BONES_TYPE:
                        if (cur->vx < 0) {
                            img = koopa_bones_left_sprite;
                        } else {
                            img = koopa_bones_right_sprite;
                        }
                        break;
                    case KOOPA_RED_FLY_TYPE:
                        if (x > oiram.x) {
                            gfx_TransparentSprite(koopa_red_left_sprite, rel_x, rel_y);
                            gfx_TransparentSprite(wing_left_sprite, rel_x + 7, rel_y);
                        } else {
                            gfx_TransparentSprite(koopa_red_right_sprite, rel_x, rel_y);
                            gfx_TransparentSprite(wing_right_sprite, rel_x, rel_y);
                        }
                        goto HANDLE_SKIP_DRAW;
                    case KOOPA_RED_TYPE:
                        if (cur->vx < 0) {
                            img = koopa_red_left_sprite;
                        } else {
                            img = koopa_red_right_sprite;
                        }
                        break;
                    case SPIKE_TYPE:
                        if (cur->vx < 0) {
                            img = spike_left_sprite;
                        } else {
                            img = spike_right_sprite;
                        }
                        break;
                    case KOOPA_GREEN_FLY_TYPE:
                        if (x > oiram.x) {
                            gfx_TransparentSprite(koopa_green_left_sprite, rel_x, rel_y);
                            gfx_TransparentSprite(wing_left_sprite, rel_x + 7, rel_y);
                        } else {
                            gfx_TransparentSprite(koopa_green_right_sprite, rel_x, rel_y);
                            gfx_TransparentSprite(wing_right_sprite, rel_x, rel_y);
                        }
                        goto HANDLE_SKIP_DRAW;
                    case KOOPA_GREEN_TYPE:
                        if (cur->vx < 0) {
                            img = koopa_green_left_sprite;
                        } else {
                            img = koopa_green_right_sprite;
                        }
                        break;
                    case KOOPA_BONES_DEAD_TYPE:
HANDLE_DRAW_BONES_FLAT:
                        img = cur->sprite;
                        if (cur->counter < 20) { if (cur->counter & 1) { goto HANDLE_SKIP_DRAW; } }
                        break;
                    case KOOPA_RED_SHELL_TYPE:
                        if (cur->sprite == koopa_red_shell_1) { img = koopa_red_shell_0; } else { img = koopa_red_shell_1; }
                        goto HANDLE_DRAW_SHELL;
                    case KOOPA_GREEN_SHELL_TYPE:
                        if (cur->sprite == koopa_green_shell_1) { img = koopa_green_shell_0; } else { img = koopa_green_shell_1; }
                        goto HANDLE_DRAW_SHELL;
                    case SPIKE_SHELL_TYPE:
                        if (cur->sprite == spike_shell_1) { img = spike_shell_0; } else { img = spike_shell_1; }
HANDLE_DRAW_SHELL:
                        if (cur->vx) {
                            uint8_t j;
                            cur->sprite = img;

                            for(j = 0; j < num_simple_movers; j++) {
                                simple_move_t *hit = simple_mover[j];
                                uint8_t hit_type = hit->type;

                                if (hit_type > HITABLE_TYPES) {
                                    int hit_x = hit->x;
                                    int hit_y = hit->y;
                                    if (gfx_CheckRectangleHotspot(hit_x, hit_y, hit->hitbox.width, hit->hitbox.height, x, y, 15, 15)) {
                                        if (i != j) {
                                            add_score(cur->score_counter, x, y);
                                            if(cur->score_counter != 8) { cur->score_counter++; }
                                            remove_simple_mover(j);
                                            add_poof(hit_x + 4, hit_y + 4);
                                            i = -1;
                                            break;
                                        }
                                    }
                                }
                            }

                            for(j = 0; j < num_chompers; j++) {
                                chomper_t *hit = chomper[j];

                                if (hit->y < hit->start_y) {
                                    if (gfx_CheckRectangleHotspot(hit->x, hit->y, 15, 30, x, y, 15, 15)) {
                                        remove_chomper(j);
                                        add_score(cur->score_counter, x, y);
                                        if(cur->score_counter != 8) { cur->score_counter++; }
                                        add_poof(hit->x + 4, y);
                                        break;
                                    }
                                }
                            }

                        } else {
                            if (cur->counter < 20) { if (cur->counter & 1) { goto HANDLE_SKIP_DRAW; } }
                        }

                        break;
                    default:
                        goto HANDLE_SKIP_DRAW;
                }
                gfx_TransparentSprite(img, rel_x, rel_y);
            } else {
                switch(type) {
                    case RESWOB_TYPE:
                        shrink_oiram();
                        break;
                    case MUSHROOM_TYPE:
                        eat_mushroom();
                        goto HANDLE_REMOVE_MOVER;
                    case MUSHROOM_1UP_TYPE:
                        add_score(8, x, y);
                        goto HANDLE_REMOVE_MOVER_NO_SCORE;
                    case FIRE_FLOWER_TYPE:
                        eat_fire_flower();
                        goto HANDLE_REMOVE_MOVER;
                    case STAR_TYPE:
                        eat_star();
HANDLE_REMOVE_MOVER:
                        add_score(4, x, y);
HANDLE_REMOVE_MOVER_NO_SCORE:
                        remove_simple_mover(i);
                        i = -1;
                        break;
                    case GOOMBA_TYPE:
                        if ((oiram.vy <= 0 && oiram.y + ((oiram.flags & FLAG_OIRAM_BIG) ? 11 : 0) >= y) ||
                            (oiram.flags & (FLAG_OIRAM_INVINCIBLE | FLAG_OIRAM_SLIDE))) {
                            if (!shrink_oiram()) {
                                add_next_chain_score(x, y);
                                add_poof(oiram.x, oiram.y + 2);
                                goto HANDLE_REMOVE_MOVER_NO_SCORE;
                            } else {
                                goto HANDLE_DRAW_SPRITE_GOOMBA;
                            }
                        } else {
                            add_next_chain_score(x, y);
                            oiram.vy = -5;
                            cur->hitbox.height = 8;
                            type = FLAT_GOOMBA_TYPE;
                            cur->vx = 0;
                            y += 5;
                            cur->counter = 30;
                            goto HANDLE_DRAW_SPRITE_GOOMBA;
                        }
                        break;
                    case FLAT_GOOMBA_TYPE:
                        goto HANDLE_DRAW_GOOMBA_FLAT;
                    case KOOPA_GREEN_FLY_TYPE:
                    case KOOPA_RED_FLY_TYPE:
                        if ((oiram.vy <= 0 && oiram.y >= y) || oiram.flags & (FLAG_OIRAM_INVINCIBLE | FLAG_OIRAM_SLIDE)) {
                            if (!shrink_oiram()) {
                                add_next_chain_score(x, y);
                                add_poof(oiram.x, oiram.y + 2);
                                goto HANDLE_REMOVE_MOVER_NO_SCORE;
                            } else {
                                goto HANDLE_DRAW_SPRITE;
                            }
                        } else {
                            add_next_chain_score(x, y);
                            oiram.vy = -9;
                            if (type == KOOPA_GREEN_FLY_TYPE) {
                                type = KOOPA_GREEN_TYPE;
                                cur->smart = false;
                            } else {
                                type = KOOPA_RED_TYPE;
                            }
                            cur->vy = 0;
                            if (rel_x < oiram.x) {
                                cur->vx = 1;
                            } else {
                                cur->vx = -1;
                            }
                            cur->is_flyer = false;
                            goto HANDLE_DRAW_SPRITE;
                        }
                        break;
                    case KOOPA_BONES_TYPE:
                    case KOOPA_RED_TYPE:
                    case KOOPA_GREEN_TYPE:
                        if ((oiram.vy <= 0 && oiram.y >= y) || oiram.flags & (FLAG_OIRAM_INVINCIBLE | FLAG_OIRAM_SLIDE)) {
                    case SPIKE_TYPE:
                            if (!shrink_oiram()) {
                                add_next_chain_score(x, y);
                                add_poof(oiram.x, oiram.y + 2);
                                goto HANDLE_REMOVE_MOVER_NO_SCORE;
                            } else {
                                goto HANDLE_DRAW_SPRITE;
                            }
                        } else {
                            oiram.vy = -5;
HANDLE_CREATE_SHELL:
                            add_next_chain_score(x, y);
                            if (type == KOOPA_RED_TYPE) {
                                cur->sprite = koopa_red_shell_0;
                                type = KOOPA_RED_SHELL_TYPE;
                                cur->hitbox.height = 15;
                            } else if (type == KOOPA_GREEN_TYPE) {
                                cur->sprite = koopa_green_shell_0;
                                type = KOOPA_GREEN_SHELL_TYPE;
                                cur->hitbox.height = 15;
                            } else if (type == SPIKE_TYPE) {
                                cur->sprite = spike_shell_0;
                                type = SPIKE_SHELL_TYPE;
                            } else {
                                cur->sprite = (cur->vx < 0) ? koopa_bones_dead_left : koopa_bones_dead_right;
                                type = KOOPA_BONES_DEAD_TYPE;
                                cur->hitbox.height = 12;
                                cur->vx = 0;
                                y += 11;
                                cur->smart = false;
                                cur->counter = 127;
                                goto HANDLE_DRAW_BONES_FLAT;
                            }
                            cur->vx = 0;
                            y += 11;
                            cur->smart = false;
                            cur->counter = 127;
                            goto HANDLE_DRAW_SPRITE;
                        }
                        break;
                    case KOOPA_RED_SHELL_TYPE: case KOOPA_GREEN_SHELL_TYPE:
                        if (cur->vy > 0 || oiram.y + oiram.hitbox.height - 8 < y) {
                            oiram.vy = -8;
                            if (!cur->vx && (oiram.x != x)) {
                                goto HANDLE_KICK_SHELL;
                            }
                            cur->vx = 0;
                            cur->score_counter = 0;
                            cur->counter = 127;
                        } else {
                            if (cur->vx) {
                    case SPIKE_SHELL_TYPE:
                                if (oiram.vy <= 0 || oiram.flags & (FLAG_OIRAM_INVINCIBLE | FLAG_OIRAM_SLIDE)) {
                                    if (!shrink_oiram()) {
                                        add_score(0, oiram.x, oiram.y);
                                        add_poof(oiram.x, oiram.y + 2);
                                        goto HANDLE_REMOVE_MOVER_NO_SCORE;
                                    } else {
                                        goto HANDLE_DRAW_SPRITE;
                                    }
                                }
                            } else {
HANDLE_KICK_SHELL:
                                if ((oiram.x + OIRAM_HITBOX_WIDTH/2) < x) {
                                    cur->vx = 5;
                                } else {
                                    cur->vx = -5;
                                }
                                add_score(0, x, y);
                                cur->counter = -1;
                            }
                        }
                    case KOOPA_BONES_DEAD_TYPE:
                        goto HANDLE_DRAW_SPRITE;
                    default:
                        break;
                }
            }
HANDLE_SKIP_DRAW:

            if (cur->counter >= 0) {
                cur->counter--;
                if (!cur->counter) {
                    cur->smart = true;
                    cur->hitbox.height = 26;
                    y -= 11;

                    if (type == KOOPA_GREEN_SHELL_TYPE) {
                        cur->smart = false;
                        type = KOOPA_GREEN_TYPE;
                    } else if (type == KOOPA_RED_SHELL_TYPE) {
                        type = KOOPA_RED_TYPE;
                    } else if (type == KOOPA_BONES_DEAD_TYPE) {
                        y -= 3;
                        type = KOOPA_BONES_TYPE;
                    } else if (type == SPIKE_SHELL_TYPE) {
                        type = SPIKE_TYPE;
                        y += 10;
                        cur->hitbox.height = 15;
                        cur->smart = false;
                    } else {
                        goto HANDLE_REMOVE_MOVER_NO_SCORE;
                    }
                    cur->counter = -1;
                    if (oiram.x < rel_x) {
                        cur->vx = -1;
                    } else {
                        cur->vx = 1;
                    }
                }
            }
            cur->y = y;
            cur->type = type;
            continue;
        }
    }

    if (num_bumped_tiles) {
        for(i = 0; i < num_bumped_tiles; i++) {
            bumped_tile_t *cur = bumped_tile[i];
            uint8_t tile_img = cur->tile;

            x = cur->x;
            y = cur->y;

            if (!in_viewport(x, y)) {
                continue;
            }

            switch(cur->dir) {
                case TILE_BOTTOM:
                    y += 2;
                    break;
                case TILE_TOP:
                    y -= 2;
                    break;
                default:
                    break;
            }

            cur->count--;

            if (tile_img == TILE_VANISH) {
                if (cur->count > 8) {
                    goto HANDLE_SKIP_BUMP;
                } else {
                    if (cur->count & 1) {
                        y--;
                    } else {
                        y++;
                    }
                }
            }

            cur->y = y;

HANDLE_SKIP_BUMP:

            // draw the tile
            gfx_TransparentSprite(tileset_tiles[tile_img], x - oiram.scrollx, y - oiram.scrolly);

            if (!cur->count) {
                remove_bumped_tile(i--);
            }
        }
    }

    if (num_chompers && !easter_egg4) {
        uint8_t dir = 0;

        for(i = 0; i < num_chompers; i++) {
            chomper_t *cur = chomper[i];
            int start_y;

            x = cur->x;
            y = cur->y;

            if (!in_viewport(x, y)) {
                continue;
            }

            rel_x = x - oiram.scrollx;
            rel_y = y - oiram.scrolly;
            start_y = cur->start_y;

            if (y < start_y) {
                int ymax = start_y - oiram.scrolly;
                if (ymax <= 0) { goto HANDLE_CHOMER_NO_DRAW; }
                if (ymax > Y_PXL_MAX) { ymax = Y_PXL_MAX; }
                gfx_SetClipRegion(0, 0, X_PXL_MAX, ymax);

                if (cur->throws_fire) {
                    gfx_sprite_t *img;
                    if (x > oiram.x) {
                        if (y < oiram.y) {
                            img = chomper_fire_down_left;
                            dir = DOWN_LEFT;
                        } else {
                            img = chomper_fire_up_left;
                            dir = UP_LEFT;
                        }
                    } else {
                        if (y < oiram.y) {
                            img = chomper_fire_down_right;
                            dir = DOWN_RIGHT;
                        } else {
                            img = chomper_fire_up_right;
                            dir = UP_RIGHT;
                        }
                    }
                    gfx_TransparentSprite(img, rel_x, rel_y);
                } else {
                    gfx_TransparentSprite(chomper_sprite, rel_x, rel_y);
                }
                gfx_TransparentSprite(chomper_body, rel_x, rel_y + 16);
            }
HANDLE_CHOMER_NO_DRAW:
            if (!cur->count) {

                y += cur->vy;

                if (y + 30 == start_y || y - 10 == start_y) {
                    cur->count = 40;
                    cur->vy = -cur->vy;
                }

            } else {
                cur->count--;
                if (cur->vy > 0 && cur->throws_fire && (cur->count == 36 || cur->count == 5)) {
                    add_fireball(x + 4, y + 4, dir, CHOMPER_FIREBALL);
                }
            }

            if (oiram_collision(x, y, 15, 30)) {
                if (!warp.style && !shrink_oiram()) {
                    add_score(1, x, y);
                    add_poof(oiram.x, oiram.y + 2);
                    remove_chomper(i--);
                    continue;
                }
            }

            cur->y = y;
        }
        gfx_SetClipRegion(0, 0, X_PXL_MAX, Y_PXL_MAX);
    }

    if (num_simple_enemies) {
        for(i = 0; i < num_simple_enemies; i++) {
            enemy_t *cur = simple_enemy[i];
            gfx_sprite_t *img;
            uint8_t *tile;
            int tmp_add;

            x = cur->x;
            y = cur->y;

            rel_x = x - oiram.scrollx;
            rel_y = y - oiram.scrolly;

            tile = gfx_TilePtr(&tilemap, x, y);

            switch(cur->type) {
                case SCORE_TYPE:
                    cur->counter--;
                    if (!cur->counter) {
                        remove_simple_enemy(i--);
                    } else {
                        cur->y--;
                        gfx_TransparentSprite(cur->sprite, rel_x, rel_y);
                    }
                    break;
                case FISH_TYPE:
                    if (!in_viewport(x, y)) {
                        continue;
                    }
                    if (cur->vx < 0) {
                        img = fish_left_sprite;
                        tmp_add = 0;
                    } else {
                        img = fish_right_sprite;
                        tmp_add = 16;
                    }
                    gfx_TransparentSprite(img, rel_x, rel_y);
                    if (!moveable_tile(x + cur->vx + tmp_add, y)) {
                        cur->vx = -cur->vx;
                    }
                    cur->x += cur->vx;
                    if (oiram_collision(x, y, 16, 16)) {
                        if (!shrink_oiram()) {
                            add_score_no_sprite(1);
                            cur->type = SCORE_TYPE;
                            cur->counter = 16;
                            cur->sprite = score_200;
                        }
                    }
                    break;
                case BULLET_CREATOR_TYPE:
                    if (!in_viewport(x, y)) {
                        continue;
                    }

                    if (!cur->counter) {
                        add_simple_enemy(tile, BULLET_TYPE);
                        add_poof(x, y + 2);
                        cur->counter = 100;
                    } else {
                        cur->counter--;
                    }
                    break;
                case CANNONBALL_DOWN_CREATOR_TYPE:
                    if (!in_viewport(x, y)) {
                        continue;
                    }
                    if (!cur->counter) {
                        enemy_t *cannon;
                        cannon = add_simple_enemy(tile, CANNONBALL_TYPE);
                        add_poof(x, y + 6);
                        cannon->vy = 2;
                        cur->counter = 110;
                    } else {
                        cur->counter--;
                    }
                    break;
                case CANNONBALL_UP_CREATOR_TYPE:
                    if (!in_viewport(x, y)) {
                        continue;
                    }
                    if (!cur->counter) {
                        enemy_t *cannon = add_simple_enemy(tile, CANNONBALL_TYPE);
                        add_poof(x, y);
                        cannon->vy = -2;
                        cur->counter = 110;
                    } else {
                        cur->counter--;
                    }
                    break;
                case CANNONBALL_TYPE:
                case BULLET_TYPE:
                    if (!in_viewport(x, y)) {
                        remove_simple_enemy(i--);
                        continue;
                    }

                    gfx_TransparentSprite(cur->sprite, rel_x, rel_y);
                    cur->x += cur->vx;
                    cur->y += cur->vy;
                    if (oiram_collision(x, y, 15, 13)) {
                        if ((y < oiram.y + oiram.hitbox.height/2) || oiram.flags & (FLAG_OIRAM_INVINCIBLE | FLAG_OIRAM_SLIDE)) {
                            if (!shrink_oiram()) {
                                cur->vy = 4;
                                cur->vx = 0;
                            }
                        } else {
                            oiram.vy = -8;
                            cur->vy = 4;
                            cur->vx = 0;
                        }
                    }
                    break;
                case LEAF_TYPE:
                    if (y > level_map.max_y) {
                        remove_simple_enemy(i--);
                        continue;
                    }

                    if (cur->vx < 0) {
                        img = leaf_left;
                    } else {
                        img = leaf_right;
                    }

                    gfx_TransparentSprite(img, rel_x, rel_y);
                    cur->x += cur->vx;
                    cur->y++;

                    cur->counter--;

                    if (!cur->counter) {
                        cur->vx = -cur->vx;
                        cur->counter = 32;
                    }

                    if (oiram_collision(x, y, 16, 11)) {
                        eat_leaf();
                        add_score_no_sprite(4);
                        cur->type = SCORE_TYPE;
                        cur->counter = 16;
                        cur->sprite = score_1000;
                        goto start_boo_check;
                    }
                    break;
            }
        }
    }

    start_boo_check:

    if (num_boos) {
        for(i = 0; i < num_boos; i++) {
            boo_t *cur = boo[i];
            gfx_sprite_t *img;
            int prev_x = x = cur->x;
            y = cur->y;

            if (!in_viewport(x, y)) {
                continue;
            }

            rel_x = x - oiram.scrollx;
            rel_y = y - oiram.scrolly;

            if (oiram.x < x) {
                if (oiram.direction == FACE_RIGHT) {
                    img = boo_left_hide;
                } else {
                    img = boo_left;
                    x--;
                }
            } else {
                if (oiram.direction == FACE_LEFT) {
                    img = boo_right_hide;
                } else {
                    img = boo_right;
                    x++;
                }
            }

            if (prev_x != x) {
                if (cur->count < 6) {
                    cur->count++;
                } else {
                    if (oiram.y < y) {
                        cur->vy = -1;
                    } else {
                        cur->vy = 1;
                    }
                    cur->count = 0;
                    cur->dir = !cur->dir;
                }
                if (cur->dir) {
                    y += 2;
                } else {
                    y -= 2;
                }
                y += cur->vy;
            }


            gfx_TransparentSprite(img, rel_x, rel_y);

            if (oiram_collision(x, y, 15, 15)) {
                if (!shrink_oiram()) {
                    add_score(1, x, y);
                    add_poof(oiram.x, oiram.y + 2);
                    remove_boo(i--);
                    continue;
                }
            }

            cur->x = x;
            cur->y = y;
        }
    }

    if (num_flames) {

        for(i = 0; i < num_flames; i++) {
            int8_t tmp_vy;
            flame_t *cur = flame[i];

            x = cur->x;
            y = cur->y;

            if (!in_viewport(x, y)) {
                continue;
            }

            tmp_vy = cur->vy;

            if (y < cur->start_y) {
                gfx_sprite_t *img;
                if (tmp_vy < 0) { img = flame_sprite_up; } else { img = flame_sprite_down; }
                gfx_TransparentSprite(img, x - oiram.scrollx, y - oiram.scrolly);
            }

            if (cur->count) {
                cur->count--;
            } else {
                if (tmp_vy < 9) { tmp_vy++; }
                y += tmp_vy;

                if (y >= cur->start_y) {
                    cur->count = 60;
                    tmp_vy = -15;
                }
            }

            if (oiram_collision(x, y, 14, 16)) {
                if (!shrink_oiram()) {
                    add_score(1, x, y);
                    add_poof(oiram.x, oiram.y + 2);
                    remove_flame(i--);
                    continue;
                }
            }

            cur->y = y;
            cur->vy = tmp_vy;
        }
    }

    if (num_poofs) {
        for(i = 0; i < num_poofs; i++) {
            poof_t *cur = poof[i];

            cur->count--;
            if (!cur->count) {
                if (cur->second) {
                    remove_poof(i--);
                    continue;
                } else {
                    cur->second = true;
                    cur->count = 3;
                }
            }

            gfx_TransparentSprite((cur->second) ? poof_1 : poof_0, cur->x - oiram.scrollx, cur->y - oiram.scrolly);
        }
    }

    if (num_fireballs) {
        int8_t tmp_vx;
        uint8_t j;

        for(i = 0; i < num_fireballs; i++) {
            fireball_t *cur = fireball[i];
            uint8_t cur_type = cur->mover->type;
            tmp_vx = cur->mover->vx;

            if (cur_type == OIRAM_FIREBALL) {
                cur->mover->type = FIREBALL_TYPE;
                simple_move_handler(cur->mover);
                cur->mover->type = cur_type;
                if (something_died) {
                    something_died = false;
                    add_poof(cur->mover->x + 4, cur->mover->y + 4);
                    goto HANDLE_REMOVE_FIREBALL;
                }
            } else {
                cur->mover->x += tmp_vx;
                cur->mover->y += cur->mover->vy;
            }

            x = cur->mover->x;
            y = cur->mover->y;

            rel_x = x - oiram.scrollx;
            rel_y = y - oiram.scrolly;

            if (tmp_vx != cur->mover->vx || !cur->count-- || rel_x < -20 || rel_x > 350) {
                add_poof(x + 2, y + 2);
HANDLE_REMOVE_FIREBALL:
                remove_fireball(i--);
                continue;
            }

            if (cur_type == OIRAM_FIREBALL) {
                for(j = 0; j < num_simple_movers; j++) {
                    simple_move_t *hit = simple_mover[j];

                    if (hit->type > HITABLE_TYPES) {
                        if (gfx_CheckRectangleHotspot(hit->x, hit->y, hit->hitbox.width, hit->hitbox.height, x, y, 7, 7)) {
                            add_score(1, x,y );
                            add_poof(hit->x + 4, hit->y + 4);
                            remove_simple_mover(j);
                            goto HANDLE_REMOVE_FIREBALL;
                        }
                    }
                }

                for(j = 0; j < num_chompers; j++) {
                    chomper_t *hit = chomper[j];

                    if (gfx_CheckRectangleHotspot(hit->x, hit->y, 15, 29, x, y, 7, 7)) {
                        add_score(1, x, y);
                        add_poof(hit->x + 4, y);
                        remove_chomper(j);
                        goto HANDLE_REMOVE_FIREBALL;
                    }
                }

            // the type that can hit oiram
            } else {
                if (oiram_collision(x, y, 7, 7)) {
                    shrink_oiram();
                    add_poof(x + 4, y + 4);
                    goto HANDLE_REMOVE_FIREBALL;
                }
            }
            gfx_TransparentSprite(fireball_sprite, rel_x, rel_y);
        }
    }

    // draw the oiram sprite
    if (oiram.failed && oiram.started_fail) {
        gfx_TransparentSprite(oiram_fail, oiram.fail_x - oiram.scrollx, oiram.fail_y - oiram.scrolly);
    } else if (in_quicksand) {
        gfx_SetClipRegion(0, 0, X_PXL_MAX, quicksand_clip_y - oiram.scrolly);
        goto HANDLE_DRAW_OIRAM;
    } else if (warp.style) {
        if (!warp.enter) {
            switch (warp.style) {
                case PIPE_DOWN:
                HANDLE_PIPE_DOWN:
                    gfx_SetClipRegion(0, warp.clip_y - oiram.scrolly, X_PXL_MAX, Y_PXL_MAX);
                    break;
                case PIPE_LEFT:
                HANDLE_PIPE_LEFT:
                    gfx_SetClipRegion(warp.clip_x - oiram.scrollx, 0, X_PXL_MAX, Y_PXL_MAX);
                    break;
                case PIPE_RIGHT:
                HANDLE_PIPE_RIGHT:
                    gfx_SetClipRegion(0, 0, warp.clip_x - oiram.scrollx, Y_PXL_MAX);
                    break;
                case PIPE_UP:
                HANDLE_PIPE_UP:
                    gfx_SetClipRegion(0, 0, X_PXL_MAX, warp.clip_y - oiram.scrolly);
                    break;
                case DOOR_WARP:
                HANDLE_DOOR_WARP:
                    gfx_Sprite(door_top, oiram.door_x - oiram.scrollx, oiram.door_y - oiram.scrolly);
                    gfx_Sprite(door_bot, oiram.door_x - oiram.scrollx, oiram.door_y + 16 - oiram.scrolly);
                    break;
            }
        } else {
            switch (warp.style) {
                case PIPE_DOWN:
                    goto HANDLE_PIPE_UP;
                case PIPE_LEFT:
                    goto HANDLE_PIPE_RIGHT;
                case PIPE_RIGHT:
                    goto HANDLE_PIPE_LEFT;
                case PIPE_UP:
                    goto HANDLE_PIPE_DOWN;
                case DOOR_WARP:
                    goto HANDLE_DOOR_WARP;
            }
        }
        goto HANDLE_DRAW_OIRAM;
    } else if (oiram.invincible) {
        oiram.invincible--;
        if (oiram.invincible & 1) {
            goto HANDLE_DRAW_OIRAM;
        } else {
            if (!oiram.invincible) { oiram.flags &= ~FLAG_OIRAM_INVINCIBLE; }
        }
    } else if (oiram.shrink_count) {
        oiram.shrink_count--;
        if (oiram.shrink_count & 1) {
            goto HANDLE_DRAW_OIRAM;
        }
    } else {
HANDLE_DRAW_OIRAM:
        gfx_TransparentSprite(oiram.sprite, oiram.rel_x, oiram.rel_y);
    }

    if (oiram.has_shell) {
        gfx_sprite_t *shell;
        int shell_x, shell_y;
        if (oiram.has_red_shell) {
            shell = koopa_red_shell_0;
        } else {
            shell = koopa_green_shell_0;
        }
        if (oiram.direction == FACE_LEFT) {
            shell_x = oiram.rel_x - 10;
        } else {
            shell_x = oiram.rel_x + 15 - 6;
        }
        if ((oiram.flags & FLAG_OIRAM_BIG)) {
            shell_y = oiram.rel_y + 26/2 - 4;
        } else {
            shell_y = oiram.rel_y;
        }
        gfx_TransparentSprite(shell, shell_x, shell_y);
    }

    if ((oiram.flags & FLAG_OIRAM_RACOON) && !oiram.on_vine) {
        gfx_sprite_t *tail_img;
        int tail_x, tail_y = oiram.rel_y + 17;
        if (oiram.spin_count) {
            if (oiram.sprite == oiram_0_buffer_right) {
                goto set_tail_right;
            } else {
                goto set_tail_left;
            }
        } else {
            if (oiram.direction == FACE_LEFT) {
    set_tail_left:
                tail_img = tail_left_0;
                tail_x = oiram.rel_x + OIRAM_HITBOX_WIDTH;
            } else {
    set_tail_right:
                tail_img = tail_right_0;
                tail_x = oiram.rel_x - 5;
            }
        }
        if (oiram.crouched) {
            tail_y -= 7;
        }
        if (oiram.invincible) {
            if (oiram.invincible & 1) {
                goto draw_tail;
            }
        } else {
    draw_tail:
            gfx_TransparentSprite(tail_img, tail_x, tail_y);
        }
    }

    gfx_SetClipRegion(0, 0, X_PXL_MAX, Y_PXL_MAX);

    handling_events = false;
}
