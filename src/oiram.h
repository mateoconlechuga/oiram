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

void compute_oiram_start_location(void);
void set_normal_oiram_sprites(void);

#endif

