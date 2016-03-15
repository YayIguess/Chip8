chip8: main.c opcode.c cpucore.c gfx.c
	gcc -o chip8 main.c chip8.c -I -Wall `sdl-config --cflags --libs`.
