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

// oiram stuffs
#include "tile_handlers.h"
#include "defines.h"
#include "powerups.h"
#include "enemies.h"
#include "events.h"
#include "loadscreen.h"
#include "images.h"
#include "oiram.h"
#include "lower.h"
#include "simple_mover.h"

map_t level_map;
tiles_struct_t tiles;
gfx_tilemap_t tilemap;
oiram_t oiram;

/* Place to hold decompressed tile pointers */
gfx_image_t *tileset_tiles[256];
game_t game;

void *safe_malloc(size_t bytes) {
    uint8_t *data = malloc(bytes);
    if (!data) {
        save_progress();
        int_Reset();
        exit(0);
    }
    return (void*)data;
}

void missing_appvars(void) {
    gfx_End();
    prgm_CleanUp();
    os_SetCursorPos(0, 0);
    os_PutStrFull("Err: Missing Oiram AppVars");
    while(!os_GetCSC());
    prgm_CleanUp();
    exit(0);
}

/* This interrupt is called every second -- maybe you can do important things with it */
void interrupt isr_timer1(void) {
    
    game.seconds++;
    draw_time();
   
    // ack in the controller
    int_Acknowledge = INT_TIMER1;
    
    // ack in the timer
    timer_IntAcknowledge = TIMER1_RELOADED;
}

