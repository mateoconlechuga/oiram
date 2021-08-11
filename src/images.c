#include <stdlib.h>

#include <graphx.h>
#include <fileioc.h>
#include <debug.h>

#include "images.h"
#include "lower.h"
#include "defines.h"
#include "tile_handlers.h"

gfx_sprite_t *tileset_tiles[256];

gfx_sprite_t oiram_0_buffer_left[27*27 + 2];
gfx_sprite_t oiram_1_buffer_left[27*27 + 2];
gfx_sprite_t oiram_0_buffer_right[27*27 + 2];
gfx_sprite_t oiram_1_buffer_right[27*27 + 2];

gfx_sprite_t *oiram_0_small;
gfx_sprite_t *oiram_1_small;
gfx_sprite_t *oiram_0_big;
gfx_sprite_t *oiram_1_big;
gfx_sprite_t *oiram_0_fire;
gfx_sprite_t *oiram_1_fire;
gfx_sprite_t *oiram_0_racoon;
gfx_sprite_t *oiram_1_racoon;
gfx_sprite_t *oiram_crouch_big;
gfx_sprite_t *oiram_crouch_fire;
gfx_sprite_t *oiram_crouch_racoon;
gfx_sprite_t *oiram_fail;
gfx_sprite_t *mushroom;
gfx_sprite_t *fire_flower;
gfx_sprite_t *goomba_0;
gfx_sprite_t *goomba_1;
gfx_sprite_t *goomba_flat;
gfx_sprite_t *koopa_red_right_0;
gfx_sprite_t *koopa_red_right_1;
gfx_sprite_t *koopa_red_left_0;
gfx_sprite_t *koopa_red_left_1;
gfx_sprite_t *koopa_red_shell_0;
gfx_sprite_t *koopa_red_shell_1;
gfx_sprite_t *koopa_green_right_0;
gfx_sprite_t *koopa_green_right_1;
gfx_sprite_t *koopa_green_left_0;
gfx_sprite_t *koopa_green_left_1;
gfx_sprite_t *koopa_green_shell_0;
gfx_sprite_t *koopa_green_shell_1;
gfx_sprite_t *koopa_bones_right_0;
gfx_sprite_t *koopa_bones_right_1;
gfx_sprite_t *koopa_bones_left_0;
gfx_sprite_t *koopa_bones_left_1;
gfx_sprite_t *koopa_bones_dead_left;
gfx_sprite_t *koopa_bones_dead_right;
gfx_sprite_t *chomper_0;
gfx_sprite_t *chomper_1;
gfx_sprite_t *chomper_fire_down_left;
gfx_sprite_t *chomper_fire_down_right;
gfx_sprite_t *chomper_fire_up_left;
gfx_sprite_t *chomper_fire_up_right;
gfx_sprite_t *chomper_body;
gfx_sprite_t *fire_0;
gfx_sprite_t *fire_1;
gfx_sprite_t *poof_0;
gfx_sprite_t *poof_1;
gfx_sprite_t *flame_fire_up_0;
gfx_sprite_t *flame_fire_up_1;
gfx_sprite_t *flame_fire_down_0;
gfx_sprite_t *flame_fire_down_1;
gfx_sprite_t *thwomp_0;
gfx_sprite_t *boo_left_hide;
gfx_sprite_t *boo_right_hide;
gfx_sprite_t *boo_left;
gfx_sprite_t *boo_right;
gfx_sprite_t *bullet_left;
gfx_sprite_t *cannonball_sprite;
gfx_sprite_t *wing_left_0;
gfx_sprite_t *wing_left_1;
gfx_sprite_t *wing_right_0;
gfx_sprite_t *wing_right_1;
gfx_sprite_t *star_0;
gfx_sprite_t *easter_egg_0;
gfx_sprite_t *easter_egg_1;
gfx_sprite_t *oiram_lives;
gfx_sprite_t *timer;
gfx_sprite_t *one_up;
gfx_sprite_t *oiram_up_small_0;
gfx_sprite_t *oiram_up_small_1;
gfx_sprite_t *oiram_up_big_0;
gfx_sprite_t *oiram_up_big_1;
gfx_sprite_t *oiram_up_fire_0;
gfx_sprite_t *oiram_up_fire_1;
gfx_sprite_t *fish_left_0;
gfx_sprite_t *fish_left_1;
gfx_sprite_t *fish_right_0;
gfx_sprite_t *fish_right_1;
gfx_sprite_t *mushroom_1up;
gfx_sprite_t *spike_left_0;
gfx_sprite_t *spike_left_1;
gfx_sprite_t *spike_right_0;
gfx_sprite_t *spike_right_1;
gfx_sprite_t *spike_shell_0;
gfx_sprite_t *spike_shell_1;

