#ifndef EVENTS_H
#define EVENTS_H

// standard headers
#include <stdbool.h>
#include <stdint.h>
#include <tice.h>
#include "defines.h"
#include "simple_mover.h"

#define MAX_POOFS     11
#define MAX_FIREBALLS 20

typedef struct {
    int x, y;
    uint8_t count;
    bool second;
} poof_t;

extern poof_t *poof[MAX_POOFS];
void add_rel_poof(int x, int y);
void add_poof(int x, int y);
void remove_poof(uint8_t i);

// ---------------------------------------------------
// a fireball is a simple mover with some extra things
// ---------------------------------------------------

typedef struct {
    bool type;
    uint8_t count;
    simple_move_t *mover;
} fireball_t;

extern fireball_t *fireball[MAX_FIREBALLS];

enum fireball_directions {
    UP_LEFT=0,
    UP_RIGHT,
    DOWN_LEFT,
    DOWN_RIGHT
};

enum fireball_type {
    MARIO_FIREBALL=1,
    CHOMPER_FIREBALL,
};

void remove_fireball(uint8_t i);
fireball_t *add_fireball(int x, int y, uint8_t dir);

extern bool handling_events;

void handle_pending_events(void);
bool in_viewport(int x, int y);

#endif