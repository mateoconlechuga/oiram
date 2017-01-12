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

uint8_t *tilemap_data;
unsigned int *warp_pipe_info;
unsigned int pipe_max_tests;

pack_info_t pack_info[256];

static uint8_t *search_pos = NULL;
static char *var_name;
static const char search_string[] = { 0xAB, 0xCD, 0x00 };
static const char *save_name = "OiramSV";
uint8_t num_packs = 0;
char *pack_author;

// this routine should only be used right before an exit
void save_progress(void) {
    ti_var_t variable;
    ti_CloseAll();
    if ((variable = ti_Open(save_name, "w"))) {
        ti_PutC((char)num_packs, variable);
        ti_Write(&pack_info, sizeof(pack_info_t), num_packs, variable);
	ti_SetArchiveStatus(true, variable);
    }
    ti_CloseAll();
    gfx_End();
    prgm_CleanUp();
}

void load_progress(void) {
    ti_var_t variable;
    pack_info_t **pack_info_in_var = NULL;
    uint8_t num_packs_in_var = 0;
    unsigned int j;
    
    ti_CloseAll();
    if ((variable = ti_Open(save_name, "r"))) {
        num_packs_in_var = (uint8_t)ti_GetC(variable);
        pack_info_in_var = (pack_info_t**)ti_GetDataPtr(variable);
    }
    ti_CloseAll();
    
    memset(pack_info, 0, sizeof(pack_info));
    for(j=0; j<256; j++) {
        pack_info[j].lives = 15;
    }
    
    num_packs = 0;
    while((var_name = ti_Detect( &search_pos, search_string )) != NULL) {
        pack_info_t *pack = &pack_info[num_packs];
        for(j=0; j<num_packs_in_var; j++) {
            pack_info_t *pack_in_var = (pack_info_t*)((uint8_t*)((uint8_t*)(pack_info_in_var) + (j*sizeof(pack_info_t))));
            if (!strcmp(pack_in_var->name, var_name)) {
                memcpy(pack, pack_in_var, sizeof(pack_info_t));
                break;
            }
        }
        
        // if we didn't break from the loop
        if (j == num_packs_in_var) {
            memcpy(pack->name, var_name, 8);
        }
        num_packs++;
    }
}

static void init_level(gfx_tilemap_t *tilemap, map_t *map, gfx_image_t **img_tiles, uint8_t *data, uint8_t width, uint8_t height) {
    map->max_y = height * TILE_HEIGHT;
    map->max_x = width * TILE_WIDTH;
    map->max_x_scroll = map->max_x - ((TILEMAP_DRAW_WIDTH-1) * TILE_WIDTH);
    map->max_y_scroll = map->max_y - ((TILEMAP_DRAW_HEIGHT-1) * TILE_HEIGHT);
    tilemap->map = data;
    tilemap->tiles = img_tiles;
    tilemap->type_width = gfx_tile_16_pixel;
    tilemap->type_height = gfx_tile_16_pixel;
    tilemap->tile_height = TILE_HEIGHT;
    tilemap->tile_width = TILE_WIDTH;
    tilemap->draw_height = TILEMAP_DRAW_HEIGHT;
    tilemap->draw_width = TILEMAP_DRAW_WIDTH;
    tilemap->width = width;
    tilemap->height = height;
    tilemap->y_loc = 0;
    tilemap->x_loc = 0;
}

static void decode(uint8_t *in, uint8_t *out) {
    for(;;) {
        uint8_t c, i, cnt;
        c = *in;
        in++;
        if (c == 255) return;
        if (c > 128) {
            cnt = c - 128;
            for (i = 0; i < cnt; i++) {
                *out = *in;
                out++; in++;
            }
        } else {
            cnt = c;
            c = *in;
            in++;
            for (i = 0; i < cnt; i++) {
                *out = c;
                out++;
            }
        }
    }
}

