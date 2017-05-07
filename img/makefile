SPASM ?= spasm

ifeq ($(OS),Windows_NT)
MV = move
else
MV = mv
endif

all: cv sp
	$(MV) OiramS.8xv ../bin/OiramS.8xv
	$(MV) OiramT.8xv ../bin/OiramT.8xv

sp:
	$(SPASM) -E oiram_tiles.ez80 OiramT.8xv
	$(SPASM) -E oiram_sprites.ez80 OiramS.8xv

cv:
	convpng
	mkdir ../bin

clean:
	$(RM) *.asm *.inc main_pal.png convpng.log

.PHONY: cv sp all

