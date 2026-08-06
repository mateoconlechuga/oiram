struct oiram_ds3_usb;
#define usb_callback_data_t struct oiram_ds3_usb
struct oiram_ds3_transfer;
#define usb_transfer_data_t struct oiram_ds3_transfer
#include <usbdrvce.h>
#include <tice.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <fileioc.h>
#include "ds3_common.h"
#define JOY_NAME "DS3JOY"
static uint8_t joy_mode=2,joy_engage_x=48,joy_release_x=24,joy_engage_y=48;
static bool joy_left,joy_right,joy_down;
static void joy_load(void){uint8_t h,b[8];h=ti_Open(JOY_NAME,"r");if(!h)return;if(ti_Read(b,1,8,h)==8&&!memcmp(b,"J3CF",4)&&b[4]<=2){joy_mode=b[4];joy_engage_x=b[5];joy_release_x=b[6];joy_engage_y=b[7];}ti_Close(h);}
#include "oiram_ds3.h"

#define DS3_VID 0x054C
#define DS3_PID 0x0268
#define DS3_CROSS (1UL << 14)
#define DS3_SQUARE (1UL << 15)
#define DS3_TRIANGLE (1UL << 12)
#define DS3_START (1UL << 3)
#define DS3_UP (1UL << 4)
#define DS3_RIGHT (1UL << 5)
#define DS3_DOWN (1UL << 6)
#define DS3_LEFT (1UL << 7)

typedef struct oiram_ds3_usb {
    usb_device_t device;
    bool enabled;
    bool disconnected;
} oiram_ds3_usb_t;

typedef struct oiram_ds3_transfer {
    volatile bool active;
    volatile bool ready;
    volatile usb_transfer_status_t status;
    volatile size_t transferred;
} oiram_ds3_transfer_t;

typedef struct {
    bool hid;
    uint8_t interface_number;
    uint8_t endpoint_in;
    uint16_t packet_size;
} oiram_ds3_layout_t;

static oiram_ds3_usb_t g_usb;
static oiram_ds3_transfer_t g_transfer;
static oiram_ds3_state_t g_state;
static usb_endpoint_t g_endpoint;
static uint8_t g_report[64];
static uint16_t g_packet_size;
static uint32_t g_previous_buttons;
static ds3_profile_t g_profile;
static bool g_usb_initialized;
/* CLEANUP FIX (DS3MERG2, see DS3MERG2_CLEANUP_FIX.md): tracks whether
 * usb_Init() was ever CALLED, independent of whether it SUCCEEDED.
 * g_usb_initialized (above, unchanged in meaning) still means "fully
 * initialized"; oiram_ds3_update()'s guard keeps using that one. This new
 * flag is what oiram_ds3_cleanup() gates on, so usb_Cleanup() runs even
 * when usb_Init() itself returned an error -- per usbdrvce.h's own
 * documented contract ("must be called... even if usb_Init returns an
 * error"), which the original never satisfied for that one path. */
static bool g_usb_init_attempted;

static void oiram_ds3_drop_input(void) {
    g_previous_buttons = 0;
    joy_left = joy_right = joy_down = false;
    memset(&g_state, 0, sizeof(g_state));
}

static usb_error_t oiram_ds3_event(usb_event_t event, void *event_data,
                                   oiram_ds3_usb_t *state) {
    switch (event) {
        case USB_DEVICE_CONNECTED_EVENT:
            state->device = (usb_device_t)event_data;
            state->enabled = false;
            state->disconnected = false;
            return usb_ResetDevice(state->device);
        case USB_DEVICE_ENABLED_EVENT:
            state->device = (usb_device_t)event_data;
            state->enabled = true;
            state->disconnected = false;
            break;
        case USB_DEVICE_DISCONNECTED_EVENT:
            state->device = NULL;
            state->enabled = false;
            state->disconnected = true;
            g_transfer.active = false;
            oiram_ds3_drop_input();
            break;
        case USB_DEVICE_DISABLED_EVENT:
            state->enabled = false;
            g_endpoint = NULL;
            g_packet_size = 0;
            g_transfer.active = false;
            g_transfer.ready = false;
            oiram_ds3_drop_input();
            break;
        default:
            break;
    }
    return USB_SUCCESS;
}

