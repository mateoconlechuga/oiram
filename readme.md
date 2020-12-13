# Oiram
Oiram is a mario-style platformer programmed from scratch in C and assembly using the CE development toolchain.

You can find the complete readme [here](https://github.com/mateoconlechuga/oiram/blob/master/extra/readme.md).
There's also a handy level editor [here](https://github.com/mateoconlechuga/oiram-editor/releases/latest).

Animated Screenshots:

![screenshot0](https://raw.githubusercontent.com/mateoconlechuga/oiram/master/extra/screen0.png)
![screenshot1](https://raw.githubusercontent.com/mateoconlechuga/oiram/master/extra/screen1.gif)

## Building
If you would like to build oiram, be sure you have the latest LLVM [CE C Toolchain](https://github.com/CE-Programming/toolchain/releases/latest) installed.

Then simply clone or download the repository from above, and run the following commands:

    make gfx
    make

The following files make up the built project, which should be sent to the calculator:

   `src/gfx/OiramS.8xv`: Oriam sprites

   `src/gfx/OiramT.8xv`: Oriam tileset

   `extra/OiramPK.8xv`: Oriam default level pack

   `bin/Oiram.8xp`: Oriam program


