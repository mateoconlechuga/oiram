	.assume adl=1
	
; tile_handlers.h
	.def _animate
	.def _moveable_tile
	.def _moveable_tile_left_bottom
	.def _moveable_tile_right_bottom
	.def _solid_tile_handler
	.def _empty_tile_handler
	.def _tile_to_abs_xy_pos

_moveable_tile_right_bottom:
	xor	a,a                      ; TEST_RIGHT
	jr	_test_against
_moveable_tile_left_bottom:
	ld	a,1                      ; TEST_LEFT
	jr	_test_against
_moveable_tile:
	ld	a,2                      ; TEST_NONE
_test_against:
	ld	(_testing_side),a
	pop	bc
	pop	hl
	pop	de
	push	de
	push	hl
	push	bc
	xor	a,a
	bit	7,h
	ret	nz                       ; if (x < 0) { return false; }

	ld	bc,(_level_map+14)
	sbc	hl,bc
	ret	nc                       ; if (x > width) { return false; }
	add	hl,bc
	
	ex	de,hl
	ld	a,1
	bit	7,h
	ret	nz                       ; if (y < 0) { return true; }
	
	ld	bc,(_level_map+11)
	or	a,a
	sbc	hl,bc
	ld	a,1
	ret	nc
	add	hl,bc
	
	ld	iy,_tilemap
	ld	(_test_x),de
	ld	(_test_y),hl
	ld	b,(iy+11)
divloop1:
	srl	h
	rr	l
	djnz	divloop1
	ld	h,(iy+13)
	mlt	hl
	ex	de,hl
	ld	b,(iy+11)
divloop2:
	srl	h
	rr	l
	djnz	divloop2
	add	hl,de
	ld	de,(iy+0)
	add	hl,de
	push	hl
	ld	l,(hl)
	ld	h,3
	mlt	hl
	ld	de,_tile_handler
	add	hl,de
	ld	iy,(hl)
	call	__indcall
	pop	de
	ret

_solid_tile_handler:
	xor	a,a
	ret

_empty_tile_handler:
	ld	a,1
	ret

_tile_to_abs_xy_pos:
	call	__frameset0
	ld	a,(_tilemap+13)
	or	a,a
	sbc	hl,hl
	ld	l,a
	push	hl
	pop	bc
	ld	de,(_tilemap+0)
	ld	hl,(ix+6)
	or	a,a
	sbc	hl,de
	call	__idvrmu
	ex	de,hl
	add	hl,hl
	add	hl,hl
	add	hl,hl
	add	hl,hl
	ld	iy,(ix+12)
	ld	(iy),hl
	ex	de,hl
	add	hl,hl
	add	hl,hl
	add	hl,hl
	add	hl,hl
	ld	iy,(ix+9)
	ld	(iy),hl
	pop	ix
	ret

_animate:
	ld	a,(_tiles+0)
	inc	a
	ld	(_tiles+0),a
	cp	a,4
	ret	nz
	ld	bc,(_goomba_0)
	ld	hl,(_goomba_sprite)
	or	a,a
	sbc	hl,bc
	jr	nz,a_0
	ld	bc,(_goomba_1)
a_0:	ld	(_goomba_sprite),bc
	ld	bc,(_koopa_red_left_0)
	ld	hl,(_koopa_red_left_sprite)
	or	a,a
	sbc	hl,bc
	jr	nz,a_1
	ld	bc,(_koopa_red_left_1)
a_1:	ld	(_koopa_red_left_sprite),bc
	ld	bc,(_koopa_red_right_0)
	ld	hl,(_koopa_red_right_sprite)
	or	a,a
	sbc	hl,bc
	jr	nz,a_2
	ld	bc,(_koopa_red_right_1)
a_2:	ld	(_koopa_red_right_sprite),bc
	ld	bc,(_koopa_green_left_0)
	ld	hl,(_koopa_green_left_sprite)
	or	a,a
	sbc	hl,bc
	jr	nz,a_3
	ld	bc,(_koopa_green_left_1)