static usb_error_t oiram_ds3_transfer_done(usb_endpoint_t endpoint,
                                           usb_transfer_status_t status,
                                           size_t transferred,
                                           oiram_ds3_transfer_t *state) {
    (void)endpoint;
    state->status = status;
    state->transferred = transferred;
    state->active = false;
    state->ready = true;
    return USB_SUCCESS;
}

static bool oiram_ds3_parse_layout(const uint8_t *data, size_t length,
                                   oiram_ds3_layout_t *layout) {
    size_t offset = 0;
    uint8_t current_interface = 0xFF;
    memset(layout, 0, sizeof(*layout));
    while (offset + 2 <= length) {
        uint8_t descriptor_length = data[offset];
        uint8_t descriptor_type = data[offset + 1];
        if (descriptor_length < 2 || offset + descriptor_length > length) return false;
        if (descriptor_type == USB_INTERFACE_DESCRIPTOR && descriptor_length >= 9) {
            current_interface = data[offset + 2];
            if (!layout->hid && data[offset + 5] == USB_HID_CLASS) {
                layout->hid = true;
                layout->interface_number = current_interface;
            }
        } else if (descriptor_type == USB_ENDPOINT_DESCRIPTOR &&
                   descriptor_length >= 7 && layout->hid &&
                   current_interface == layout->interface_number) {
            uint8_t address = data[offset + 2];
            uint8_t transfer_type = data[offset + 3] & 3;
            if ((address & 0x80) && transfer_type == USB_INTERRUPT_TRANSFER) {
                layout->endpoint_in = address;
                layout->packet_size = (uint16_t)data[offset + 4] |
                                      ((uint16_t)data[offset + 5] << 8);
            }
        }
        offset += descriptor_length;
    }
    return layout->hid && layout->endpoint_in && layout->packet_size;
}

