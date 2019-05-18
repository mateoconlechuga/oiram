#include "enemies.h"
#include "simple_mover.h"
#include "tile_handlers.h"
#include "loadscreen.h"
#include "images.h"
#include "oiram.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

// allocation tables for enemies
boo_t *boo[MAX_BOOS];
chomper_t *chomper[MAX_CHOMPERS];
thwomp_t *thwomp[MAX_THWOMPS];
flame_t *flame[MAX_FLAMES];
enemy_t *simple_enemy[MAX_SIMPLE_ENEMY];

// storage counters for enemies
uint8_t num_chompers = 0;
uint8_t num_boos = 0;
uint8_t num_thwomps = 0;
uint8_t num_flames = 0;
uint8_t num_simple_enemies = 0;

// types of shell enemies
static enum { KOOPA_GREEN, KOOPA_RED, KOOPA_GREEN_FLY, KOOPA_RED_FLY, KOOPA_BONES, SPIKE };

void add_goomba(uint8_t *tile) {
    simple_move_t *e = add_simple_mover(tile);

    e->hitbox.width = GOOMBA_WIDTH;
    e->hitbox.height = GOOMBA_HEIGHT;
    e->vx = -1;
    e->type = GOOMBA_TYPE;
}

void add_reswob(uint8_t *tile) {
    simple_move_t *e = add_simple_mover(tile);

    e->hitbox.width = RESWOB_WIDTH;
    e->hitbox.height = RESWOB_HEIGHT;
    e->vx = -1;
    e->type = RESWOB_TYPE;
}

void add_boo(uint8_t *tile) {
    boo_t *e;
    unsigned int x, y;

    if (num_boos > MAX_BOOS - 1) {
        return;
    }

    tile_to_abs_xy_pos(tile, &x, &y);

    boo[num_boos] = e = malloc(sizeof(boo_t));

    e->x = x;
    e->y = y;
    e->vy = -1;
    e->dir = false;
    e->count = 0;
    num_boos++;
}

void remove_boo(uint8_t i) {
    boo_t *e;
    uint8_t num_boos_less;

    if (!num_boos) {
        return;
    }

    e = boo[i];
    num_boos_less = num_boos--;

    for(; i < num_boos_less; i++) {
        boo[i] = boo[i+1];
    }

    free(e);
}

void add_shell_enemy(uint8_t *tile, uint8_t type) {
    simple_move_t *e = add_simple_mover(tile);

    e->hitbox.width = 15;
    e->hitbox.height = 26;
    e->vx = -1;
    switch(type) {
        case KOOPA_GREEN:
            e->type = KOOPA_GREEN_TYPE;
            e->smart = false;
            break;
        case KOOPA_RED:
            e->type = KOOPA_RED_TYPE;
            e->smart = true;
            break;
        case KOOPA_GREEN_FLY:
            e->type = KOOPA_GREEN_FLY_TYPE;
            goto FLYING_TYPE;
        case KOOPA_RED_FLY:
            e->type = KOOPA_RED_FLY_TYPE;
        FLYING_TYPE:
            e->smart = true;
            e->vx = 0;
            e->vy = 1;
            e->is_flyer = true;
            break;
        case KOOPA_BONES:
            e->type = KOOPA_BONES_TYPE;
            e->smart = true;
            break;
        default:
            e->hitbox.height = SPIKE_HEIGHT;
            e->smart = false;
            e->type = SPIKE_TYPE;
            break;
    }
}

void add_chomper(uint8_t *tile, bool throws_fire) {
    chomper_t *e;
    unsigned int x, y;

    if (num_chompers > MAX_CHOMPERS - 1) {
        return;
    }

    tile_to_abs_xy_pos(tile, &x, &y);
    chomper[num_chompers] = e = malloc(sizeof(chomper_t));

    e->x = x + TILE_WIDTH/2;
    e->y = y;
    e->start_y = y;
    e->vy = -2;
    e->throws_fire = throws_fire;
    e->count = 0;
    num_chompers++;
}

void remove_chomper(uint8_t i) {
    chomper_t *e;
    uint8_t num_chompers_less;

    if (!num_chompers) {
        return;
    }

    e = chomper[i];
    num_chompers_less = num_chompers--;

    for(; i < num_chompers_less; i++) {
        chomper[i] = chomper[i+1];
    }

    free(e);
}

void add_flame(uint8_t *tile) {
    flame_t *e;
    unsigned int x, y;

    if (num_flames > MAX_FLAMES - 1) {
        return;
    }

    tile_to_abs_xy_pos(tile, &x, &y);

    flame[num_flames] = e = malloc(sizeof(flame_t));

    e->x = x;
    e->y = y;
    e->start_y = y;
    e->vy = -15;
    e->count = 0;
    num_flames++;
}

void remove_flame(uint8_t i) {
    flame_t *e;
    uint8_t num_flames_less;

    if (!num_flames) {
        return;
    }

    e = flame[i];
    num_flames_less = num_flames--;

    for(; i < num_flames_less; i++) {
        flame[i] = flame[i+1];
    }

    free(e);
}

void add_thwomp(uint8_t *tile) {
    thwomp_t *e;
    unsigned int x, y;

    if (num_thwomps > MAX_THWOMPS - 1) {
        return;
    }

    tile_to_abs_xy_pos(tile, &x, &y);

    thwomp[num_thwomps] = e = malloc(sizeof(thwomp_t));

    e->x = x + 4;
    e->y = y;
    e->start_y = y;
    e->vy = 0;
    e->count = 0;
    num_thwomps++;
}

