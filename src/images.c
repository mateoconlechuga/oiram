#include <stdlib.h>

#include <lib/ce/graphx.h>
#include <lib/ce/fileioc.h>

#include "images.h"
#include "defines.h"

gfx_image_t mario_0_buffer_left[27*27 + 2];
gfx_image_t mario_1_buffer_left[27*27 + 2];
gfx_image_t mario_0_buffer_right[27*27 + 2];
gfx_image_t mario_1_buffer_right[27*27 + 2];

gfx_image_t *mario_0_small;
gfx_image_t *mario_1_small;
gfx_image_t *mario_0_big;
gfx_image_t *mario_1_big;
gfx_image_t *mario_0_fire;
gfx_image_t *mario_1_fire;
gfx_image_t *mario_crouch_big;
gfx_image_t *mario_crouch_fire;
gfx_image_t *mario_fail;
gfx_image_t *mushroom;
gfx_image_t *fire_flower;
gfx_image_t *goomba_0;
gfx_image_t *goomba_1;
gfx_image_t *goomba_flat;
gfx_image_t *koopa_red_right_0;
gfx_image_t *koopa_red_right_1;
gfx_image_t *koopa_red_left_0;
gfx_image_t *koopa_red_left_1;
gfx_image_t *koopa_red_shell_0;
gfx_image_t *koopa_red_shell_1;
gfx_image_t *koopa_green_right_0;
gfx_image_t *koopa_green_right_1;
gfx_image_t *koopa_green_left_0;
gfx_image_t *koopa_green_left_1;
gfx_image_t *koopa_green_shell_0;
gfx_image_t *koopa_green_shell_1;
gfx_image_t *koopa_bones_right_0;
gfx_image_t *koopa_bones_right_1;
gfx_image_t *koopa_bones_left_0;
gfx_image_t *koopa_bones_left_1;
gfx_image_t *koopa_bones_dead_left;
gfx_image_t *koopa_bones_dead_right;
gfx_image_t *chomper_0;
gfx_image_t *chomper_1;
gfx_image_t *chomper_fire_down_left;
gfx_image_t *chomper_fire_down_right;
gfx_image_t *chomper_fire_up_left;
gfx_image_t *chomper_fire_up_right;
gfx_image_t *chomper_body;
gfx_image_t *fire_0;
gfx_image_t *fire_1;
gfx_image_t *poof_0;
gfx_image_t *poof_1;
gfx_image_t *flame_fire_up_0;
gfx_image_t *flame_fire_up_1;
gfx_image_t *flame_fire_down_0;
gfx_image_t *flame_fire_down_1;
gfx_image_t *thwomp_0;
gfx_image_t *boo_left_hide;
gfx_image_t *boo_right_hide;
gfx_image_t *boo_left;
gfx_image_t *boo_right;
gfx_image_t *bullet_left;
gfx_image_t *bullet_right;
gfx_image_t *cannonball_sprite;
gfx_image_t *wing_left_0;
gfx_image_t *wing_left_1;
gfx_image_t *wing_right_0;
gfx_image_t *wing_right_1;
gfx_image_t *star_0;
gfx_image_t *easter_egg_0;
gfx_image_t *easter_egg_1;
gfx_image_t *mario_lives;
gfx_image_t *clock;
gfx_image_t *one_up;
gfx_image_t *mario_up_small_0;
gfx_image_t *mario_up_small_1;
gfx_image_t *mario_up_big_0;
gfx_image_t *mario_up_big_1;
gfx_image_t *mario_up_fire_0;
gfx_image_t *mario_up_fire_1;
gfx_image_t *fish_left_0;
gfx_image_t *fish_left_1;
gfx_image_t *fish_right_0;
gfx_image_t *fish_right_1;
gfx_image_t *mushroom_1up;

gfx_image_t *fireball_sprite;
gfx_image_t *goomba_sprite;
gfx_image_t *chomper_sprite;
gfx_image_t *wing_left_sprite;
gfx_image_t *wing_right_sprite;
gfx_image_t *koopa_red_left_sprite;
gfx_image_t *koopa_red_right_sprite;
gfx_image_t *koopa_green_left_sprite;
gfx_image_t *koopa_green_right_sprite;
gfx_image_t *koopa_bones_left_sprite;
gfx_image_t *koopa_bones_right_sprite;
gfx_image_t *flame_sprite_up;
gfx_image_t *flame_sprite_down;
gfx_image_t *fish_left_sprite;
gfx_image_t *fish_right_sprite;

gfx_image_t *mario_right[] = {
    mario_0_buffer_right, mario_1_buffer_right
};

gfx_image_t *mario_left[] = {
    mario_0_buffer_left, mario_1_buffer_left
};

void extract_tiles(void) {
    uint8_t slot;
    
    ti_CloseAll();
    slot = ti_Open("MarioT", "r");
    if (slot) {
        uint8_t i;
        uint16_t pal_size;
        uint16_t *pal_ptr;
        uint8_t *tmp_ptr;

        pal_ptr = (uint16_t*)ti_GetDataPtr(slot);
        pal_size = *pal_ptr;
        pal_ptr++;
        
        // set up the palette
        gfx_SetPalette( pal_ptr, pal_size, 0 );
        tmp_ptr = (uint8_t*)pal_ptr;
        tmp_ptr += pal_size;
        
        for(i = 0; i < 224; i++) {
            tileset_tiles[i] = (gfx_image_t*)tmp_ptr;
            tmp_ptr += TILE_DATA_SIZE;
        }
        // now the order should be the same as in convpng.ini
        
    } else {
        exit(0);
    }
    ti_CloseAll();
}
    