bool oiram_ds3_init_bounded(void) {
    usb_error_t error;
    usb_device_descriptor_t descriptor;
    size_t transferred = 0;
    size_t config_length;
    static uint8_t config[256];
    oiram_ds3_layout_t layout;
    usb_control_setup_t enable;
    uint8_t enable_data[4] = {0x42, 0x0C, 0x00, 0x00};
    uint32_t start;

    memset(&g_usb, 0, sizeof(g_usb));
    memset(&g_transfer, 0, sizeof(g_transfer));
    memset(&g_state, 0, sizeof(g_state));
    g_previous_buttons = 0;
    if (!ds3_profile_load(&g_profile)) ds3_profile_defaults(&g_profile);
    joy_load();
    g_usb_initialized = false;
    g_usb_init_attempted = false;

    g_usb_init_attempted = true;
    error = usb_Init(oiram_ds3_event, &g_usb, NULL, USB_DEFAULT_INIT_FLAGS);
    if (error != USB_SUCCESS) {
        /* CLEANUP FIX: previously returned here with no cleanup call at
         * all. oiram_ds3_cleanup() now sees g_usb_init_attempted=true and
         * calls usb_Cleanup(), matching every other failure path in this
         * function (which already called oiram_ds3_cleanup() themselves). */
        oiram_ds3_cleanup();
        return false;
    }
    g_usb_initialized = true;

    start = usb_GetCycleCounter();
    while (!g_usb.enabled) {
        usb_HandleEvents();
        if ((uint32_t)(usb_GetCycleCounter() - start) > (uint32_t)usb_MsToCycles(1200)) {
            oiram_ds3_cleanup();
            return false;
        }
    }

    error = usb_GetDeviceDescriptor(g_usb.device, &descriptor,
                                    sizeof(descriptor), &transferred);
    if (error != USB_SUCCESS || transferred < sizeof(descriptor) ||
        descriptor.idVendor != DS3_VID || descriptor.idProduct != DS3_PID) {
        oiram_ds3_cleanup();
        return false;
    }

    config_length = usb_GetConfigurationDescriptorTotalLength(g_usb.device, 0);
    if (config_length < 9 || config_length > sizeof(config)) {
        oiram_ds3_cleanup();
        return false;
    }
    transferred = 0;
    error = usb_GetConfigurationDescriptor(g_usb.device, 0,
        (usb_configuration_descriptor_t *)config, config_length, &transferred);
    if (error != USB_SUCCESS || transferred != config_length ||
        !oiram_ds3_parse_layout(config, config_length, &layout)) {
        oiram_ds3_cleanup();
        return false;
    }

    error = usb_SetConfiguration(g_usb.device,
        (const usb_configuration_descriptor_t *)config, config_length);
    if (error != USB_SUCCESS) {
        oiram_ds3_cleanup();
        return false;
    }

    enable.bmRequestType = USB_HOST_TO_DEVICE | USB_CLASS_REQUEST |
                           USB_RECIPIENT_INTERFACE;
    enable.bRequest = 0x09;
    enable.wValue = 0x03F4;
    enable.wIndex = layout.interface_number;
    enable.wLength = sizeof(enable_data);
    transferred = 0;
    error = usb_ControlTransfer(usb_GetDeviceEndpoint(g_usb.device, 0),
                                &enable, enable_data, 3, &transferred);
    if (error != USB_SUCCESS) {
        oiram_ds3_cleanup();
        return false;
    }

    if (layout.packet_size > sizeof(g_report)) {
        oiram_ds3_cleanup();
        return false;
    }
    g_endpoint = usb_GetDeviceEndpoint(g_usb.device, layout.endpoint_in);
    if (!g_endpoint) {
        oiram_ds3_cleanup();
        return false;
    }
    g_packet_size = layout.packet_size;
    if (!g_usb.device || g_usb.disconnected || !g_usb.enabled || !g_endpoint || !g_packet_size) {
        g_transfer.active = false;
        g_transfer.ready = false;
        oiram_ds3_drop_input();
        return false;
    }
    memset(g_report, 0, sizeof(g_report));
    g_transfer.active = true;
    error = usb_ScheduleInterruptTransfer(g_endpoint, g_report, g_packet_size,
                                           oiram_ds3_transfer_done, &g_transfer);
    if (error != USB_SUCCESS) {
        g_transfer.active = false;
        oiram_ds3_cleanup();
        return false;
    }
    g_state.active = true;
    return true;
}

