#ifndef EVENTS_H
#define EVENTS_H

#include <stdbool.h>

extern bool handling_events;

void handle_pending_events(void);
bool in_viewport(int x, int y);

#endif
