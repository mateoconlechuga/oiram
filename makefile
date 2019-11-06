# ----------------------------
# Program Options
# ----------------------------

NAME         ?= OIRAM
ICON         ?= iconc.png
DESCRIPTION  ?= "Oiram"
COMPRESSED   ?= NO
ARCHIVED     ?= NO

# ----------------------------
# Compile Options
# ----------------------------

OPT_MODE     ?= -Oz
EXTRA_CFLAGS ?= -Wall -Wextra

# ----------------------------
# Debug Options
# ----------------------------

OUTPUT_MAP   ?= NO

include $(CEDEV)/include/.makefile
