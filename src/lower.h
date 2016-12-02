#ifndef LOWER_H
#define LOWER_H

extern uint8_t seconds;

void draw_coins(void);
void draw_lives(void);
void draw_world(void);
void draw_score(void);
void draw_time(void);

void add_score(unsigned int add);
void add_next_chain_score(void);
void add_coin(void);

#endif