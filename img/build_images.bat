cd gfx
convpng
spasm -E mario_tiles.ez80 MarioT.8xv
spasm -E mario_sprites.ez80 MarioS.8xv
move MarioS.8xv ..\..\bin\MarioS.8xv
move MarioT.8xv ..\..\bin\MarioT.8xv