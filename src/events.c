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
#include "events.h"
#include "images.h"
#include "lower.h"
#include "powerups.h"
#include "enemies.h"
#include "simple_mover.h"

fireball_t *fireball[MAX_FIREBALLS];
uint8_t num_fireballs = 0;

poof_t *poof[MAX_POOFS];
uint8_t num_poofs = 0;

bool someone_died = false;
bool something_died = false;
unsigned int num_coins;

const unsigned int shell_score_chain[] = { 500, 800, 1000, 2000, 4000, 5000, 8000, ONE_UP_SCORE };

// only handle if somewhat within view; otherwise we can just ignore it
bool in_viewport(int x, int y) {
    if (x - 360 >= mario.x) {
        return false;
    }
    if (x + 360 <= mario.x) {
        return false;
    }
    if (y - 360 >= mario.y) {
        return false;
    }
    if (y + 360 <= mario.y) {
        return false;
    }
    return true;
}

// gloabl used to handle events that aren't mario
bool handling_events;

void handle_pending_events(void) {
    uint8_t i;
    int x, y;
    int rel_x, rel_y;
        
    handling_events = true;
    
    if (num_thwomps) {
        for(i = 0; i < num_thwomps; i++) {
            thwomp_t *cur = thwomp[i];
            int tmp_y;
            int8_t tmp_vy;
            
            x = cur->x;
            y = cur->y;
            
            if (!in_viewport(x, y)) {
                continue;
            }
            
            tmp_vy = cur->vy;
            rel_x = x - mario.scrollx;
            rel_y = y - mario.scrolly;
            
            gfx_TransparentSprite(thwomp_0, rel_x, rel_y);
            
            if (y == cur->start_y) {
                if (mario.x >= x - 32 && mario.x <= x + 32 + mario.hitbox.width) {
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
            
            if (gfx_CheckRectangleHotspot(mario.x, mario.y, mario.hitbox.width, mario.hitbox.height, x, y, 23, 31)) {
                if (!shrink_mario()) {
                    add_poof(mario.x, mario.y + 2);
                    remove_thwomp(i--);
                }
            }
            
            cur->vy = tmp_vy;
            cur->y = y;
        }
    }
    
    if (num_simple_movers) {
        
        for(i = 0; i < num_simple_movers; i++) {
            simple_move_t *cur = simple_mover[i];
            
            x = cur->x;
            y = cur->y;
            
            if (!in_viewport(x, y)) {
                continue;
            }
            
            simple_move_handler(cur);
            
            if (something_died) {
                something_died = false;
                add_poof(x + 4, y + 4);
                remove_simple_mover(i--);
            }
            
            rel_x = x - mario.scrollx;
            rel_y = y - mario.scrolly;
            
            if (cur->bumped) {
                bumped_tile_t *bump;
                
                switch(cur->type) {
                    case GOOMBA_TYPE:
                        add_poof(x + 4, y + 4);
                        remove_simple_mover(i--);
                        add_score(100);
                        continue;
                    case KOOPA_RED_TYPE: case KOOPA_GREEN_TYPE: case KOOPA_BONES_TYPE:
                        add_score(100);
                        goto create_koopa_shell;
                    default:
                        cur->bumped = false;
                        break;
                }
            }
            
            if (!gfx_CheckRectangleHotspot(mario.x, mario.y, mario.hitbox.width, mario.hitbox.height, x, y, cur->hitbox.width, cur->hitbox.height) || someone_died) {
                gfx_image_t *img;
draw_sprite:
                switch(cur->type) {
                    case MUSHROOM_TYPE:
                        img = mushroom;
                        break;
                    case FIRE_FLOWER_TYPE:
                        img = fire_flower;
                        break;
                    case STAR_TYPE:
                        img = star_0;
                        break;
                    case GOOMBA_TYPE:
draw_goomba_sprite:
                        img = goomba_sprite;
                        break;
                    case FLAT_GOOMBA_TYPE:
draw_flat_goomba_sprite:
                        img = goomba_flat;
                        break;
                    case KOOPA_BONES_TYPE:
                        img = (cur->vx < 0) ? koopa_bones_left_sprite : koopa_bones_right_sprite;
                        break;
                    case KOOPA_RED_FLY_TYPE:
                        if (rel_x > mario.x) {
                            gfx_TransparentSprite(koopa_red_left_sprite, rel_x, rel_y);
                            gfx_TransparentSprite(wing_left_sprite, rel_x + 7, rel_y);
                        } else {
                            gfx_TransparentSprite(koopa_red_right_sprite, rel_x, rel_y);
                            gfx_TransparentSprite(wing_right_sprite, rel_x, rel_y);
                        }
                        goto skip_draw;
                        break;
                    case KOOPA_RED_TYPE:
                        img = (cur->vx < 0) ? koopa_red_left_sprite : koopa_red_right_sprite;
                        break;
                    case KOOPA_GREEN_FLY_TYPE:
                        if (cur->vx < 0) {
                            gfx_TransparentSprite(koopa_green_left_sprite, rel_x, rel_y);
                            gfx_TransparentSprite(wing_left_sprite, rel_x + 7, rel_y);
                        } else {
                            gfx_TransparentSprite(koopa_green_right_sprite, rel_x, rel_y);
                            gfx_TransparentSprite(wing_right_sprite, rel_x, rel_y);
                        }
                        goto skip_draw;
                        break;
                    case KOOPA_GREEN_TYPE:
                        img = (cur->vx < 0) ? koopa_green_left_sprite : koopa_green_right_sprite;
                        break;
                    case KOOPA_BONES_DEAD_TYPE:
draw_koopa_bones_dead_sprite:
                        img = cur->sprite;
                        if (cur->counter < 20) { if (cur->counter & 1) { goto skip_draw; } }
                        break;
                    case KOOPA_RED_SHELL_TYPE:
                    case KOOPA_GREEN_SHELL_TYPE:
draw_koopa_shell:
                        if (cur->vx) {
                            uint8_t j;
                            if (cur->type == KOOPA_RED_SHELL_TYPE) {
                                img = cur->sprite = (cur->sprite == koopa_red_shell_0) ? koopa_red_shell_1 : koopa_red_shell_0;
                            } else {
                                img = cur->sprite = (cur->sprite == koopa_green_shell_0) ? koopa_green_shell_1 : koopa_green_shell_0;
                            }
                            
                            for(j = 0; j < num_simple_movers; j++) {
                                simple_move_t *hit = simple_mover[j];
                                
                                if (hit->type > HITABLE_TYPES) {
                                    if (gfx_CheckRectangleHotspot(hit->x, hit->y, hit->hitbox.width, hit->hitbox.height, x, y, 15, 15)) {
                                        if (hit != cur) {
                                            add_score(shell_score_chain[cur->score_counter]);
                                            if(cur->score_counter != 7) { cur->score_counter++; }
                                            remove_simple_mover(j);
                                            add_poof(hit->x + 4, hit->y + 4);
                                            if (hit->type == KOOPA_RED_SHELL_TYPE || hit->type == KOOPA_GREEN_SHELL_TYPE) {
                                                remove_simple_mover(i);
                                            }
                                            
                                            // need to reprocess all the cleared enemies
                                            i = j;
                                            goto continue_loop;
                                        }
                                    }
                                }
                            }
                            
                            for(j = 0; j < num_chompers; j++) {
                                chomper_t *hit = chomper[j];
                                
                                if (gfx_CheckRectangleHotspot(hit->x, hit->y, 15, 30, x, y, 15, 15)) {
                                    remove_chomper(j--);
                                    add_score(shell_score_chain[cur->score_counter]);
                                    if(cur->score_counter != 7) { cur->score_counter++; }
                                    add_poof(hit->x + 4, y);
                                    goto continue_loop;
                                }
                            }
                            
                        } else {
                            img = (cur->type == KOOPA_RED_SHELL_TYPE) ? koopa_red_shell_0 : koopa_green_shell_0;
                            if (cur->counter < 20) { if (cur->counter & 1) { goto skip_draw; } }
                        }
                        
                        break;
                    default:
                        img = NULL;
                        break;
                }
                gfx_TransparentSprite(img, rel_x, rel_y);
            } else {
                switch(cur->type) {
                    case MUSHROOM_TYPE:
                        eat_mushroom();
                        remove_simple_mover(i--);
                        break;
                    case MUSHROOM_1UP_TYPE:
                        eat_mushroom_1up();
                        remove_simple_mover(i--);
                        break;
                    case FIRE_FLOWER_TYPE:
                        eat_fire_flower();
                        remove_simple_mover(i--);
                        break;
                    case STAR_TYPE:
                        eat_star();
                        remove_simple_mover(i--);
                        break;
                    case GOOMBA_TYPE:
                        if ((mario.vy <= 0) || mario.flags & (FLAG_MARIO_INVINCIBLE | FLAG_MARIO_SLIDE)) {
                            if (!shrink_mario()) {
                                add_next_chain_score();
                                add_poof(mario.x, mario.y + 2);
                                remove_simple_mover(i--);
                            } else {
                                goto draw_goomba_sprite;
                            }
                        } else if (mario.flags & (FLAG_MARIO_INVINCIBLE | FLAG_MARIO_SLIDE)) {
                            add_next_chain_score();
                            add_poof(mario.x, mario.y + 2);
                            remove_simple_mover(i--);
                        } else {
                            add_next_chain_score();
                            mario.vy = -5;
                            cur->hitbox.height = 8;
                            cur->type = FLAT_GOOMBA_TYPE;
                            cur->vx = 0;
                            cur->y += 5;
                            cur->counter = 30;
                            goto draw_goomba_sprite;
                        }
                        break;
                    case FLAT_GOOMBA_TYPE:
                        goto draw_flat_goomba_sprite;
                    case KOOPA_GREEN_FLY_TYPE:
                    case KOOPA_RED_FLY_TYPE:
                        if ((mario.vy <= 0) || mario.flags & (FLAG_MARIO_INVINCIBLE | FLAG_MARIO_SLIDE)) {
                            if (!shrink_mario()) {
                                add_next_chain_score();
                                add_poof(mario.x, mario.y + 2);
                                remove_simple_mover(i--);
                            } else {
                                goto draw_sprite;
                            }
                        } else {
                            add_next_chain_score();
                            mario.vy = -9;
                            if (cur->type == KOOPA_GREEN_FLY_TYPE) {
                                cur->type = KOOPA_GREEN_TYPE;
                            } else {
                                cur->type = KOOPA_RED_TYPE;
                            }
                            cur->vy = 0; cur->vx = (rel_x < mario.x) ? 1 : -1;
                            cur->is_flyer = false;
                            goto draw_sprite;
                        }
                        break;
                    case KOOPA_BONES_TYPE:
                    case KOOPA_RED_TYPE:
                    case KOOPA_GREEN_TYPE:
                        if ((mario.vy <= 0) || mario.flags & (FLAG_MARIO_INVINCIBLE | FLAG_MARIO_SLIDE)) {
                            if (!shrink_mario()) {
                                add_next_chain_score();
                                add_poof(mario.x, mario.y + 2);
                                remove_simple_mover(i--);
                            } else {
                                goto draw_sprite;
                            }
                        } else {
                            mario.vy = -5;
create_koopa_shell:
                            add_next_chain_score();
                            if (cur->type == KOOPA_RED_TYPE) {
                                cur->sprite = koopa_red_shell_0;
                                cur->type = KOOPA_RED_SHELL_TYPE;
                                cur->hitbox.height = 15;
                            } else if (cur->type == KOOPA_GREEN_TYPE) {
                                cur->sprite = koopa_green_shell_0;
                                cur->type = KOOPA_GREEN_SHELL_TYPE;
                                cur->hitbox.height = 15;
                            } else {
                                cur->sprite = (cur->vx < 0) ? koopa_bones_dead_left : koopa_bones_dead_right;
                                cur->type = KOOPA_BONES_DEAD_TYPE;
                                cur->hitbox.height = 12;
                                cur->vx = 0;
                                cur->y += 11;
                                cur->smart = false;
                                cur->counter = 127;
                                goto draw_koopa_bones_dead_sprite;
                            }
                            cur->vx = 0;
                            cur->y += 11;
                            cur->smart = false;
                            cur->counter = 127;
                            goto draw_koopa_shell;
                        }
                        break;
                    case KOOPA_RED_SHELL_TYPE: case KOOPA_GREEN_SHELL_TYPE:
                        if (mario.y + mario.hitbox.height - 7 < y) {
                            mario.vy = -8;
                            if (!cur->vx && (mario.x != x)) {
                                goto kick_shell;
                            }
                            cur->vx = 0;
                            cur->score_counter = 0;
                            cur->counter = 127;
                        } else {
                            if (cur->vx) {
                                if (mario.vy <= 0 || mario.flags & (FLAG_MARIO_INVINCIBLE | FLAG_MARIO_SLIDE)) {
                                    if (!shrink_mario()) {
                                        add_score(100);
                                        add_poof(mario.x, mario.y + 2);
                                        remove_simple_mover(i--);
                                    } else {
                                        goto draw_sprite;
                                    }
                                }
                            } else {
kick_shell:
                                if ((mario.x + mario.hitbox.width/2) < x) {
                                    cur->vx = 4;
                                } else {
                                    cur->vx = -4;
                                }
                                add_next_chain_score();
                                cur->counter = -1;
                            }
                        }
                    case KOOPA_BONES_DEAD_TYPE:
                        goto draw_sprite;
                    default:
                        break;
                }
            }
skip_draw:

            if (cur->counter >= 0) {
                if (!cur->counter--) {
                    cur->smart = true;
                    cur->y -= 11;
                    if (cur->type == KOOPA_GREEN_SHELL_TYPE) {
                        cur->smart = false;
                        cur->type = KOOPA_GREEN_TYPE;
                    } else if (cur->type == KOOPA_RED_SHELL_TYPE) {
                        cur->type = KOOPA_RED_TYPE;
                    } else if (cur->type == KOOPA_BONES_DEAD_TYPE) {
                        cur->y -= 3;
                        cur->type = KOOPA_BONES_TYPE;
                    } else {
                        remove_simple_mover(i--);
                        goto continue_loop;
                    }
                    cur->counter = -1;
                    if (mario.x < rel_x) {
                        cur->vx = -1;
                    } else {
                        cur->vx = 1;
                    }
                    cur->hitbox.height = 26;
                }
            }
continue_loop:
            continue;
        }
    }
    
    if (num_bumped_tiles) {
        for(i = 0; i < num_bumped_tiles; i++) {
            bumped_tile_t *cur = bumped_tile[i];
            
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
                case TILE_RIGHT:
                    x += 2;
                    break;
                case TILE_LEFT:
                    x -= 2;
                    break;
                default:
                    abort();
            }
            
            cur->x = x;
            cur->y = y;
           
            // draw the tile
            gfx_TransparentSprite(tileset_tiles[cur->tile], x - mario.scrollx, y - mario.scrolly);
            
            if (!(cur->count--)) {
                remove_bumped_tile(i--);
            }
        }
    }
    
    if (num_chompers) {
        uint8_t dir = 0;
        
        for(i = 0; i < num_chompers; i++) {
            chomper_t *cur = chomper[i];
            
            x = cur->x;
            y = cur->y;
            
            if (!in_viewport(x, y)) {
                continue;
            }
            
            rel_x = x - mario.scrollx;
            rel_y = y - mario.scrolly;
            
            if (y < cur->start_y) {
                gfx_image_t *img;
                
                if (cur->start_y - mario.scrolly < (TILEMAP_DRAW_HEIGHT * TILE_HEIGHT) - TILE_HEIGHT) {
                    gfx_SetClipRegion(0, 0, 320, cur->start_y - mario.scrolly);
                }
                
                if (cur->throws_fire) {
                    if (x > mario.x) {
                        if (y < mario.y) {
                            img = chomper_fire_down_left;
                            dir = DOWN_LEFT;
                        } else {
                            img = chomper_fire_up_left;
                            dir = UP_LEFT;
                        }
                    } else {
                        if (y < mario.y) {
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
                gfx_SetClipRegion(0, 0, 320, (TILEMAP_DRAW_HEIGHT * TILE_HEIGHT) - TILE_HEIGHT);
            }
            
            if (!cur->count) {
                
                y += cur->vy;
                
                if (y + 30 == cur->start_y || y - 10 == cur->start_y) {
                    cur->count = 40;
                    cur->vy = -cur->vy;
                }
                
            } else {
                cur->count--;
                if (cur->vy > 0 && cur->throws_fire && (cur->count == 36 || cur->count == 5)) {
                    fireball_t *ball;
                    ball = add_fireball(x + 4, y + 4, dir);
                    ball->type = CHOMPER_FIREBALL;
                }
            }
            
            if (gfx_CheckRectangleHotspot(mario.x, mario.y, mario.hitbox.width, mario.hitbox.height, x, y, 15, 30)) {
                if (!shrink_mario()) {
                    add_score(200);
                    add_poof(mario.x, mario.y + 2);
                    remove_chomper(i--);
                } else {
                    if (mario.crouched) { mario.y -= 12; }
                }
            }
            
            cur->y = y;
        }
    }
    
    if (num_simple_enemies) {
        for(i = 0; i < num_simple_enemies; i++) {
            enemy_t *cur = simple_enemy[i];
            gfx_image_t *img;
            int tmp_add;
            
            x = cur->x;
            y = cur->y;
            
            rel_x = x - mario.scrollx;
            rel_y = y - mario.scrolly;
            
            switch(cur->type) {
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
                    if (gfx_CheckRectangleHotspot(mario.x, mario.y, mario.hitbox.width, mario.hitbox.height, x, y, 16, 16)) {
                        if (!shrink_mario()) {
                            add_poof(x, y + 4);
                            remove_simple_enemy(i--);
                        }
                    }
                    break;
                case BULLET_CREATOR_TYPE:
                    if (!in_viewport(x, y)) {
                        continue;
                    }
            
                    if (!cur->counter) {
                        enemy_t *bullet;
                        bullet = add_simple_enemy(gfx_TilePtr(&tilemap, x, y), BULLET_TYPE);
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
                        cannon = add_simple_enemy(gfx_TilePtr(&tilemap, x, y), CANNONBALL_TYPE);
                        add_poof(x, y + 6);
                        cur->counter = 90;
                        cannon->vx = -2;
                        cannon->vy = 2;
                    } else {
                        cur->counter--;
                    }
                    break;
                case CANNONBALL_UP_CREATOR_TYPE:
                    if (!in_viewport(x, y)) {
                        continue;
                    }
                    if (!cur->counter) {
                        enemy_t *cannon;
                        cannon = add_simple_enemy(gfx_TilePtr(&tilemap, x, y), CANNONBALL_TYPE);
                        add_poof(x, y);
                        cannon->vx = -2;
                        cannon->vy = -2;
                        cur->counter = 90;
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
                    
                    gfx_TransparentSprite(cur->type == CANNONBALL_TYPE ? cannonball_sprite : bullet_left, rel_x, rel_y);
                    cur->x += cur->vx;
                    cur->y += cur->vy;
                    if (gfx_CheckRectangleHotspot(mario.x, mario.y, mario.hitbox.width, mario.hitbox.height, x, y, 15, 13)) {
                        if ((y < mario.y + mario.hitbox.height/2) || mario.flags & (FLAG_MARIO_INVINCIBLE | FLAG_MARIO_SLIDE)) {
                            if (!shrink_mario()) {
                                add_next_chain_score();
                                cur->vy = 4;
                                cur->vx = 0;
                            }
                        } else {
                            mario.vy = -8;
                            cur->vy = 4;
                            cur->vx = 0;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
    
    if (num_boos) {
        for(i = 0; i < num_boos; i++) {
            boo_t *cur = boo[i];
            gfx_image_t *img;
            int prev_x;
            
            prev_x = x = cur->x;
            y = cur->y;
            
            if (!in_viewport(x, y)) {
                continue;
            }
            
            rel_x = x - mario.scrollx;
            rel_y = y - mario.scrolly;
            
            if (mario.x < x) {
                if (mario.direction == FACE_RIGHT) {
                    img = boo_left_hide;
                } else {
                    img = boo_left;
                    x--;
                }
            } else {
                if (mario.direction == FACE_LEFT) {
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
                    if (mario.y < y) {
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
            
            if (gfx_CheckRectangleHotspot(mario.x, mario.y, mario.hitbox.width, mario.hitbox.height, x, y, 15, 15)) {
                if (!shrink_mario()) {
                    add_score(200);
                    add_poof(mario.x, mario.y + 2);
                    remove_boo(i--);
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
                gfx_image_t *img = tmp_vy < 0 ? flame_sprite_up : flame_sprite_down;
                gfx_TransparentSprite(img, x - mario.scrollx, y - mario.scrolly);
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
            
            if (gfx_CheckRectangleHotspot(mario.x, mario.y, mario.hitbox.width, mario.hitbox.height, x, y, 14, 16)) {
                if (!shrink_mario()) {
                    add_score(200);
                    add_poof(mario.x, mario.y + 2);
                    remove_flame(i--);
                }
            }
            
            cur->y = y;
            cur->vy = tmp_vy;
        }
    }
    
    if (num_poofs) {
        for(i = 0; i < num_poofs; i++) {
            poof_t *cur = poof[i];
            
            if (!cur->count--) {
                if (cur->second) {
                    remove_poof(i--);
                    continue;
                } else {
                    cur->second = true;
                    cur->count = 3;
                }
            }
            
            gfx_TransparentSprite((cur->second) ? poof_1 : poof_0, cur->x - mario.scrollx, cur->y - mario.scrolly);
        }
    }
    
    if (num_fireballs) {
        int8_t tmp_vx;
        uint8_t j;
        
        for(i = 0; i < num_fireballs; i++) {
            fireball_t *cur = fireball[i];
            tmp_vx = cur->mover->vx;
            
            if (cur->type == MARIO_FIREBALL) {
                simple_move_handler(cur->mover);
            } else {
                cur->mover->x += cur->mover->vx;
                cur->mover->y += cur->mover->vy;
            }
            
            x = cur->mover->x;
            y = cur->mover->y;
            
            rel_x = x - mario.scrollx;
            rel_y = y - mario.scrolly;
            
            if (tmp_vx != cur->mover->vx || !cur->count-- || rel_x < -20 || rel_x > 350) {
                add_poof(x + 2, y + 2);
                remove_fireball(i--);
            }
            
            if (cur->type == MARIO_FIREBALL) {
                for(j = 0; j < num_simple_movers; j++) {
                    simple_move_t *hit = simple_mover[j];
                    
                    if (hit->type > HITABLE_TYPES) {
                        if (gfx_CheckRectangleHotspot(hit->x, hit->y, hit->hitbox.width, hit->hitbox.height, x, y, 8, 8)) {
                            add_score(200);
                            add_poof(hit->x + 4, hit->y + 4);
                            remove_fireball(i--);
                            remove_simple_mover(j);
                            goto done_checks_fireball;
                        }
                    }
                }
                
                for(j = 0; j < num_chompers; j++) {
                    chomper_t *hit = chomper[j];
                    
                    if (gfx_CheckRectangleHotspot(hit->x, hit->y, 15, 30, x, y, 8, 8)) {
                        add_score(200);
                        add_poof(hit->x + 4, y);
                        remove_chomper(j);
                        remove_fireball(i--);
                        break;
                    }
                }
                
            // the type that can hit mario
            } else {
                if (gfx_CheckRectangleHotspot(mario.x, mario.y, mario.hitbox.width, mario.hitbox.height, x, y, 8, 8)) {
                    shrink_mario();
                    add_poof(x + 4, y + 4);
                    remove_fireball(i--);
                }
            }
done_checks_fireball:    
            gfx_TransparentSprite(fireball_sprite, rel_x, rel_y);
        }
    }

    // draw the mario sprite
    if (in_quicksand) {
        gfx_SetClipRegion(0, 0, X_PXL_MAX, quicksand_clip_y - mario.scrolly);
        goto draw_mario_clipped;
    } else if (mario.in_pipe) {
        if (!mario.enter_pipe) {
            switch (mario.in_pipe) {
                case PIPE_DOWN:
                    gfx_SetClipRegion(0, mario.pipe_clip_y, X_PXL_MAX, Y_PXL_MAX);
                    break;
                case PIPE_LEFT:
                    gfx_SetClipRegion(mario.rel_x + mario.pipe_counter, 0, X_PXL_MAX, Y_PXL_MAX);
                    break;
                case PIPE_RIGHT:
                    gfx_SetClipRegion(mario.rel_x, 0, mario.rel_x + mario.hitbox.width - mario.pipe_counter + 1, Y_PXL_MAX);
                    break;
                case PIPE_UP:
                    gfx_SetClipRegion(0, 0, X_PXL_MAX, mario.pipe_clip_y);
                    break;
            }
        } else {
            switch (mario.in_pipe) {
                case PIPE_DOWN:
                    gfx_SetClipRegion(0, 0, X_PXL_MAX, mario.rel_y + mario.pipe_counter + 1);
                    break;
                case PIPE_LEFT:
                    gfx_SetClipRegion(mario.rel_x, 0, mario.x + mario.pipe_counter + 1, Y_PXL_MAX);
                    break;
                case PIPE_RIGHT:
                    gfx_SetClipRegion(mario.rel_x + mario.hitbox.width - mario.pipe_counter - 1, 0, X_PXL_MAX, Y_PXL_MAX);
                    break;
                case PIPE_UP:
                    gfx_SetClipRegion(0, mario.rel_y + mario.hitbox.height - mario.pipe_counter - 1, X_PXL_MAX, Y_PXL_MAX);
                    break;
            }
        }
draw_mario_clipped:
        gfx_TransparentSprite(mario.curr_sprite, mario.rel_x, mario.rel_y);
        gfx_SetClipRegion(0, 0, X_PXL_MAX, Y_PXL_MAX);
    } else if (mario.invincible) {
        if (mario.invincible-- & 1) {
            goto draw_mario;
        }
        if (!mario.invincible) { mario.flags &= ~FLAG_MARIO_INVINCIBLE; }
    } else if (shrink_counter) {
        if (shrink_counter-- & 1) {
            goto draw_mario;
        }
    } else {
draw_mario:
        gfx_TransparentSprite(mario.curr_sprite, mario.rel_x, mario.rel_y);
    }
    
    if (mario.has_shell) {
        gfx_TransparentSprite(mario.has_red_shell ? koopa_red_shell_0 : koopa_green_shell_0, (mario.direction == FACE_LEFT) ? mario.rel_x - 10 : mario.rel_x + mario.hitbox.width - 6, (mario.flags & FLAG_MARIO_BIG) ? mario.rel_y + 26/2 - 4: mario.rel_y);
    }
    
    handling_events = false;
}

void remove_poof(uint8_t i) {
    poof_t *free_me = poof[i];
    uint8_t num_poofs_less = num_poofs--;
    
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
    uint8_t num_fireballs_less = num_fireballs--;
    
    for(; i < num_fireballs_less; i++) {
        fireball[i] = fireball[i+1];
    }
    
    if (free_me->type == MARIO_FIREBALL) {
        mario.fireballs--;
    }
    
    free(free_me->mover);
    free(free_me);
}

// add a fireball
fireball_t *add_fireball(int x, int y, uint8_t dir) {
    fireball_t *ball;
    simple_move_t *mover;
    
    if (num_fireballs >= MAX_FIREBALLS - 1) {
        remove_fireball(0);
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
    mover->is_bouncer = true;
    mover->hitbox.height = mover->hitbox.width = 7;
    mover->type = FIREBALL_TYPE;
    mover->is_flyer = false;
    ball->count = 127;
    num_fireballs++;
    return ball;
}