gfx_sprite_t *fireball_sprite;
gfx_sprite_t *goomba_sprite;
gfx_sprite_t *chomper_sprite;
gfx_sprite_t *wing_left_sprite;
gfx_sprite_t *wing_right_sprite;
gfx_sprite_t *koopa_red_left_sprite;
gfx_sprite_t *koopa_red_right_sprite;
gfx_sprite_t *koopa_green_left_sprite;
gfx_sprite_t *koopa_green_right_sprite;
gfx_sprite_t *koopa_bones_left_sprite;
gfx_sprite_t *koopa_bones_right_sprite;
gfx_sprite_t *spike_left_sprite;
gfx_sprite_t *spike_right_sprite;
gfx_sprite_t *flame_sprite_up;
gfx_sprite_t *flame_sprite_down;
gfx_sprite_t *fish_left_sprite;
gfx_sprite_t *fish_right_sprite;
gfx_sprite_t *coin_sprite;

gfx_sprite_t *oiram_logo;

gfx_sprite_t *score_100;
gfx_sprite_t *score_200;
gfx_sprite_t *score_400;
gfx_sprite_t *score_800;
gfx_sprite_t *score_1000;
gfx_sprite_t *score_2000;
gfx_sprite_t *score_4000;
gfx_sprite_t *score_8000;

gfx_sprite_t *leaf_left;
gfx_sprite_t *leaf_right;
gfx_sprite_t *oiram_start;

gfx_sprite_t *oiram_up_racoon_0;
gfx_sprite_t *oiram_up_racoon_1;

gfx_sprite_t *tail_left_0;
gfx_sprite_t *tail_right_0;

gfx_rletsprite_t *reswob_left_0;
gfx_rletsprite_t *reswob_left_1;
gfx_rletsprite_t *reswob_right_0;
gfx_rletsprite_t *reswob_right_1;
gfx_rletsprite_t *reswob_down;

gfx_sprite_t *door_top;
gfx_sprite_t *door_bot;

gfx_sprite_t *oiram_right[] = {
    oiram_0_buffer_right, oiram_1_buffer_right
};

gfx_sprite_t *oiram_left[] = {
    oiram_0_buffer_left, oiram_1_buffer_left
};

void extract_tiles(void) {
    uint8_t slot, i;
    gfx_sprite_t *tile_question_box;
    gfx_sprite_t *empty;
    uint16_t pal_size;
    uint16_t *pal_ptr;
    uint8_t *tmp_ptr;
    gfx_sprite_t **tiles = tileset_tiles;

    slot = ti_Open("OiramT", "r");
    if (!slot) {
        missing_appvars();
    }

    pal_ptr = (uint16_t*)ti_GetDataPtr(slot);
    pal_size = *pal_ptr;
    pal_ptr++;

    // set up the palette
    gfx_SetPalette(pal_ptr, pal_size, 0);
    tmp_ptr = (uint8_t*)pal_ptr;
    tmp_ptr += pal_size;

    // get the question block tile
    tile_question_box = (gfx_sprite_t*)tmp_ptr;

    for(i = 0; i++;) {
        tiles[i] = tile_question_box;
    }

    for(i = 0; i < 252; i++) {
        tiles[i] = (gfx_sprite_t*)tmp_ptr;
        tmp_ptr += TILE_DATA_SIZE;
    }

    // close the open file
    ti_Close(slot);

    // extract common sprite tiles
    empty       = tiles[TILE_EMPTY];
    coin_sprite = tiles[TILE_COIN];
    door_top    = tiles[228];
    door_bot    = tiles[229];

	// snow tiles fixup
    tiles[TILE_SNOW_TL] = tiles[239];
    tiles[TILE_SNOW_TM] = tiles[240];
    tiles[TILE_SNOW_TR] = tiles[241];
    tiles[TILE_SNOW_BL] = tiles[242];
    tiles[TILE_SNOW_BM] = tiles[243];
    tiles[TILE_SNOW_BR] = tiles[244];

    // handling ice/coins is rather odd...
    tiles[TILE_BLUE_COIN] = tiles[247];
    tiles[TILE_ICE_COIN]  = tiles[238];
    tiles[TILE_ICE]       = tiles[230];

    // end pipe fixup
    tiles[TILE_END_PIPE_L] = tiles[245];
    tiles[TILE_END_PIPE_R] = tiles[246];

    // blue stuffs
    tiles[TILE_BLUE_BRICK_X] = tiles[TILE_EMPTY];
    tiles[TILE_BLUE_COIN_X]  = tiles[TILE_EMPTY];

    // handle the question boxes
    tiles[TILE_COIN_BOX]       = tile_question_box;
    tiles[TILE_1UP_BOX]        = tile_question_box;
    tiles[TILE_MUSHROOM_BOX]   = tile_question_box;
    tiles[TILE_STAR_BOX]       = tile_question_box;
    tiles[TILE_FIREFLOWER_BOX] = tile_question_box;
    tiles[TILE_LEAF_BOX]       = tile_question_box;
}