void oiram_ds3_update(void) {
    usb_error_t error;
    uint32_t buttons;
    if (!g_usb_initialized || !g_state.active || !g_usb.device) return;
    g_state.fresh = false;
    error = usb_HandleEvents();
    if (!g_usb.device || g_usb.disconnected || !g_usb.enabled || !g_endpoint || !g_packet_size) {
        g_transfer.active = false;
        g_transfer.ready = false;
        oiram_ds3_drop_input();
        return;
    }
    if (error != USB_SUCCESS) {
        g_state.errors++;
        if (g_state.errors >= 4) {
            g_endpoint = NULL;
            g_packet_size = 0;
            g_transfer.active = false;
            g_transfer.ready = false;
            oiram_ds3_drop_input();
        }
        return;
    }
    if (!g_transfer.ready) return;
    g_transfer.ready = false;
    if (g_transfer.status != USB_TRANSFER_COMPLETED ||
        g_transfer.transferred < 10 || g_report[0] != 1) {
        g_state.errors++;
    } else {
        buttons = (uint32_t)g_report[2] |
                  ((uint32_t)g_report[3] << 8) |
                  ((uint32_t)g_report[4] << 16);
        g_state.buttons = buttons;
        g_state.pressed = buttons & ~g_previous_buttons;
        g_state.released = g_previous_buttons & ~buttons;
        g_state.pause_pressed = (buttons & DS3_START) && !(g_previous_buttons & DS3_START);
        g_state.lx = ds3_apply_axis(g_report[6],g_profile.lc_x,g_profile.lx_min,g_profile.lx_max,g_profile.dead_l,g_profile.invert_flags&1);
        g_state.ly = ds3_apply_axis(g_report[7],g_profile.lc_y,g_profile.ly_min,g_profile.ly_max,g_profile.dead_l,g_profile.invert_flags&2);
        if(joy_mode){if(!joy_left&&g_state.lx<=128-joy_engage_x)joy_left=true;else if(joy_left&&g_state.lx>=128-joy_release_x)joy_left=false;if(!joy_right&&g_state.lx>=128+joy_engage_x)joy_right=true;else if(joy_right&&g_state.lx<=128+joy_release_x)joy_right=false;if(!joy_down&&g_state.ly>=128+joy_engage_y)joy_down=true;else if(joy_down&&g_state.ly<=128+joy_release_x)joy_down=false;}else joy_left=joy_right=joy_down=false;
        g_state.left = oiram_ds3_action_down(ACT_LEFT) || (joy_mode!=0 && joy_left);
        g_state.right = oiram_ds3_action_down(ACT_RIGHT) || (joy_mode!=0 && joy_right);
        g_state.down = oiram_ds3_action_down(ACT_DOWN) || (joy_mode!=0 && joy_down);
        g_state.jump = oiram_ds3_action_down(ACT_CONFIRM);
        g_state.run = oiram_ds3_action_down(ACT_PRIMARY) || ((buttons & DS3_L3) != 0);
        g_state.special = ((buttons & DS3_TRIANGLE) != 0) ||
                          ((buttons & DS3_UP) != 0);
        g_state.start_pressed = ((buttons & DS3_START) != 0) &&
                                ((g_previous_buttons & DS3_START) == 0);
        g_state.lx = g_report[6];
        g_state.ly = g_report[7];
        g_state.rx = g_report[8];
        g_state.ry = g_report[9];
        g_state.reports++;
        g_state.fresh = true;
        g_previous_buttons = buttons;
    }
    if (!g_usb.device || g_usb.disconnected || !g_usb.enabled || !g_endpoint || !g_packet_size) {
        g_transfer.active = false;
        g_transfer.ready = false;
        oiram_ds3_drop_input();
        return;
    }
    memset(g_report, 0, sizeof(g_report));
    g_transfer.active = true;
    error = usb_ScheduleInterruptTransfer(g_endpoint, g_report, g_packet_size,
                                           oiram_ds3_transfer_done, &g_transfer);
    if (error != USB_SUCCESS) {
        g_transfer.active = false;
        g_state.errors++;
        oiram_ds3_drop_input();
    }
}

void oiram_ds3_cleanup(void) {
    /* CLEANUP FIX: gate on "was usb_Init ever attempted", not "did it fully
     * succeed" -- this is the one-line change that makes cleanup run for
     * the usb_Init-itself-failed path too. Resetting g_usb_init_attempted
     * to false immediately after means a second call to this function
     * (e.g. if a caller called it twice) is a safe no-op, same as the
     * original's own double-call safety via g_usb_initialized. */
    if (g_usb_init_attempted) usb_Cleanup();
    g_usb_init_attempted = false;
    g_usb_initialized = false;
    g_endpoint = NULL;
    g_packet_size = 0;
    memset(&g_usb, 0, sizeof(g_usb));
    memset(&g_transfer, 0, sizeof(g_transfer));
    memset(&g_state, 0, sizeof(g_state));
}

const oiram_ds3_state_t *oiram_ds3_state(void) {
    return &g_state;
}
bool oiram_ds3_action_down(uint8_t action) {
    if (action == 0 || action >= ACT_COUNT) return false;
    return (g_state.buttons & g_profile.action_buttons[action]) != 0;
}
bool oiram_ds3_pause_pressed(void){return g_state.active && g_state.pause_pressed;}
