// standard headers
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>
#include <intce.h>
#include <debug.h>

// shared libraries
#include <lib/ce/graphx.h>
#include <lib/ce/keypadc.h>
#include <lib/ce/fileioc.h>

// mario stuffs
#include "tilemapdata.h"
#include "tile_handlers.h"
#include "defines.h"
#include "powerups.h"
#include "enemies.h"
#include "events.h"
#include "images.h"
#include "mario.h"
#include "lower.h"
#include "simple_mover.h"

map_t level_map;
tiles_struct_t tiles;
gfx_tilemap_t tilemap;
mario_t mario;

/* Place to hold decompressed tile pointers */
gfx_image_t *tileset_tiles[224];
game_t game;

/* This interrupt is called every second -- maybe you can do important things with it */
void interrupt isr_timer1(void) {
    
    seconds++;
    draw_time();
   
    // ack in the controller
    int_Acknowledge = INT_TIMER1;
    
    // ack in the timer
    timer_IntAcknowledge = TIMER1_RELOADED;
}

/* Interrupt routine to run when the [ON] key is pressed */
void interrupt isr_on(void) {
    /* Exit the "infinite" loop */
    game.exit = true;
    game.fastexit = true;
    
    /* Must acknowledge that the interrupt occured to clear the flag */
    int_Acknowledge = INT_ON;
}

void interrupt isr_keyboard(void) {
    /* Read the keypad data */
    kb_key_t g1_key = kb_Data[kb_group_1];
    kb_key_t g7_key = kb_Data[kb_group_7];
    
    pressed_yequ  = (g1_key & kb_Yequ);
    pressed_2nd   = (g1_key & kb_2nd);
    
    pressed_down  = (g7_key & kb_Down);
    pressed_up    = (g7_key & kb_Up);
    pressed_left  = (g7_key & kb_Left);
    pressed_right = (g7_key & kb_Right);
    
    /* Must acknowledge that the interrupt occured to clear the flag */
    int_Acknowledge = INT_KEYBOARD;
    
    /* Acknowledge in the keypad controller (Not technically required because interrupt controller handles signal) */
    kb_IntAcknowledge = KB_DATA_CHANGED;
}

void system_init(void) {
    // initialize the 8bpp graphics
    gfx_Begin( gfx_8bpp );
    
    // extract palette and tiles
    extract_tiles();
    
    // extract sprites from sprite file
    extract_sprites();
    
    if (mario.less) {
        mario_0_small = easter_egg_0;
        mario_1_small = easter_egg_1;
    }
    
    // copy the mario sprites to the temporary bufferes
    memcpy(mario_0_buffer_right, mario_0_small, 16*16+2);
    memcpy(mario_1_buffer_right, mario_1_small, 16*16+2);
    gfx_FlipSpriteY(mario_0_buffer_right, mario_0_buffer_left);
    gfx_FlipSpriteY(mario_1_buffer_right, mario_1_buffer_left);

    gfx_palette[BACKGROUND_COLOR_INDEX] = gfx_RGBTo1555( 176, 224, 248 );
    gfx_palette[BLACK_INDEX] = gfx_RGBTo1555( 0, 0, 0 );
    gfx_palette[WHITE_INDEX] = gfx_RGBTo1555( 255, 255, 255 );

    gfx_palette[252] = gfx_RGBTo1555( 24, 120, 184 );
    gfx_palette[251] = gfx_RGBTo1555( 64, 128, 240 );
    
    // draw to buffer to avoid tearing
    gfx_SetTextFGColor( WHITE_INDEX );
    gfx_SetTextBGColor( 252 );
    gfx_SetTextTransparentColor( 1 );
    
    gfx_SetClipRegion(X_OFFSET, Y_OFFSET, ((TILEMAP_DRAW_WIDTH-1) * TILE_WIDTH), ((TILEMAP_DRAW_HEIGHT-1) * TILE_HEIGHT));
    gfx_SetDrawBuffer();
}

void double_rectangle(uint24_t x, uint8_t y, uint24_t width, uint8_t height) {
    gfx_SetColor(WHITE_INDEX);
    gfx_Rectangle_NoClip(x, y, width, height);
    gfx_Rectangle_NoClip(x + 1, y + 1, width - 2, height - 2);
    gfx_SetColor(252);
    gfx_FillRectangle_NoClip(x + 2, y + 2, width - 4, height - 4);
    gfx_SetColor(BLACK_INDEX);
    gfx_SetPixel(x,y);
    gfx_SetPixel(x + width - 1, y);
    gfx_SetPixel(x + width - 1, y + height - 1);
    gfx_SetPixel(x, y + height - 1);
    gfx_SetColor(WHITE_INDEX);
    gfx_SetPixel(x + 2, y + 2);
    gfx_SetPixel(x + width - 3, y + 2);
    gfx_SetPixel(x + width - 3, y + height - 3);
    gfx_SetPixel(x + 2, y + height - 3);
}

