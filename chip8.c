#include "chip8.h"


unsigned char chip8_fontset[80] =
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

void Chip8_Initialize(void)
{
	pc = 0x200;  // Program counter starts at 0x200
	opcode = 0;      // Reset current opcode
	I = 0;      // Reset index register
	sp = 0;      // Reset stack pointer

	memset(gfx, 0, sizeof(gfx)); //clear display
	memset(stack, 0, sizeof(stack)); // Clear stack
	memset(V, 0, sizeof(V)); // Clear registers V0-VF
	memset(memory, 0, sizeof(memory));// Clear memory

	//Load fontset
	for(int i = 0; i < 80; ++i)
	{
		memory[i] = chip8_fontset[i];
	}

	//Reset timers
}

int Chip8_LoadGame(char * file)
{
	FILE *fp;

	if(file == NULL)
	{
		printf("What game do you want to play?\n");
		scanf("%s", current_file);
	}

	fp = fopen(current_file, "rb");


	if(!fp)
	{
		printf("Specified file does not exist! Quitting!\n");
		exit(1);
	}
	//Load game
	fread(memory + 0x200, 1, memsize - 0x200, fp);
}


void Chip8_EmulateCycle(void)
{

	Uint8 *keys;
	int y, x, vx, vy, times, i;
	unsigned height, pixel;

	for(times = 0; times < 10; times++)
	{
		//Fetch opcode
		opcode = memory[pc] << 8 | memory[pc + 1];
		printf("Executing %04X at %04X , I:%02X SP:%02X\n", opcode, pc, I, sp);
		//Decode opcode
		switch(opcode & 0xF000)
		{
			case 0x0000:
				switch(opcode & 0x000F)
				{
					case 0x0000: // 00E0: Clears the screen
						memset(gfx, 0, sizeof(gfx));
						pc += 2;
						break;

					case 0x000E: // 00EE: Returns from subroutine
						pc = stack[(--sp) & 0xF] + 2;
						break;
					default:
						printf("Wrong opcode: %X\n", opcode);
						getchar();
				}
				break;

			case 0x1000: // 1NNN: Jumps to address NNN
				pc = opcode & 0x0FFF;
				break;

			case 0x2000: // 2NNN: Calls subroutine at NNN
				stack[(sp++) & 0xF] = pc;
				pc = opcode & 0x0FFF;
				break;

			case 0x3000: // 3XNN: Skips the next instruction if VX equals NN
				if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
					pc += 4;
				else
					pc += 2;
				break;

			case 0x4000: // 4XNN: Skips the next instruction if VX doesn't equal NN
				if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
					pc += 4;
				else
					pc += 2;
				break;

			case 0x5000: // 5XY0: Skips the next instruction if VX equals VY
				if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
					pc += 4;
				else
					pc += 2;
				break;

			case 0x6000: // 6XNN: Sets VX to NN
				V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
				pc += 2;
				break;

			case 0x7000: // 7XNN: Adds NN to VX
				V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
				pc += 2;
				break;

			case 0x8000:
				switch(opcode & 0x000F)
				{
					case 0x0000: // 8XY0: Sets VX to the value of VY
						V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
						pc += 2;
						break;

					case 0x0001: // 8XY1: Sets VX to VX or VY
						V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];
						pc += 2;
						break;

					case 0x0002: // 8XY2: Sets VX to VX and VY
						V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];
						pc += 2;
						break;

					case 0x0003: // 8XY3: Sets VX to VX xor VY
						V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4];
						pc += 2;
						break;

					case 0x0004: // 8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
						if(((int) V[(opcode & 0x0F00) >> 8] + (int) V[(opcode & 0x00F0) >> 4]) < 256)
							V[0xF] &= 0;
						else
							V[0xF] = 1;
						V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
						pc += 2;
						break;

					case 0x0005: // 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
						if(((int) V[(opcode & 0x0F00) >> 8] - (int) V[(opcode & 0x00F0) >> 4]) >= 0)
							V[0xF] = 1;
						else
							V[0xF] &= 0;

						V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
						pc += 2;
						break;

					case 0x0006: // 8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift
						V[0xF] = V[(opcode & 0x0F00) >> 8] & 7;
						V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] >> 1;
						pc += 2;
						break;

					case 0x0007: // 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
						if(((int) V[(opcode & 0x0F00) >> 8] - (int) V[(opcode & 0x00F0) >> 4]) > 0)
							V[0xF] = 1;
						else
							V[0xF] &= 0;

						V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
						pc += 2;
						break;

					case 0x000E: // 8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift
						V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
						V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] << 1;
						pc += 2;
						break;
					default:
						printf("Wrong opcode: %X\n", opcode);
						getchar();
				}
				break;

			case 0x9000: // 9XY0: Skips the next instruction if VX doesn't equal VY
				if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
					pc += 4;
				else
					pc += 2;
				break;

			case 0xA000: // ANNN: Sets I to the address NNN
				I = opcode & 0x0FFF;
				pc += 2;
				break;

			case 0xB000: // BNNN: Jumps to the address NNN plus V0
				pc = (opcode & 0x0FFF) + V[0];
				break;

			case 0xC000: // CXNN: Sets VX to a random number and NN
				V[(opcode & 0x0F00) >> 8] = rand() & (opcode & 0x00FF);
				pc += 2;
				break;

			case 0xD000: // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels
				vx = V[(opcode & 0x0F00) >> 8];
				vy = V[(opcode & 0x00F0) >> 4];
				height = opcode & 0x000F;
				V[0xF] &= 0;

				for(y = 0; y < height; y++)
				{
					pixel = memory[I + y];
					for(x = 0; x < 8; x++)
					{
						if(pixel & (0x80 >> x))
						{
							if(gfx[x + vx + (y + vy) * 64])
								V[0xF] = 1;
							gfx[x + vx + (y + vy) * 64] ^= 1;
						}
					}
				}
				pc += 2;
				break;

			case 0xE000:
				switch(opcode & 0x000F)
				{
					case 0x000E: // EX9E: Skips the next instruction if the key stored in VX is pressed
						keys = SDL_GetKeyState(NULL);
						if(keys[keymap[V[(opcode & 0x0F00) >> 8]]])
							pc += 4;
						else
							pc += 2;
						break;

					case 0x0001: // EXA1: Skips the next instruction if the key stored in VX isn't pressed
						keys = SDL_GetKeyState(NULL);
						if(!keys[keymap[V[(opcode & 0x0F00) >> 8]]])
							pc += 4;
						else
							pc += 2;

						break;
					default:
						printf("Wrong opcode: %X\n", opcode);
						getchar();
				}
				break;

			case 0xF000:
				switch(opcode & 0x00FF)
				{
					case 0x0007: // FX07: Sets VX to the value of the delay timer
						V[(opcode & 0x0F00) >> 8] = delay_timer;
						pc += 2;
						break;

					case 0x000A: // FX0A: A key press is awaited, and then stored in VX
						keys = SDL_GetKeyState(NULL);
						for(i = 0; i < 0x10; i++)
						{
							if(keys[keymap[i]])
							{
								V[(opcode & 0x0F00) >> 8] = i;
								pc += 2;
							}
						}
						break;

					case 0x0015: // FX15: Sets the delay timer to VX
						delay_timer = V[(opcode & 0x0F00) >> 8];
						pc += 2;
						break;

					case 0x0018: // FX18: Sets the sound timer to VX
						sound_timer = V[(opcode & 0x0F00) >> 8];
						pc += 2;
						break;

					case 0x001E: // FX1E: Adds VX to I
						I += V[(opcode & 0x0F00) >> 8];
						pc += 2;
						break;

					case 0x0029: // FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
						I = V[(opcode & 0x0F00) >> 8] * 5;
						pc += 2;
						break;

					case 0x0033: // FX33: Stores the Binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2
						memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
						memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
						memory[I + 2] = V[(opcode & 0x0F00) >> 8] % 10;
						pc += 2;
						break;

					case 0x0055: // FX55: Stores V0 to VX in memory starting at address I
						for(i = 0; i <= ((opcode & 0x0F00) >> 8); i++)
						{
							memory[I + i] = V[i];
						}
						pc += 2;
						break;

					case 0x0065: //FX65: Fills V0 to VX with values from memory starting at address I
						for(i = 0; i <= ((opcode & 0x0F00) >> 8); i++)
						{
							V[i] = memory[I + i];
						}
						pc += 2;
						break;

					default:
						printf("Wrong opcode: %X\n", opcode);
						getchar();
				}
				break;
			default:
				printf("Wrong opcode: %X\n", opcode);
				getchar();
		}
		// Update timers
		Chip8_Timers();
	}
}

