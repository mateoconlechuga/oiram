cd gfx
convpng
spasm -E oiram_tiles.ez80 OiramT.8xv
spasm -E oiram_sprites.ez80 OiramS.8xv
move OiramS.8xv ..\..\bin\OiramS.8xv
move OiramT.8xv ..\..\bin\OiramT.8xv