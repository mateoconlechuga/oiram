#include <stdint.h>
#include <graphx.h>

#include "images.h"
#include "defines.h"
#include "oiram.h"
#include "lower.h"
#include "enemies.h"

const unsigned int oiram_score_chain[] = { 100, 200, 400, 800, 1000, 2000, 4000, 8000, ONE_UP_SCORE };
gfx_sprite_t *oiram_score_chain_sprites[9];

void draw_time(void) {
    gfx_SetTextXY(285, 144);
    gfx_PrintUInt(game.seconds, 3);
    gfx_BlitLines(gfx_buffer, 144, 8);
}

void draw_score(void) {
    gfx_SetTextXY(253, 164);
    gfx_PrintUInt(game.score, 7);
    gfx_BlitLines(gfx_buffer, 161, 14);
}

void draw_level(void) {
    gfx_PrintStringXY("LEVEL ", 12, 207);
    gfx_PrintUInt(game.level + 1, 3);
    gfx_BlitLines(gfx_buffer, 207, 8);
}

void draw_coins(void) {
    gfx_SetTextXY(29, 144);
    gfx_PrintUInt(game.coins, 2);
    gfx_BlitLines(gfx_buffer, 144, 8);
}

void draw_lives(void) {
    gfx_SetTextXY(29, 164);
    gfx_PrintUInt(oiram.lives, 2);
    gfx_BlitLines(gfx_buffer, 164, 8);
}

void add_life(void) {
    oiram.lives++;
    if (oiram.lives > 99) { oiram.lives = 99; }
    draw_lives();
}

// add a coin, if we reach 100 coins, add another life
void add_coin(int x, int y) {
    game.coins++;
    if (game.coins == 100) {
        game.coins = 0;
        add_life();
    }
    draw_coins();
    add_score(1, x, y);
}

void add_next_chain_score(int x, int y) {
    add_score(oiram.score_chain, x, y);
    if(oiram.score_chain != 8) {
        oiram.score_chain++;
    }
}

void add_score(uint8_t add, int x, int y) {
    enemy_t *score = add_simple_enemy(NULL, SCORE_TYPE);
    score->x = x;
    score->y = y;
    score->sprite = oiram_score_chain_sprites[add];
    if (add == ONE_UP_SCORE) {
        add_life();
    } else {
        game.score += oiram_score_chain[add];
    }
    draw_score();
}

void add_score_no_sprite(uint8_t add) {
    if (add == ONE_UP_SCORE) {
        add_life();
    } else {
        game.score += oiram_score_chain[add];
    }
    draw_score();
}

