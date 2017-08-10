#ifndef SIMPLE_MOVER_H
#define SIMPLE_MOVER_H

#include <stdint.h>

#include "defines.h"

// -----------------------------
// Simple movers
// -----------------------------

#define MAX_SIMPLE_MOVERS 252

typedef struct simple_move {
    int x, y;      // x and y absolute posistions on screen
    int8_t vy;
    int8_t vx;
    uint8_t type;
    hitbox_t hitbox;
    bool bumped;
    bool smart;
    bool is_bouncer;
    bool is_flyer;
    int8_t fly_counter;
    int8_t counter;
    gfx_sprite_t *sprite;
    uint8_t score_counter;
} simple_move_t;

enum simple_move_type {
    NO_TYPE=0,
    MUSHROOM_TYPE,
    MUSHROOM_1UP_TYPE,
    FIRE_FLOWER_TYPE,
    FLAT_GOOMBA_TYPE,
    FIREBALL_TYPE,
    KOOPA_BONES_TYPE,
    KOOPA_BONES_DEAD_TYPE,
    STAR_TYPE,
    RESWOB_TYPE,
    HITABLE_TYPES,      // divider
    GOOMBA_TYPE,
    KOOPA_RED_TYPE,
    KOOPA_GREEN_TYPE,
    KOOPA_RED_FLY_TYPE,
    KOOPA_GREEN_FLY_TYPE,
    SPIKE_TYPE,
    SHELL_TYPES,       // divider
    KOOPA_RED_SHELL_TYPE,
    KOOPA_GREEN_SHELL_TYPE,
    SPIKE_SHELL_TYPE
};

extern uint8_t simple_mover_type;
extern uint8_t num_simple_movers;
extern simple_move_t *simple_mover[MAX_SIMPLE_MOVERS];

simple_move_t *add_simple_mover(uint8_t *spawing_tile);
void remove_simple_mover(uint8_t index);
void simple_move_handler(simple_move_t *this);

#endif

