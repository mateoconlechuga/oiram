	assume adl=1

	section	.text

	public _moveable_tile_right_bottom
_moveable_tile_right_bottom:
	xor	a,a                      ; TEST_RIGHT
	jr	_test_against

	public _moveable_tile_left_bottom
_moveable_tile_left_bottom:
	ld	a,1                      ; TEST_LEFT
	jr	_test_against

	public _moveable_tile
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
	call	__jump_iy
	pop	de
	ret

__jump_iy:
	jp	(iy)

	public _solid_tile_handler
_solid_tile_handler:
	xor	a,a
	ret

	public _empty_tile_handler
_empty_tile_handler:
	ld	a,1
	ret

	public _tile_to_abs_xy_pos
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

	public _animate
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

	extern _fish_right_sprite
	extern _fish_left_sprite
	extern _flame_sprite_down
	extern _flame_sprite_up
	extern _spike_right_sprite
	extern _spike_left_sprite
	extern _koopa_bones_right_sprite
	extern _koopa_bones_left_sprite
	extern _koopa_green_right_sprite
	extern _koopa_green_left_sprite
	extern _koopa_red_right_sprite
	extern _koopa_red_left_sprite
	extern _wing_right_sprite
	extern _wing_left_sprite
	extern _chomper_sprite
	extern _goomba_sprite
	extern _fireball_sprite
	extern _spike_right_1
	extern _spike_right_0
	extern _spike_left_1
	extern _spike_left_0
	extern _fish_right_1
	extern _fish_right_0
	extern _fish_left_1
	extern _fish_left_0
	extern _wing_right_1
	extern _wing_right_0
	extern _wing_left_1
	extern _wing_left_0
	extern _flame_fire_down_1
	extern _flame_fire_down_0
	extern _flame_fire_up_1
	extern _flame_fire_up_0
	extern _fire_1
	extern _fire_0
	extern _chomper_1
	extern _chomper_0
	extern _koopa_bones_left_1
	extern _koopa_bones_left_0
	extern _koopa_bones_right_1
	extern _koopa_bones_right_0
	extern _koopa_green_left_1
	extern _koopa_green_left_0
	extern _koopa_green_right_1
	extern _koopa_green_right_0
	extern _koopa_red_left_1
	extern _koopa_red_left_0
	extern _koopa_red_right_1
	extern _koopa_red_right_0
	extern _goomba_1
	extern _goomba_0
	extern _oiram
	extern _level_map
	extern _tilemap
	extern _tiles
	extern _tileset_tiles
	extern _test_x
	extern _test_y
	extern _testing_side
	extern _tile_handler
	extern __indcall
	extern __idvrmu
	extern __frameset0
