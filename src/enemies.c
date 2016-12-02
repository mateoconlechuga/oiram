// standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include "simple_mover.h"
#include "enemies.h"
#include "tile_handlers.h"
#include "tilemapdata.h"
#include "defines.h"

unsigned int goomba_data[] = {
    10,       // number of goombas
    16,
    22,
    30,
    40,
    45,
    17,
    24,
    32,
    34,
    19,
};

unsigned int fire_flame_data[] = {
    0,       // number of fire flames
    13 + (50 * 6),
};

unsigned int thwomp_data[] = {
    0,       // number of thwomps
    2,
    6
};

unsigned int chomper_data[] = {
    0,
    20+(3*50) | 0x800000,
    16+(15*50),
};

unsigned int boo_data[] = {
    0,
    20
};

unsigned int koopa_data[] = {
    5,       // number of koopas
    (10 + 11*50) | 1<<22,
    10,
    13,
    26,
    48,
    5,
};

unsigned int simple_enemy_data[] = {
    0,       // number of bullets/cannonballs
    11+ (50*4),
};

void add_goomba(uint8_t *tile) {
    simple_move_t *goomba = add_simple_mover(tile);
    
    if (!goomba) {
        return;
    }
    
    goomba->hitbox.width = 15;
    goomba->hitbox.height = 15;
    goomba->vx = -1;
    goomba->type = GOOMBA_TYPE;
}

boo_t *boo[MAX_BOOS];
uint8_t num_boos = 0;

void add_boo(uint8_t *tile) {
    boo_t *booooo;
    unsigned int x, y;
    
    if (num_boos > MAX_BOOS - 1) {
        return;
    }
    
    tile_to_abs_xy_pos(tile, &x, &y);
    
    booooo = boo[num_boos] = malloc(sizeof(boo_t));
    
    if (!booooo) {
        return;
    }
    
    booooo->x = x;
    booooo->y = y;
    booooo->vy = -1;
    booooo->dir = false;
    booooo->count = 0;
    num_boos++;
}

void remove_boo(uint8_t i) {
    boo_t *free_me = boo[i];
    uint8_t num_boos_less = num_boos--;
    
    for(; i < num_boos_less; i++) {
        boo[i] = boo[i+1];
    }
    
    free(free_me);
}

void add_koopa(uint8_t *tile, uint8_t type) {
    simple_move_t *koopa = add_simple_mover(tile);
    
    if (!koopa) {
        return;
    }
    
    koopa->hitbox.width = 15;
    koopa->hitbox.height = 26;
    koopa->vx = -1;
    switch(type) {
        case 0:
            koopa->type = KOOPA_GREEN_TYPE;
            koopa->smart = false;
            break;
        case 1:
            koopa->type = KOOPA_RED_TYPE;
            koopa->smart = true;
            break;
        case 2:
            koopa->type = KOOPA_GREEN_FLY_TYPE;
            koopa->smart = false;
            koopa->vx = 0;
            koopa->vy = 1;
            koopa->is_flyer = true;
            break;
        case 3:
            koopa->type = KOOPA_RED_FLY_TYPE;
            koopa->smart = true;
            koopa->vx = 0;
            koopa->vy = 1;
            koopa->is_flyer = true;
            break; 
        case 4:
            koopa->type = KOOPA_BONES_TYPE;
            koopa->smart = true;
            break;
        default:
            abort();
    }
}

chomper_t *chomper[MAX_CHOMPERS];
uint8_t num_chompers = 0;

void add_chomper(uint8_t *tile, bool throws_fire) {
    chomper_t *chomp;
    unsigned int x, y;
    
    if (num_chompers > MAX_CHOMPERS - 1) {
        return;
    }
    
    tile_to_abs_xy_pos(tile, &x, &y);
    
    chomp = chomper[num_chompers] = malloc(sizeof(chomper_t));
    
    if (!chomp) {
        return;
    }
    
    chomp->x = x + TILE_WIDTH/2;
    chomp->y = y;
    chomp->start_y = y;
    chomp->vy = -2;
    chomp->throws_fire = throws_fire;
    chomp->count = 0;
    num_chompers++;
}

void remove_chomper(uint8_t i) {
    chomper_t *free_me = chomper[i];
    uint8_t num_chompers_less = num_chompers--;
    
    for(; i < num_chompers_less; i++) {
        chomper[i] = chomper[i+1];
    }
    
    free(free_me);
}


flame_t *flame[MAX_FLAMES];
uint8_t num_flames = 0;

void add_flame(uint8_t *tile) {
    flame_t *fire;
    unsigned int x, y;
    
    if (num_flames > MAX_FLAMES - 1) {
        return;
    }
    
    tile_to_abs_xy_pos(tile, &x, &y);
    
    fire = flame[num_flames] = malloc(sizeof(flame_t));
    
    if (!fire) {
        return;
    }
    
    fire->x = x;
    fire->y = y;
    fire->start_y = y;
    fire->vy = -15;
    fire->count = 0;
    num_flames++;
}