void set_level(uint8_t abs_pack, uint8_t level) {
    ti_var_t slot = 0;
    uint8_t level_width = 0, level_height = 0;
    uint16_t color = 0;
    
    game.end_level = 0;
    search_pos = NULL;
    
    ti_CloseAll();
    while(((var_name = ti_Detect( &search_pos, search_string )) != NULL) && abs_pack) {
        abs_pack--;
    }
    
    if ((slot = ti_Open((char*)var_name, "r"))) {
        uint8_t *pack_data = ((uint8_t*)ti_GetDataPtr(slot)) + 2;
        uint8_t num_pipes;
        
        // find start of actual data
        pack_data += strlen((char*)pack_data) + 1;
        
        pack_author = (char*)pack_data;
        pack_data += strlen(pack_author) + 1;
        
        // get the number of levels in the pack
        game.end_level = *pack_data;
        pack_data++;
        if (level) {
            uint16_t *level_data = (uint16_t*)pack_data;
            pack_data += level_data[level-1];
        }
        pack_data += (game.end_level-1)*2;
        
        // extract color channel
        color = *((uint16_t*)pack_data);
        pack_data+=2;
        
        // get the number of pipes
        num_pipes = *pack_data;
        pack_data++;
        warp_pipe_info = (unsigned int*)pack_data;
        pipe_max_tests = num_pipes * 2;
        
        pack_data += num_pipes * 6;
        
        // get level width and height
        level_width = *pack_data;
        pack_data++;
        level_height = *pack_data;
        pack_data++;
        
        // allocate and decompress the level
        tilemap_data = safe_malloc(level_width * level_height);
        decode(pack_data, tilemap_data);
    }
    
    if (!level_width || !level_height) {
        save_progress();
        exit(0);
    }
    
    // init the tilemap structure
    init_level(&tilemap, &level_map, tileset_tiles, tilemap_data, level_width, level_height);
    gfx_palette[BACKGROUND_COLOR_INDEX] = color;
}

#define MAX_SHOW 7

