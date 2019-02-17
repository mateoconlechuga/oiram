ASM ?= fasmg

ifeq ($(OS),Windows_NT)
MV = move
MKDIR = mkdir
else
MV = mv
MKDIR = mkdir -p
endif

all: cv sp
	$(MV) OiramS.8xv ../bin/OiramS.8xv
	$(MV) OiramT.8xv ../bin/OiramT.8xv

sp:
	export INCLUDE="$(CEDEV)/include/fasmg-ez80" && $(ASM) oiram_tiles.ez80 OiramT.8xv
	export INCLUDE="$(CEDEV)/include/fasmg-ez80" && $(ASM) oiram_sprites.ez80 OiramS.8xv

cv:
	convpng
	$(MKDIR) ../bin

clean:
	$(RM) *.asm *.inc main_pal.png convpng.log

.PHONY: cv sp all

