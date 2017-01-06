#ifndef LOWER_H
#define LOWER_H

extern const unsigned int oiram_score_chain[9];
extern gfx_image_t *oiram_score_chain_sprites[9];

void draw_coins(void);
void draw_lives(void);
void draw_level(void);
void draw_score(void);
void draw_time(void);

void add_score(uint8_t add, int x, int y);
void add_score_no_sprite(uint8_t add);
void add_next_chain_score(int x, int y);
void add_coin(int x, int y);

#endif