void Chip8_Timers(void)
{
	if(delay_timer > 0)
		delay_timer--;
	if(sound_timer > 0)
		sound_timer--;
	if(sound_timer != 0)
		printf("%c", 7);
}

void Chip8_SetKeys(SDL_Event *event)
{
	Uint8 *keys = SDL_GetKeyState(NULL);
	if(keys[SDLK_ESCAPE])
		exit(1);
	if(keys[SDLK_r])
	{
		Chip8_Initialize();
		Chip8_LoadGame(current_file);
	}
	else if(keys[SDLK_c])
	{
		Chip8_Initialize();
		Chip8_LoadGame(NULL);
	}
	if(keys[SDLK_p])
	{
		while(1)
		{
			if(SDL_PollEvent(event))
			{
				keys = SDL_GetKeyState(NULL);
				if(keys[SDLK_ESCAPE])
					exit(1);
				if(keys[SDLK_u])
					break;
			}
		}
	}
}

void Chip8_Draw(void)
{

	int i, j;
	SDL_Surface *surface = SDL_GetVideoSurface();
	SDL_LockSurface(surface);
	Uint32 *screen = (Uint32 *) surface->pixels;
	memset(screen, 0, surface->w * surface->h * sizeof(Uint32));

	for(i = 0; i < SCREEN_H; i++)
	{
		for(j = 0; j < SCREEN_W; j++)
		{
			screen[j + i * surface->w] = gfx[(j / 10) + (i / 10) * 64] ? 0xFFFFFFFF : 0;
		}
	}

	SDL_UnlockSurface(surface);
	SDL_Flip(surface);
	SDL_Delay(15);
}