void remove_flame(uint8_t i) {
    flame_t *free_me = flame[i];
    uint8_t num_flames_less = num_flames--;
    
    for(; i < num_flames_less; i++) {
        flame[i] = flame[i+1];
    }
    
    free(free_me);
}

thwomp_t *thwomp[MAX_THWOMPS];
uint8_t num_thwomps = 0;

void add_thwomp(uint8_t *tile) {
    thwomp_t *womp;
    unsigned int x, y;
    
    if (num_thwomps > MAX_THWOMPS - 1) {
        return;
    }
    
    tile_to_abs_xy_pos(tile, &x, &y);
    
    womp = thwomp[num_thwomps] = malloc(sizeof(thwomp_t));
    
    if (!womp) {
        return;
    }
    
    womp->x = x + 4;
    womp->y = y;
    womp->start_y = y;
    womp->vy = 0;
    womp->count = 0;
    num_thwomps++;
}

void remove_thwomp(uint8_t i) {
    thwomp_t *free_me = thwomp[i];
    uint8_t num_thowmps_less = num_thwomps--;
    
    for(; i < num_thowmps_less; i++) {
        thwomp[i] = thwomp[i+1];
    }
    
    free(free_me);
}

enemy_t *simple_enemy[MAX_SIMPLE_ENEMY];
uint8_t num_simple_enemies = 0;

enemy_t *add_simple_enemy(uint8_t *tile, uint8_t type) {
    enemy_t *enemy;
    unsigned int x, y;
    
    if (num_simple_enemies > MAX_SIMPLE_ENEMY - 1) {
        return NULL;
    }
    
    tile_to_abs_xy_pos(tile, &x, &y);
    
    enemy = simple_enemy[num_simple_enemies] = malloc(sizeof(enemy_t));
    
    if (!enemy) {
        return NULL;
    }
    
    enemy->vx = 0;
    enemy->vy = 0;
    
    switch(type) {
        case BULLET_TYPE:
            enemy->vx = -2;
            enemy->y = y + 1;
            enemy->x = x - 4;
            break;
        default:
            enemy->y = y;
            enemy->x = x;
            break;
    }
    
    enemy->counter = 100;
    enemy->type = type;
    num_simple_enemies++;
    return enemy;
}

void remove_simple_enemy(uint8_t i) {
    enemy_t *free_me = simple_enemy[i];
    uint8_t num_simple_enemies_less = num_simple_enemies--;
    
    for(; i < num_simple_enemies_less; i++) {
        simple_enemy[i] = simple_enemy[i+1];
    }
    
    free(free_me);
}

void get_enemies(void) {
    unsigned int j;
    uint8_t i;
    uint8_t num_goombas = *goomba_data;
    uint8_t num_red_koopas = *koopa_data;
    uint8_t num_chomps = *chomper_data;
    uint8_t num_flamers = *fire_flame_data;
    uint8_t num_womps = *thwomp_data;
    uint8_t num_boooos = *boo_data;
    uint8_t num_simples = *simple_enemy_data;
    
    for(i = 1; i <= num_goombas; i++) {
        add_goomba(tilemapdata + goomba_data[i]);
    }
    
    for(i = 1; i <= num_red_koopas; i++) {
        unsigned int koop = koopa_data[i];
        uint8_t type;
        
        switch((koop & ((1<<23) | (1<<22) | (1<<21) | (1<<20)))) {
            case 1<<23:
                type = 4;
                break;
            case 1<<22:
                type = 3;
                break;
            case 1<<21:
                type = 2;
                break;
            case 1<<20:
                type = 1;
                break;
            default:
                type = 0;
                break;
        }
        
        add_koopa(tilemapdata + (koop & ~((1<<23) | (1<<22) | (1<<21) | (1<<20))), type);
    }
    
    for(i = 1; i <= num_chomps; i++) {
        unsigned int chomp = chomper_data[i];
        add_chomper(tilemapdata + (chomp & ~(1<<23)), (bool)((chomp & (1<<23)) == (1<<23)));
    }
    
    for(i = 1; i <= num_flamers; i++) {
        add_flame(tilemapdata + fire_flame_data[i]);
    }
    
    for(i = 1; i <= num_womps; i++) {
        add_thwomp(tilemapdata + thwomp_data[i]);
    }
    
    for(i = 1; i <= num_boooos; i++) {
        add_boo(tilemapdata + boo_data[i]);
    }
    
    for(j = 0; j < sizeof(tilemapdata); j++) {
        uint8_t tile = tilemapdata[j];
        if (tile == 0x61) {
            add_simple_enemy(tilemapdata + j, CANNONBALL_DOWN_CREATOR_TYPE);
        }
        if (tile == 0x53) {
            add_simple_enemy(tilemapdata + j, CANNONBALL_UP_CREATOR_TYPE);
        }
        if (tile == 0x46) {
            add_simple_enemy(tilemapdata + j, BULLET_CREATOR_TYPE);
        }
    }
    
}
