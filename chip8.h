#ifndef CHIP8
#define CHIP8

#include <SDL/SDL.h>
#include <stdio.h>

#define memsize 4096
#define SCREEN_W 640
#define SCREEN_H 320
#define SCREEN_BPP 32

static int keymap[0x10] = 
{
    SDLK_5,
    SDLK_6,
    SDLK_7,
    SDLK_8,
    SDLK_t,
    SDLK_y,
    SDLK_u,
    SDLK_i,
    SDLK_g,
    SDLK_h,
    SDLK_j,
    SDLK_k,
    SDLK_v,
    SDLK_b,
    SDLK_n,
    SDLK_m,
};

unsigned char chip8_fontset[80];

unsigned short opcode;
unsigned char memory[4096];
unsigned char V[0x10];
unsigned short I;
unsigned short pc;
unsigned char gfx[64 * 32];
unsigned char delay_timer;
unsigned char sound_timer;
unsigned short stack[0x10];
unsigned short sp;
char current_file[255];

void Chip8_Initialize(void);
int Chip8_LoadGame(char * file);
void Chip8_EmulateCycle(void);
void Chip8_SetKeys(SDL_Event * event);
void Chip8_Timers(void);
void Chip8_Draw(void);

#endif
