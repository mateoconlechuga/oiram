#----------------------------
L := graphx fileioc keypadc
#----------------------------

#----------------------------
TARGET ?= OIRAM
DEBUGMODE ?= NDEBUG
ARCHIVED ?= NO
COMPRESSED ?= YES
#----------------------------
ICONPNG ?= iconc.png
DESCRIPTION ?= "Oiram v1.2"
#----------------------------

#----------------------------
SRCDIR := src
OBJDIR := obj
BINDIR := bin
GFXDIR := src/gfx

BSSHEAP_LOW := D031F6
BSSHEAP_HIGH := D13FD6
STACK_HIGH := D1A87E
INIT_LOC := D1A87F
#----------------------------

include $(CEDEV)/include/.makefile
