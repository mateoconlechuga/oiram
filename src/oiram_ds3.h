#ifndef OIRAM_DS3_H
#define OIRAM_DS3_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool active;
    bool fresh;
    bool left;
    bool right;
    bool down;
    bool jump;
    bool run;
    bool special;
    bool start_pressed;
    uint8_t lx;
    uint8_t ly;
    uint8_t rx;
    uint8_t ry;
    uint32_t buttons;
    uint32_t pressed;
    uint32_t released;
    bool pause_pressed;
    uint32_t reports;
    uint16_t errors;
} oiram_ds3_state_t;

bool oiram_ds3_init_bounded(void);
void oiram_ds3_update(void);
void oiram_ds3_cleanup(void);
const oiram_ds3_state_t *oiram_ds3_state(void);
bool oiram_ds3_action_down(uint8_t action);
bool oiram_ds3_pause_pressed(void);

#endif