a_3:	ld	(_koopa_green_left_sprite),bc
	ld	bc,(_koopa_green_right_0)
	ld	hl,(_koopa_green_right_sprite)
	or	a,a
	sbc	hl,bc
	jr	nz,a_4
	ld	bc,(_koopa_green_right_1)
a_4:	ld	(_koopa_green_right_sprite),bc
	ld	bc,(_koopa_bones_left_0)
	ld	hl,(_koopa_bones_left_sprite)
	or	a,a
	sbc	hl,bc
	jr	nz,a_5
	ld	bc,(_koopa_bones_left_1)
a_5:	ld	(_koopa_bones_left_sprite),bc
	ld	bc,(_koopa_bones_right_0)
	ld	hl,(_koopa_bones_right_sprite)
	or	a,a
	sbc	hl,bc
	jr	nz,a_6
	ld	bc,(_koopa_bones_right_1)
a_6:	ld	(_koopa_bones_right_sprite),bc
	ld	bc,(_fish_right_0)
	ld	hl,(_fish_right_sprite)
	or	a,a
	sbc	hl,bc
	jr	nz,a_7
	ld	bc,(_fish_right_1)
a_7:	ld	(_fish_right_sprite),bc
	ld	bc,(_fish_left_0)
	ld	hl,(_fish_left_sprite)
	or	a,a
	sbc	hl,bc
	jr	nz,a_8
	ld	bc,(_fish_left_1)
a_8:	ld	(_fish_left_sprite),bc
	ld	bc,(_chomper_0)
	ld	hl,(_chomper_sprite)
	or	a,a
	sbc	hl,bc
	jr	nz,a_9
	ld	bc,(_chomper_1)
a_9:	ld	(_chomper_sprite),bc
	ld	bc,(_fire_0)
	ld	hl,(_fireball_sprite)
	or	a,a
	sbc	hl,bc
	jr	nz,a_10
	ld	bc,(_fire_1)
a_10:	ld	(_fireball_sprite),bc
	ld	bc,(_flame_fire_up_0)
	ld	hl,(_flame_sprite_up)
	or	a,a
	sbc	hl,bc
	jr	nz,a_11
	ld	bc,(_flame_fire_up_1)
a_11:	ld	(_flame_sprite_up),bc
	ld	bc,(_flame_fire_down_0)
	ld	hl,(_flame_sprite_down)
	or	a,a
	sbc	hl,bc
	jr	nz,a_12
	ld	bc,(_flame_fire_down_1)
a_12:	ld	(_flame_sprite_down),bc
	ld	bc,(_wing_right_0)
	ld	hl,(_wing_right_sprite)
	or	a,a
	sbc	hl,bc
	jr	nz,a_13
	ld	bc,(_wing_right_1)
a_13:	ld	(_wing_right_sprite),bc
	ld	bc,(_wing_left_0)
	ld	hl,(_wing_left_sprite)
	or	a,a
	sbc	hl,bc
	jr	nz,a_14
	ld	bc,(_wing_left_1)
a_14:	ld	(_wing_left_sprite),bc
	ld	bc,(_spike_left_0)
	ld	hl,(_spike_left_sprite)
	or	a,a
	sbc	hl,bc
	jr	nz,a_15
	ld	bc,(_spike_left_1)
a_15:	ld	(_spike_left_sprite),bc
	ld	bc,(_spike_right_0)
	ld	hl,(_spike_right_sprite)
	or	a,a
	sbc	hl,bc
	jr	nz,a_16
	ld	bc,(_spike_right_1)
a_16:	ld	(_spike_right_sprite),bc

	ld	a,(_oiram+0)
	xor	a,1
	ld	(_oiram+0),a

	ld	bc,258                      ; TILE_DATA_SIZE
	ld	hl,_tiles+2
	
	ld	a,(hl)
	cp	a,3
	ld	bc,258
	jr	nz,l_192
	ld	a,255
	ld	bc,-1032+258
