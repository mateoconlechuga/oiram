#ifndef oiram_H
#define oiram_H

#include "defines.h"

void move_oiram(void);

extern bool pressed_left;
extern bool pressed_right;
extern bool pressed_up;
extern bool pressed_down;
extern bool pressed_alpha;
extern bool pressed_2nd;

extern bool allow_up_press;

void oiram_start_location(void);
void set_normal_oiram_sprites(void);

typedef struct {
    uint8_t index;
    int x;
    int y;
    int vy;
    uint8_t vx;
    unsigned int scrollx;
    unsigned int scrolly;
    int rel_x;
    int rel_y;
    gfx_sprite_t *sprite;
    hitbox_t hitbox;
    uint8_t hitbox_height_half;
    bool direction;
    uint8_t flags;
    bool has_shell;
    bool has_red_shell;
    bool crouched;
    uint8_t fireballs;
    int8_t momentum;
    uint8_t invincible;
    bool on_vine;
    uint8_t lives;
    bool is_flying;
    uint8_t fly_count;
    uint8_t spin_count;
    uint8_t shrink_count;
    unsigned int door_x;
    uint8_t door_y;
    uint8_t score_chain;
    bool started_fail;
    bool failed;
    int fail_x;
    int fail_y;
} oiram_t;

extern oiram_t oiram;

#endif
