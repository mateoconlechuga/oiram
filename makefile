# ----------------------------
# Makefile Options
# ----------------------------

NAME = OIRAM
ICON = iconc.png
DESCRIPTION = "Oiram"
COMPRESSED = YES
ARCHIVED = NO

CFLAGS ?= -Wall -Wextra -Oz
CXXFLAGS ?= -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)