/* Interrupt routine to run when the [ON] key is pressed */
void interrupt isr_on(void) {
    /* Exit the "infinite" loop */
    if (!oiram.failed) {
        game.exit = true;
        game.fastexit = true;
    }
    
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

void black_circles(void) {
    uint8_t radius;
    for(radius = 5; radius < 255; radius += 5) {
            gfx_FillCircle(160,60,radius);
            gfx_BlitBuffer();
    }
}

void double_rectangle(uint24_t x, uint8_t y, uint24_t width, uint8_t height) {
    gfx_SetColor(WHITE_INDEX);
    gfx_Rectangle_NoClip(x, y, width, height);
    gfx_Rectangle_NoClip(x + 1, y + 1, width - 2, height - 2);
    gfx_SetColor(DARK_BLUE_INDEX);
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

static void print_centered(const char *string, const uint8_t ypos) {
    gfx_PrintStringXY(string, (LCD_WIDTH - gfx_GetStringWidth(string))/2, ypos);
}

static void extract_images(void) {
    // extract palette and tiles -- this is also where palette information is stored
    extract_tiles();
    
    gfx_palette[BLACK_INDEX] = gfx_RGBTo1555( 0, 0, 0 );
    gfx_palette[WHITE_INDEX] = gfx_RGBTo1555( 255, 255, 255 );
    gfx_palette[DARK_BLUE_INDEX] = gfx_RGBTo1555( 24, 120, 184 );
    if (oiram.less2) {
        unsigned int j;
        for (j=0; j<256; j++) {
            lcd_Palette[j] = ~lcd_Palette[j];
        }
    }
    
    // extract sprites from sprite file
    extract_sprites();
}

void main(void) {
    unsigned int delay;
    size_t pack_author_len;
    char end_str[100];
    real_t *real_in;
    pack_info_t *pack;
    
    // initialize the 8bpp graphics
    gfx_Begin( gfx_8bpp );
    
    gfx_SetDrawBuffer();
    
    // walrus mode!
    if(!ti_RclVar(TI_REAL_TYPE, ti_Ans, &real_in)) {
        int in = os_RealToInt24(real_in);
        if (in == 1337) { oiram.less = true; }
        if (in == 505)  { oiram.less2 = true; }
    }
    
    // init the state of the levels
    load_progress();
    
    // if we have no packs, error
    if (!num_packs) {
        missing_appvars();
    }
   
    // extract palette and tiles/sprites just to make sure they exist
    extract_images();
    
    main_start:
    
    // load the splash screen
    set_load_screen();
    
    draw_level:
    
    // extract palette and tiles/sprites
    extract_images();
    
    // draw the splash starting items
    gfx_FillScreen(BLACK_INDEX);
    
    gfx_TransparentSprite(oiram_logo, 116, 55);
    gfx_TransparentSprite(oiram_start, 165, 55);
    
    gfx_SetTextFGColor(WHITE_INDEX);
    gfx_SetTextBGColor(DARK_BLUE_INDEX);
    gfx_SetTextTransparentColor(BLACK_INDEX);
    
    // init the structs
    game.exit = false;
    game.fastexit = false;
    game.end_counter = 10;
    game.entered_end_pipe = false;
    game.seconds = 0;
    
    oiram.failed = false;
    oiram.started_fail = false;
    oiram.momentum = 0;
    oiram.sprite_index = 0;
    oiram.shrink_counter = 0;
    oiram.curr_sprite = oiram_right[0];
    oiram.direction = FACE_RIGHT;
    oiram.on_vine = false;
    oiram.crouched = false;
    oiram.has_shell = false;
    oiram.in_pipe = false;
    oiram.is_flying = false;
    oiram.scrollx = 0;
    oiram.scrolly = 0;
    oiram.x = 0;
    oiram.y = 0;
    oiram.fireballs = 0;
    
    // init the system
    tiles.animation_4_counter = 0;
    tiles.animation_3_counter = 0;
    tiles.animation_counter = 0;
    
    pack = &pack_info[game.pack];
    oiram.flags = pack->flags;
    oiram.lives = pack->lives;
    oiram.score = pack->score;
    oiram.coins = pack->coins;
    
    if (oiram.less) {
        oiram_0_small = easter_egg_0;
        oiram_1_small = easter_egg_1;
    }
    
    // copy the oiram sprites to the temporary bufferes
    set_normal_oiram_sprites();
    
    // load all the enemies in the level
    set_level(game.pack, game.level);
    get_enemies();
    compute_oiram_start_location();
    
    gfx_SetColor(WHITE_INDEX);
    double_rectangle(4, 133, 312, 48);
    double_rectangle(4, 194, 80, 33);
    gfx_TransparentSprite_NoClip(coin_sprite, 10, 140);
    gfx_TransparentSprite_NoClip(oiram_lives, 10, 164);
    gfx_TransparentSprite_NoClip(clock, 270, 143);
    
    gfx_Rectangle_NoClip(118, 145, 81, 4);
    
    draw_coins();
    draw_time();
    draw_score();
    draw_lives();
    draw_level();
    
    gfx_BlitBuffer();
    
    for(delay=0; delay<400000; delay++);
    
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
        
        // move oiram if required
        move_oiram();
        
        // draw the tilemap at the current oiram offsets
        gfx_Tilemap(&tilemap, oiram.scrollx, oiram.scrolly);
    
        // handle outstanding events, such as showing number of coins
        handle_pending_events();
    
        // swap the draw buffer
        gfx_BlitLines(gfx_buffer, 0, 146);
        
        // animate the things
        animate();
    }
    
    // reset interrupt status
    int_Reset();
    
    // deallocate
    while(num_simple_enemies) { remove_simple_enemy(0); }
    while(num_simple_movers)  { remove_simple_mover(0); }
    while(num_chompers)       { remove_chomper(0);      }
    while(num_thwomps)        { remove_thwomp(0);       }
    while(num_flames)         { remove_flame(0);        }
    while(num_boos)           { remove_boo(0);          }
    while(num_poofs)          { remove_poof(0);         }
    while(num_fireballs)      { remove_fireball(0);     }
    while(num_bumped_tiles)   { remove_bumped_tile(0);  }
    free(tilemap_data);
    
    gfx_SetColor(BLACK_INDEX);
    
    if (!game.fastexit) {
        pack->coins = oiram.coins;
        pack->score = oiram.score;
        pack->lives = oiram.lives;
        pack->flags = oiram.flags;
        
        black_circles();
        
        // if we entered the end pipe, proceed to next level if it exists
        if (game.entered_end_pipe) {
            if (game.level < game.end_level) {
                game.level++;
                if (pack->progress < game.level) { pack->progress = game.level; }
                if (game.level == game.end_level) {
                    goto pack_finish;
                } else {
                    set_level(game.pack, game.level);
                }
                goto draw_level;
            } else {
                if (pack->progress == game.end_level) {
                    goto pack_finish;
                }
                goto main_start;
            }
        } else {
            if (oiram.failed) {
                pack->lives--;
                pack->flags = FLAG_OIRAM_RESET;
                if (!pack->lives) {
                    goto game_over;
                } else {
                    goto draw_level;
                }
            }
        }
    } else {
        goto main_start;
    }

    goto exit;
    
game_over:
    gfx_SetClipRegion(0, 0, LCD_WIDTH, LCD_HEIGHT);
    gfx_SetTextBGColor(BLACK_INDEX);
    memset(pack, 0, sizeof(pack_info_t));
    pack->lives = 10;
    gfx_PrintStringXY("GAME      OVER", 120, 55);
    gfx_BlitBuffer();
    for(delay=0; delay<700010; delay++);
    goto main_start;
    
pack_finish:
    pack_author_len = strlen(pack_author);
    memset(end_str, 0, sizeof(end_str));
    memcpy(end_str, pack_author, pack_author_len);
    memcpy(end_str + pack_author_len, " thanks you for playing!", 24);
    
    gfx_SetClipRegion(0, 0, LCD_WIDTH, LCD_HEIGHT);
    for(delay=0;delay<130;delay++) {
        gfx_ShiftDown(1);
        gfx_BlitBuffer();
    }
    gfx_SetTextBGColor(BLACK_INDEX);
    print_centered("Pack Compelete!", 55);
    print_centered(end_str, 71);
    print_centered("Press [enter] to continue", 91);
    gfx_BlitBuffer();
    while(os_GetCSC() != sk_Enter);
    
    goto main_start;
    
exit:
    // save the pack states
    save_progress();
}
