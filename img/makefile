SPASM ?= spasm

ifeq ($(OS),Windows_NT)
MV = move
MKDIR = mkdir
else
MV = mv
MKDIR = mkdir -p
endif

all: cv sp
	$(MV) OiramS.8xv ../extra/OiramS.8xv
	$(MV) OiramT.8xv ../extra/OiramT.8xv

sp:
	$(SPASM) -E -Z oiram_tiles.ez80 OiramT.8xv
	$(SPASM) -E -Z oiram_sprites.ez80 OiramS.8xv

cv:
	convpng
	$(MKDIR) ../extra

clean:
	$(RM) *.asm *.inc main_pal.png convpng.log

.PHONY: cv sp all