void set_load_screen(void) {
    ti_var_t slot;
    int y;
    
    gfx_image_t *tile_208 = tileset_tiles[208];
    gfx_image_t *tile_145 = tileset_tiles[145];
    gfx_image_t *tile_194 = tileset_tiles[194];
    gfx_image_t *tile_195 = tileset_tiles[195];
    
    uint8_t *pack_data;
    uint8_t key;
    uint8_t selected_pack = 0;
    uint8_t selected_level = 0;
    uint8_t scroll_amt = 0;
    uint8_t end_level;
    uint8_t max_select_level[MAX_SHOW];
    uint8_t tmp;
    
    gfx_SetClipRegion(0, 0, LCD_WIDTH, LCD_HEIGHT);
    
    redraw_screen:
    
    gfx_SetTextBGColor(DARK_BLUE_INDEX);
    gfx_SetTextTransparentColor(DARK_BLUE_INDEX);
    
    y = 103;
    slot = 0;
    
    gfx_palette[BACKGROUND_COLOR_INDEX] = 0xD77E;
    gfx_FillScreen(BLACK_INDEX);
    gfx_ScaledTransparentSprite_NoClip(oiram_logo, 150, 32, 2, 2);
    
    gfx_TransparentSprite(tile_194, 24, 52);
    gfx_TransparentSprite(tile_195, 40, 52);
    
    gfx_TransparentSprite(tile_145, 40, 52+16);

    gfx_TransparentSprite(tile_194, 56, 52-16-16);
    gfx_TransparentSprite(tile_195, 72, 52-16-16);
    gfx_TransparentSprite(tile_145, 72, 52-16);
    gfx_TransparentSprite(tile_145, 72, 52);
    gfx_TransparentSprite(tile_145, 72, 52+16);
    gfx_TransparentSprite(tile_145, 72, 52+16+16);
    gfx_TransparentSprite(tile_208, 56, 52-16);
    gfx_TransparentSprite(tile_208, 56, 52);
    gfx_TransparentSprite(tile_208, 56, 52+16);
    
    gfx_TransparentSprite(tile_208, 24, 52+16);
    gfx_TransparentSprite(tile_208, 24, 52+16+16);
    
    gfx_TransparentSprite(tileset_tiles[222], 56, 52+16+16);
    gfx_TransparentSprite(tileset_tiles[223], 40, 52+16+16);
    
    gfx_SetTextFGColor(WHITE_INDEX);
    gfx_PrintStringXY("By Mateo", 150, 58);
    gfx_SetColor(DARK_BLUE_INDEX);
    gfx_FillRectangle(2,100,316,80);
    gfx_SetColor(WHITE_INDEX);
    gfx_Rectangle(2,100,316,((MAX_SHOW+1) * 10) + 1);
    
    gfx_TransparentSprite(mushroom, 5, selected_pack*10 + 103);
    
    gfx_PrintStringXY("Controls", 5, 187);
    
    if (game.alternate_keypad) {
        gfx_PrintStringXY("[2nd]", 9, 209);
        gfx_PrintStringXY("[up]", 9, 219);
        gfx_PrintStringXY("[alpha]", 9, 229);
    } else {
        gfx_PrintStringXY("[alpha]", 9, 209);
        gfx_PrintStringXY("[2nd]", 9, 219);
        gfx_PrintStringXY("[up]", 9, 229);
    }
    
    gfx_PrintStringXY("[del]", 9, 199);
    gfx_PrintStringXY("Quit", 65, 199);
    gfx_PrintStringXY("Run", 65, 209);
    gfx_PrintStringXY("Jump", 65, 219);
    gfx_PrintStringXY("Special, pickup shells", 65, 229);
    
    gfx_PrintStringXY("Press <> to select level", 150, 199);
    
    tmp = 0;
    num_packs = 0;
    search_pos = NULL;
    while((var_name = ti_Detect( &search_pos, search_string )) != NULL) {
        ti_Close(slot);
        
        if (scroll_amt <= num_packs && y < (103 + 10*MAX_SHOW)) {
            uint8_t max_select;
            uint8_t progress;
            
            slot = ti_Open((char*)var_name, "r");
            pack_data = ((uint8_t*)ti_GetDataPtr(slot)) + 2;
        
            gfx_PrintStringXY((char*)pack_data, 23, y + 4);
            
            pack_data += strlen((char*)pack_data) + 1;
            pack_data += strlen((char*)pack_data) + 1;
            end_level = *pack_data;
            max_select = progress = pack_info[num_packs].progress;
            
            if (progress != end_level) {
                max_select++;
            }
            
            gfx_SetTextXY(320 - 8*3 - 3, y + 4);
            gfx_SetMonospaceFont(8);
            gfx_PrintUInt(max_select, 3);
            gfx_SetMonospaceFont(0);
            
            max_select_level[num_packs] = max_select - 1;
            
            if (selected_pack == tmp) {
                selected_level = max_select - 1;
            }
            y += 10;
            tmp++;
        }
        
        num_packs++;
    }
    ti_CloseAll();
    
    gfx_BlitBuffer();
    
    // freeze bug
    asm("call 0004F4h");
    int_Disable();
    
    // debounce
    while (kb_ScanGroup(kb_group_1) & kb_Del);
    
    // reset the mode
    kb_SetMode(MODE_0_IDLE);
    kb_EnableInt = 0;
    
    while (true) {
        unsigned int delay;
        kb_key_t grp7;
        kb_key_t grp1;
        kb_key_t grp6;
        
        // scan the keypad
        kb_Scan();
        
        // debounce
        for (delay=0; delay<30000; delay++);
        
        grp7 = kb_Data[kb_group_7];
        grp6 = kb_Data[kb_group_6];
        grp1 = kb_Data[kb_group_1];
        
        if (grp1 == kb_Del) {
            save_progress();
            exit(0);
        }
        if (grp6 == kb_Enter || grp1 == kb_2nd) {
            break;
        }
        if (grp1 == kb_Mode) {
            game.alternate_keypad ^= true;
            goto redraw_screen;
        }
        if (grp7 == kb_Down || grp7 == kb_Up) {
            if (grp7 == kb_Down && ((selected_pack + scroll_amt + 1) < num_packs)) {
                if (selected_pack == MAX_SHOW-1) {
                    scroll_amt++;
                } else {
                    selected_pack++;
                }
            }
            if (grp7 == kb_Up && ((selected_pack + scroll_amt) != 0)) {
                if (selected_pack == 0) {
                    scroll_amt--;
                } else {
                    selected_pack--;
                }
            }
            goto redraw_screen;
        }
        if (grp7 == kb_Left || grp7 == kb_Right) {
            if (grp7 == kb_Left) {
                if (selected_level) {
                    selected_level--;
                }
            } else {
                if (selected_level != max_select_level[selected_pack]) {
                    selected_level++;
                }
            }
            gfx_SetTextBGColor(DARK_BLUE_INDEX);
            gfx_SetTextTransparentColor(BLACK_INDEX);
            gfx_SetTextXY(320 - 8*3 - 3, selected_pack*10 + 103 + 4);
            gfx_SetMonospaceFont(8);
            gfx_PrintUInt(selected_level + 1, 3);
            gfx_SetMonospaceFont(0);
            gfx_BlitBuffer();
        }
    }
    
    game.pack = selected_pack + scroll_amt;
    game.level = selected_level;
    
    gfx_SetClipRegion(0, 0, X_PXL_MAX, Y_PXL_MAX);
}