void extract_sprites(void) {
    uint8_t slot;

    slot = ti_Open("OiramS", "r");
    if (slot) {
        uint8_t *spr_ptr = ti_GetDataPtr(slot);

        oiram_0_small = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        oiram_1_small = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        oiram_0_big = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 434;
        oiram_1_big = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 434;
        oiram_0_fire = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 434;
        oiram_1_fire = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 434;
        oiram_crouch_big = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        oiram_crouch_fire = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        oiram_fail = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        mushroom = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        fire_flower = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        goomba_sprite = goomba_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        goomba_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        goomba_flat = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 146;
        koopa_red_right_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 434;
        koopa_red_right_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 434;
        koopa_red_left_sprite = koopa_red_left_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 434;
        koopa_red_right_sprite = koopa_red_left_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 434;
        koopa_red_shell_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        koopa_red_shell_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        koopa_green_right_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 434;
        koopa_green_right_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 434;
        koopa_green_left_sprite = koopa_green_left_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 434;
        koopa_green_right_sprite = koopa_green_left_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 434;
        koopa_green_shell_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        koopa_green_shell_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        koopa_bones_right_sprite = koopa_bones_right_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 434;
        koopa_bones_right_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 434;
        koopa_bones_left_sprite = koopa_bones_left_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 434;
        koopa_bones_left_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 434;
        koopa_bones_dead_left = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 290;
        koopa_bones_dead_right = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 290;
        chomper_sprite = chomper_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        chomper_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        chomper_fire_down_left = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        chomper_fire_down_right = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        chomper_fire_up_left = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        chomper_fire_up_right = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        chomper_body = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        fire_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 66;
        fire_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 66;
        poof_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 146;
        poof_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 146;
        flame_sprite_up = flame_fire_up_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 226;
        flame_fire_up_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 226;
        flame_sprite_down = flame_fire_down_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 226;
        flame_fire_down_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 226;
        thwomp_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 770;
        boo_left_hide = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        boo_right_hide = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        boo_left = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        boo_right = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        bullet_left = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 226;
        cannonball_sprite = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 146;
        wing_left_sprite = wing_left_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 106;
        wing_left_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 106;
        wing_right_sprite = wing_right_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 106;
        wing_right_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 106;
        star_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        easter_egg_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        easter_egg_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        oiram_lives = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 93;
        timer = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 83;
        oiram_score_chain_sprites[8] = one_up = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 380;
        oiram_up_small_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        oiram_up_small_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        oiram_up_big_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 402;
        oiram_up_big_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 402;
        oiram_up_fire_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 402;
        oiram_up_fire_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 402;
        fish_left_sprite = fish_left_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        fish_left_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        fish_right_sprite = fish_right_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        fish_right_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        mushroom_1up = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        spike_left_sprite = spike_left_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        spike_left_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        spike_right_sprite = spike_right_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        spike_right_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        spike_shell_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        spike_shell_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        oiram_logo = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 442;
        oiram_score_chain_sprites[0] = score_100 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 90;
        oiram_score_chain_sprites[1] =  score_200 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 98;
        oiram_score_chain_sprites[2] = score_400 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 98;
        oiram_score_chain_sprites[3] = score_800 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 98;
        oiram_score_chain_sprites[4] = score_1000 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 122;
        oiram_score_chain_sprites[5] = score_2000 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 130;
        oiram_score_chain_sprites[6] = score_4000 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 130;
        oiram_score_chain_sprites[7] = score_8000 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 130;
        leaf_left = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 226;
        leaf_right = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 226;
        oiram_0_racoon = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 434;
        oiram_1_racoon = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 434;
        oiram_crouch_racoon = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 258;
        oiram_up_racoon_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 434;
        oiram_up_racoon_1 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 434;
        tail_left_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 37;
        tail_right_0 = (gfx_sprite_t*)spr_ptr;
        spr_ptr += 37;
        reswob_left_0 = (gfx_rletsprite_t*)spr_ptr;
        spr_ptr += 994;
        reswob_left_1 = (gfx_rletsprite_t*)spr_ptr;
        spr_ptr += 942;
        reswob_right_0 = (gfx_rletsprite_t*)spr_ptr;
        spr_ptr += 990;
        reswob_right_1 = (gfx_rletsprite_t*)spr_ptr;
        spr_ptr += 937;
        reswob_down = (gfx_rletsprite_t*)spr_ptr;
        spr_ptr += 1029;
        oiram_start = (gfx_sprite_t*)spr_ptr;
        ti_Close(slot);
    } else {
        missing_appvars();
    }
}
