chip8: main.c chip8.c
	gcc -o chip8 main.c chip8.c -I -Wall `sdl-config --cflags --libs`.
