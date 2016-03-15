#include "chip8.h"

int main(int argc, char *argv[]) 
{
	// Set up render system and register input callbacks
    Uint8 * keys;
    SDL_Event event;
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_SetVideoMode(SCREEN_W, SCREEN_H, SCREEN_BPP, SDL_HWSURFACE | SDL_DOUBLEBUF);


	// Initialize the Chip8 system and load the game into the memory  
	Chip8_Initialize();
	Chip8_LoadGame(NULL);
 
	// Emulation loop
	for(;;)
	{
        if(SDL_PollEvent(&event))
            continue;

		// Emulate one cycle
		Chip8_EmulateCycle();
 
		// If the draw flag is set, update the screen
		Chip8_Draw();
 
		// Store key press state (Press and Release)
		Chip8_SetKeys(&event);	
	}
 
	return 0;
}
