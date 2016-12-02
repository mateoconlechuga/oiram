#include <stdint.h>
#include <lib/ce/graphx.h>

#include "images.h"
#include "defines.h"
#include "mario.h"
#include "lower.h"

uint8_t seconds = 0;
const unsigned int mario_score_chain[] = { 100, 200, 400, 500, 800, 1000, 2000, 4000, 5000, 8000, ONE_UP_SCORE };

void draw_time(void) {
    gfx_SetTextXY(285, 144);
    gfx_PrintUInt(seconds, 3);
    gfx_BlitLines(gfx_buffer, 144, 8);
}

void draw_score(void) {
    gfx_SetTextXY(253, 164);
    gfx_PrintUInt(mario.score, 7);
    gfx_BlitLines(gfx_buffer, 161, 14);
}

void draw_world(void) {
    gfx_PrintStringXY("WORLD 1", 19, 207);
    gfx_BlitLines(gfx_buffer, 207, 8);
}

void draw_coins(void) {
    gfx_SetTextXY(29, 144);
    gfx_PrintUInt(num_coins, 2);
    gfx_BlitLines(gfx_buffer, 144, 8);
}

void draw_lives(void) {
    gfx_SetTextXY(29, 164);
    gfx_PrintUInt(mario.lives, 2);
    gfx_BlitLines(gfx_buffer, 164, 8);
}

void add_life(void) {
    mario.lives++;
    gfx_TransparentSprite_NoClip(one_up, 140, 161);
    if (mario.lives > 99) { mario.lives = 99; }
    mario.score_display_counter = 2;
}

// add a coin, if we reach 100 coins, add another life
void add_coin(void) {
    num_coins++;
    if (num_coins == 100) {
        num_coins = 0;
        add_life();
    }
    draw_coins();
    add_score(200);
}

void add_next_chain_score(void) {
    add_score(mario_score_chain[mario.score_counter]);
    if(mario.score_counter != 10) { mario.score_counter++; }
}

void add_score(unsigned int add) {
    if (add == ONE_UP_SCORE) {
        add_life();
    } else {
        mario.score += add;
    }
    draw_score();
}