#ifndef ENEMIES_H
#define ENEMIES_H

#include <stdint.h>

void get_enemies(void);

void remove_flame(uint8_t i);
void remove_thwomp(uint8_t i);
void remove_chomper(uint8_t i);
void remove_boo(uint8_t i);

#define MAX_CHOMPERS 64

typedef struct {
    int start_y;
    int x, y;
    int8_t vy;
    bool throws_fire;
    uint8_t count;
} chomper_t;

extern chomper_t *chomper[MAX_CHOMPERS];
extern uint8_t num_chompers;

#define MAX_FLAMES 64

typedef struct {
    int start_y;
    int x, y;
    int8_t vy;
    uint8_t count;
} flame_t;

extern flame_t *flame[MAX_FLAMES];
extern uint8_t num_flames;

#define MAX_THWOMPS 64

typedef struct {
    int start_y;
    int x, y;
    int8_t vy;
    uint8_t count;
} thwomp_t;

extern thwomp_t *thwomp[MAX_THWOMPS];
extern uint8_t num_thwomps;

#define MAX_BOOS 64

typedef struct {
    int x, y;
    int8_t vy;
    bool dir;
    uint8_t count;
} boo_t;

extern boo_t *boo[MAX_BOOS];
extern uint8_t num_boos;

#define MAX_SIMPLE_ENEMY 64

typedef struct {
    int x, y;
    int8_t vy;
    int8_t vx;
    uint8_t type;
    uint8_t counter;
} enemy_t;

enum simple_enemy_type {
    NO_SIMPLE_ENEMY_TYPE=0,
    BULLET_CREATOR_TYPE,
    CANNONBALL_DOWN_CREATOR_TYPE,
    CANNONBALL_UP_CREATOR_TYPE,
    BULLET_TYPE,
    CANNONBALL_TYPE,
    FISH_TYPE
};

extern enemy_t *simple_enemy[MAX_SIMPLE_ENEMY];
extern uint8_t num_simple_enemies;

enemy_t *add_simple_enemy(uint8_t *tile, uint8_t type);
void remove_simple_enemy(uint8_t i);

#endif