void remove_thwomp(uint8_t i) {
    thwomp_t *e;
    uint8_t num_thowmps_less;

    if (!num_thwomps) {
        return;
    }

    e = thwomp[i];
    num_thowmps_less = num_thwomps--;

    for(; i < num_thowmps_less; i++) {
        thwomp[i] = thwomp[i+1];
    }

    free(e);
}

enemy_t *add_simple_enemy(uint8_t *tile, uint8_t type) {
    enemy_t *enemy;
    unsigned int x, y;

    if (num_simple_enemies > MAX_SIMPLE_ENEMY - 1) {
        remove_simple_enemy(0);
    }

    tile_to_abs_xy_pos(tile, &x, &y);

    enemy = simple_enemy[num_simple_enemies] = malloc(sizeof(enemy_t));

    enemy->vx = 0;
    enemy->vy = 0;
    enemy->counter = 100;
    enemy->y = y;
    enemy->x = x;

    switch(type) {
        case BULLET_TYPE:
            enemy->vx = -3;
            enemy->y = y + 1;
            enemy->x = x - 4;
            enemy->sprite = bullet_left;
            break;
        case SCORE_TYPE:
            enemy->counter = 16;
            enemy->sprite = bullet_left;
            break;
        case CANNONBALL_TYPE:
            enemy->sprite = cannonball_sprite;
            enemy->vx = -2;
            break;
        case LEAF_TYPE:
            enemy->y -= TILE_HEIGHT;
            enemy->counter = 16;
        case FISH_TYPE:
            enemy->vx = -1;
        default:
            break;
    }

    enemy->type = type;
    num_simple_enemies++;
    return enemy;
}

void remove_simple_enemy(uint8_t i) {
    enemy_t *e;
    uint8_t num_simple_enemies_less;

    if (!num_simple_enemies) {
        return;
    }

    e = simple_enemy[i];
    num_simple_enemies_less = num_simple_enemies--;

    for(; i < num_simple_enemies_less; i++) {
        simple_enemy[i] = simple_enemy[i+1];
    }

    free(e);
}

void get_enemies(void) {
    uint8_t width = tilemap.width;
    uint8_t height = tilemap.height;
    unsigned int j, delay;
    unsigned int loop = width * height;

    for(j = 0; j < loop; j++) {
        uint8_t *this = tilemap.map + j;
        uint8_t tile = *this;
        int8_t tmp1, tmp2;

        switch(tile) {
            // this case is just to avoid another function that converts coins in water to water coins
            case TILE_COIN:
                for (tmp2=-1; tmp2<2; tmp2++) {
                    int off = tmp2 * width;
                    bool at_neg_1 = tmp2 == -1;
                    for (tmp1=-1; tmp1<2; tmp1++) {
                        tile = *(this+tmp1+off);
                        if (tile == TILE_WATER || tile == TILE_WATER_COIN || (at_neg_1 && tile == TILE_WATER_TOP)) {
                            *this = TILE_WATER_COIN;
                            goto end_loops;
                        }
                    }
                }
            end_loops:
                break;
            case 0x61:
                add_simple_enemy(this, CANNONBALL_DOWN_CREATOR_TYPE);
                break;
            case 0x53:
                add_simple_enemy(this, CANNONBALL_UP_CREATOR_TYPE);
                break;
            case 0x46:
                add_simple_enemy(this, BULLET_CREATOR_TYPE);
                break;
            case TILE_E_ORIAM_START:
                tile_to_abs_xy_pos(this, (unsigned int*)&oiram.x, (unsigned int*)&oiram.y);
                if (oiram.flags & FLAG_OIRAM_BIG) {
                    oiram.y -= TILE_HEIGHT + 2;
                }
                goto SET_EMPTY_TILE;
            case TILE_E_RESWOB:
                add_reswob(this);
                goto SET_EMPTY_TILE;
            case TILE_E_FISH:
                add_simple_enemy(this, FISH_TYPE);
                *this = TILE_WATER;
                break;
            case TILE_E_GOOMBA:
                add_goomba(this);
                goto SET_EMPTY_TILE;
            case TILE_E_SPIKE:
            case TILE_E_GREEN_KOOPA:
            case TILE_E_RED_KOOPA:
            case TILE_E_GREEN_FLY_KOOPA:
            case TILE_E_RED_FLY_KOOPA:
            case TILE_E_BONES_KOOPA:
                add_shell_enemy(this, tile - TILE_E_GREEN_KOOPA);
                goto SET_EMPTY_TILE;
            case TILE_E_THWOMP:
                add_thwomp(this);
                goto SET_EMPTY_TILE;
            case TILE_E_LAVA_FIREBALL:
                add_flame(this);
                *this = TILE_LAVA_TOP;
                break;
            case TILE_E_CHOMPER:
            case TILE_E_FIRE_CHOMPER:
                add_chomper(this + width, tile == TILE_E_FIRE_CHOMPER);
                goto SET_EMPTY_TILE;
            case TILE_E_BOO:
                add_boo(this);
            SET_EMPTY_TILE:
                *this = TILE_EMPTY;
                break;
            default:
                break;
        }
    }
}