l_192:
	inc	a
	ld	(hl),a
	
	ld	hl,(_tileset_tiles)
	add	hl,bc
	ld	(_tileset_tiles),hl
	ld	(_tileset_tiles+675),hl
	ld	(_tileset_tiles+678),hl
	ld	(_tileset_tiles+681),hl
	ld	(_tileset_tiles+684),hl
	ld	(_tileset_tiles+687),hl
	ld	(_tileset_tiles+690),hl
	
	ld	hl,(_tileset_tiles+12)
	add	hl,bc
	ld	(_tileset_tiles+12),hl
	
	ld	hl,(_tileset_tiles+378)
	add	hl,bc
	ld	(_tileset_tiles+378),hl
	
	ld	hl,(_tileset_tiles+354)
	add	hl,bc
	ld	(_tileset_tiles+354),hl
	
	ld	hl,(_tileset_tiles+366)
	add	hl,bc
	ld	(_tileset_tiles+366),hl
	
	ld	hl,(_tileset_tiles+396)
	add	hl,bc
	ld	(_tileset_tiles+396),hl
	
	ld	hl,(_tileset_tiles+450)
	add	hl,bc
	ld	(_tileset_tiles+450),hl
	
	ld	hl,(_tileset_tiles+438)
	add	hl,bc
	ld	(_tileset_tiles+438),hl

	ld	hl,(_tileset_tiles+672)
	add	hl,bc
	ld	(_tileset_tiles+672),hl
	
	ld	hl,(_tileset_tiles+699)
	add	hl,bc
	ld	(_tileset_tiles+699),hl
	
	ld	hl,(_tileset_tiles+702)
	add	hl,bc
	ld	(_tileset_tiles+702),hl
	
	ld	hl,_tiles+1
	ld	a,(hl)
	cp	a,2
	ld	bc,258
	jr	nz,l_194
	ld	a,255
	ld	bc,-774+258
l_194:
	inc	a
	ld	(hl),a
	
	ld	hl,(_tileset_tiles+24)
	add	hl,bc
	ld	(_tileset_tiles+24),hl
	
	ld	hl,(_tileset_tiles+282)
	add	hl,bc
	ld	(_tileset_tiles+282),hl
	
	ld	hl,(_tileset_tiles+324)
	add	hl,bc
	ld	(_tileset_tiles+324),hl
	
	xor	a,a
	ld	(_tiles),a
	
l_199:
	ret
	
	.ref _fish_right_sprite
	.ref _fish_left_sprite
	.ref _flame_sprite_down
	.ref _flame_sprite_up
	.ref _spike_right_sprite
	.ref _spike_left_sprite
	.ref _koopa_bones_right_sprite
	.ref _koopa_bones_left_sprite
	.ref _koopa_green_right_sprite
	.ref _koopa_green_left_sprite
	.ref _koopa_red_right_sprite
	.ref _koopa_red_left_sprite
	.ref _wing_right_sprite
	.ref _wing_left_sprite
	.ref _chomper_sprite
	.ref _goomba_sprite
	.ref _fireball_sprite
	.ref _spike_right_1
	.ref _spike_right_0
	.ref _spike_left_1
	.ref _spike_left_0
	.ref _fish_right_1
	.ref _fish_right_0
	.ref _fish_left_1
	.ref _fish_left_0
	.ref _wing_right_1
	.ref _wing_right_0
	.ref _wing_left_1
	.ref _wing_left_0
	.ref _flame_fire_down_1
	.ref _flame_fire_down_0
	.ref _flame_fire_up_1
	.ref _flame_fire_up_0
	.ref _fire_1
	.ref _fire_0
	.ref _chomper_1
	.ref _chomper_0
	.ref _koopa_bones_left_1
	.ref _koopa_bones_left_0
	.ref _koopa_bones_right_1
	.ref _koopa_bones_right_0
	.ref _koopa_green_left_1
	.ref _koopa_green_left_0
	.ref _koopa_green_right_1
	.ref _koopa_green_right_0
	.ref _koopa_red_left_1
	.ref _koopa_red_left_0
	.ref _koopa_red_right_1
	.ref _koopa_red_right_0
	.ref _goomba_1
	.ref _goomba_0
	.ref _oiram
	.ref _level_map
	.ref _tilemap
	.ref _tiles
	.ref _tileset_tiles
	.ref _test_x
	.ref _test_y
	.ref _testing_side
	.ref _tile_handler
	.ref __indcall
	.ref __idvrmu
	.ref __frameset0