void extract_sprites(void) {
    uint8_t slot;
    
    ti_CloseAll();
    slot = ti_Open("MarioS", "r");
    if (slot) {
        uint8_t *spr_ptr = ti_GetDataPtr(slot);
        
        mario_0_small = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        mario_1_small = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        mario_0_big = (gfx_image_t*)spr_ptr;
        spr_ptr += 434;
        mario_1_big = (gfx_image_t*)spr_ptr;
        spr_ptr += 434;
        mario_0_fire = (gfx_image_t*)spr_ptr;
        spr_ptr += 434;
        mario_1_fire = (gfx_image_t*)spr_ptr;
        spr_ptr += 434;
        mario_crouch_big = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        mario_crouch_fire = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        mario_fail = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        mushroom = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        fire_flower = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        goomba_sprite = goomba_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        goomba_1 = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        goomba_flat = (gfx_image_t*)spr_ptr;
        spr_ptr += 146;
        koopa_red_right_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 434;
        koopa_red_right_1 = (gfx_image_t*)spr_ptr;
        spr_ptr += 434;
        koopa_red_left_sprite = koopa_red_left_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 434;
        koopa_red_right_sprite = koopa_red_left_1 = (gfx_image_t*)spr_ptr;
        spr_ptr += 434;
        koopa_red_shell_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        koopa_red_shell_1 = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        koopa_green_right_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 434;
        koopa_green_right_1 = (gfx_image_t*)spr_ptr;
        spr_ptr += 434;
        koopa_green_left_sprite = koopa_green_left_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 434;
        koopa_green_right_sprite = koopa_green_left_1 = (gfx_image_t*)spr_ptr;
        spr_ptr += 434;
        koopa_green_shell_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        koopa_green_shell_1 = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        koopa_bones_right_sprite = koopa_bones_right_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 434;
        koopa_bones_right_1 = (gfx_image_t*)spr_ptr;
        spr_ptr += 434;
        koopa_bones_left_sprite = koopa_bones_left_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 434;
        koopa_bones_left_1 = (gfx_image_t*)spr_ptr;
        spr_ptr += 434;
        koopa_bones_dead_left = (gfx_image_t*)spr_ptr;
        spr_ptr += 290;
        koopa_bones_dead_right = (gfx_image_t*)spr_ptr;
        spr_ptr += 290;
        chomper_sprite = chomper_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        chomper_1 = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        chomper_fire_down_left = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        chomper_fire_down_right = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        chomper_fire_up_left = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        chomper_fire_up_right = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        chomper_body = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        fire_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 66;
        fire_1 = (gfx_image_t*)spr_ptr;
        spr_ptr += 66;
        poof_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 146;
        poof_1 = (gfx_image_t*)spr_ptr;
        spr_ptr += 146;
        flame_sprite_up = flame_fire_up_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 226;
        flame_fire_up_1 = (gfx_image_t*)spr_ptr;
        spr_ptr += 226;
        flame_sprite_down = flame_fire_down_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 226;
        flame_fire_down_1 = (gfx_image_t*)spr_ptr;
        spr_ptr += 226;
        thwomp_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 770;
        boo_left_hide = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        boo_right_hide = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        boo_left = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        boo_right = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        bullet_left = (gfx_image_t*)spr_ptr;
        spr_ptr += 226;
        bullet_right = (gfx_image_t*)spr_ptr;
        spr_ptr += 226;
        cannonball_sprite = (gfx_image_t*)spr_ptr;
        spr_ptr += 146;
        wing_left_sprite = wing_left_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 106;
        wing_left_1 = (gfx_image_t*)spr_ptr;
        spr_ptr += 106;
        wing_right_sprite = wing_right_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 106;
        wing_right_1 = (gfx_image_t*)spr_ptr;
        spr_ptr += 106;
        star_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        easter_egg_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        easter_egg_1 = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        mario_lives = (gfx_image_t*)spr_ptr;
        spr_ptr += 93;
        clock = (gfx_image_t*)spr_ptr;
        spr_ptr += 83;
        one_up = (gfx_image_t*)spr_ptr;
        spr_ptr += 380;
        mario_up_small_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        mario_up_small_1 = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        mario_up_big_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 402;
        mario_up_big_1 = (gfx_image_t*)spr_ptr;
        spr_ptr += 402;
        mario_up_fire_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 402;
        mario_up_fire_1 = (gfx_image_t*)spr_ptr;
        spr_ptr += 402;
        fish_left_sprite = fish_left_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        fish_left_1 = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        fish_right_sprite = fish_right_0 = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        fish_right_1 = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
        mushroom_1up = (gfx_image_t*)spr_ptr;
        spr_ptr += 258;
    } else {
        gfx_End();
        prgm_CleanUp();
        exit(0);
    }
    
    ti_CloseAll();
}