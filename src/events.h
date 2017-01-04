#ifndef EVENTS_H
#define EVENTS_H

// standard headers
#include <stdbool.h>
#include <stdint.h>
#include <tice.h>

#include "defines.h"
#include "simple_mover.h"

extern bool handling_events;

void handle_pending_events(void);
bool in_viewport(int x, int y);

#endif