void init_level(gfx_tilemap_t *tilemap, map_t *map, gfx_image_t **img_tiles, uint8_t *data, uint8_t width, uint8_t height) {
    map->width = width;
    map->height = height;
    map->max_y = height * TILE_HEIGHT;
    map->max_x = width * TILE_WIDTH;
    map->max_x_offset = map->max_x - ((TILEMAP_DRAW_WIDTH-1) * TILE_WIDTH);
    map->max_y_offset = map->max_y - ((TILEMAP_DRAW_HEIGHT-1) * TILE_HEIGHT);
    tilemap->map = map->data = data;
    tilemap->tiles = img_tiles;
    tilemap->type_width = gfx_tile_16_pixel;
    tilemap->type_height = gfx_tile_16_pixel;
    tilemap->tile_height = TILE_HEIGHT;
    tilemap->tile_width = TILE_WIDTH;
    tilemap->draw_height = TILEMAP_DRAW_HEIGHT;
    tilemap->draw_width = TILEMAP_DRAW_WIDTH;
    tilemap->width = width;
    tilemap->height = height;
    tilemap->y_loc = Y_OFFSET;
    tilemap->x_loc = X_OFFSET;
}

        
void main(void) {
    uint8_t radius;
    //mario.less = true;
    
    // init the system
    system_init();
    
    // init the tilemap structure
    init_level(&tilemap, &level_map, tileset_tiles, tilemapdata, 50, 25);

    game.fastexit = false;
    game.end_counter = 10;
    game.entered_end_pipe = false;
    
    mario.x = mario.y = 0;
    mario.curr_sprite = mario_right[0];
    mario.direction = FACE_RIGHT;
    mario.momentum = 0;
    mario.sprite_index = 0;
    mario.on_vine = false;
    mario.crouched = false;
    mario.lives = 5;
    mario.score_display_counter = 0;
    
    // configure hitbox
    mario.hitbox.width = 15;
    mario.hitbox.height = 15;
    mario.hitbox_height_half = 15/2;
    
    // load all the enemies in the level
    get_enemies();
    
    gfx_SetColor(WHITE_INDEX);
    double_rectangle(4, 133, 312, 48);
    double_rectangle(4, 194, 80, 33);
    gfx_TransparentSprite_NoClip(tilemap.tiles[TILE_COIN_0], 10, 140);
    gfx_TransparentSprite_NoClip(mario_lives, 10, 164);
    gfx_TransparentSprite_NoClip(clock, 270, 143);
    
    draw_coins();
    draw_time();
    draw_score();
    draw_lives();
    draw_world();
    
    gfx_BlitBuffer();
    
    // initialize the interrupt handlers
    int_Initialize();
    
    // disable the timer so it doesn't run when we don't want it to be running
    timer_Control = TIMER1_DISABLE;
    
    // by using the 32768 kHz clock, we can count for exactly a one second
    timer_1_ReloadValue = timer_1_Counter = 32768;
    
    // enable the timer, set it to the 32768 kHz clock, enable an interrupt once it reaches 0, and make it count down
    timer_Control = TIMER1_ENABLE | TIMER1_32K | TIMER1_0INT | TIMER1_DOWN;
    
    // setup the int vectors
    int_SetVector(ON_IVECT, isr_on);
    int_SetVector(KEYBOARD_IVECT, isr_keyboard);
    int_SetVector(TIMER1_IVECT, isr_timer1);
    
    // tell the interrupt controller that the ON flag should latch and be enabled
    int_EnableConfig = INT_ON | INT_KEYBOARD | INT_TIMER1;
    int_LatchConfig = INT_TIMER1;
    kb_EnableInt = KB_DATA_CHANGED;
    
    // configure the keypad to be continously scanning
    kb_SetMode(MODE_3_CONTINUOUS);
    
    // interrupts can now generate after this
    int_Enable();
    
    // wait until the clear key is pressed
    while(!game.exit) {
        
        // move mario if required
        move_mario();
        
        // draw the tilemap at the current mario offsets
        gfx_Tilemap(&tilemap, mario.scrollx, mario.scrolly);
        
        // handle outstanding events, such as showing number of coins
        handle_pending_events();
    
        // swap the draw buffer
        gfx_BlitLines(gfx_buffer, 0, 146);
        
        // animate the tiles
        animate();
    }
    
    if (!game.fastexit) {
        gfx_SetColor(BLACK_INDEX);
        for(radius = 5; radius < 255; radius += 5) {
            gfx_FillCircle(160,60,radius);
            gfx_BlitBuffer();
        }
        // if we entered the end pipe, proceed to next level if it exists
        if (game.entered_end_pipe) {
        }
    }
    
    // close the graphics and return to the OS
    int_Reset();
    gfx_End();
    prgm_CleanUp();
}
