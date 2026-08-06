#ifndef DS3_COMMON_H
#define DS3_COMMON_H
#include <stdint.h>
#include <stdbool.h>

typedef enum {
 DS3_OK=0, DS3_USB_INIT_ERROR, DS3_WAIT_ERROR, DS3_WRONG_DEVICE,
 DS3_DESCRIPTOR_ERROR, DS3_CONFIG_ERROR, DS3_ENABLE_ERROR,
 DS3_ENDPOINT_ERROR, DS3_TRANSFER_ERROR, DS3_DISCONNECTED
} ds3_result_t;

enum {
 DS3_SELECT=1u<<0, DS3_L3=1u<<1, DS3_R3=1u<<2, DS3_START=1u<<3,
 DS3_UP=1u<<4, DS3_RIGHT=1u<<5, DS3_DOWN=1u<<6, DS3_LEFT=1u<<7,
 DS3_L2=1u<<8, DS3_R2=1u<<9, DS3_L1=1u<<10, DS3_R1=1u<<11,
 DS3_TRIANGLE=1u<<12, DS3_CIRCLE=1u<<13, DS3_CROSS=1u<<14,
 DS3_SQUARE=1u<<15, DS3_PS=1u<<16
};

typedef enum {
 ACT_NONE=0, ACT_CONFIRM, ACT_CANCEL, ACT_MENU, ACT_PAUSE,
 ACT_UP, ACT_RIGHT, ACT_DOWN, ACT_LEFT, ACT_PAGE_LEFT, ACT_PAGE_RIGHT,
 ACT_PRIMARY, ACT_SECONDARY, ACT_COUNT
} ds3_action_t;

typedef struct {
 uint32_t buttons, pressed, released;
 uint8_t raw_lx,raw_ly,raw_rx,raw_ry;
 uint8_t lx,ly,rx,ry;
 bool connected, fresh;
} ds3_state_t;

typedef struct {
 uint8_t version;
 uint16_t vid,pid;
 uint8_t lc_x,lc_y,rc_x,rc_y;
 uint8_t lx_min,lx_max,ly_min,ly_max;
 uint8_t rx_min,rx_max,ry_min,ry_max;
 uint8_t dead_l,dead_r;
 uint8_t invert_flags;
 uint8_t led_mask;
 uint32_t action_buttons[ACT_COUNT];
} ds3_profile_t;

void ds3_profile_defaults(ds3_profile_t *p);
bool ds3_profile_load(ds3_profile_t *p);
bool ds3_profile_save(const ds3_profile_t *p);
uint8_t ds3_apply_axis(uint8_t raw,uint8_t center,uint8_t minv,uint8_t maxv,uint8_t dead,bool invert);

ds3_result_t ds3_init(void);
void ds3_cleanup(void);
ds3_result_t ds3_update(void);
const ds3_state_t *ds3_state(void);
bool ds3_action_down(const ds3_profile_t *p,ds3_action_t a);
bool ds3_action_pressed(const ds3_profile_t *p,ds3_action_t a);
#endif
