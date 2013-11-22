/*###################################################*/
/*  cyclonchip8 - CHIP-8 emulator.                    */
/*  by cyclonmaster <cyclonmaster@gmail.com>         */
/*###################################################*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <time.h>
#include "cyclonchip8.h"
#include "SDL.h"

using namespace std;

/*###################################################*/
/*EMULATOR PART                                      */
/*###################################################*/

	//chip8 sprites may up to 15 bytes for possible sprite size of 8x15
	//sprites, group of bytes which binary represent the desired picture
	//interpreter area of chip8 memory for sprite data, 16*5=80
	unsigned char cyclonchip8_fontset[16*5] = {
								0xF0, 0x90, 0x90, 0x90, 0xF0,	// 0
								0x20, 0x60, 0x20, 0x20, 0x70,	// 1
								0xF0, 0x10, 0xF0, 0x80, 0xF0,	// 2
								0xF0, 0x10, 0xF0, 0x10, 0xF0,	// 3
								0x90, 0x90, 0xF0, 0x10, 0x10,	// 4
								0xF0, 0x80, 0xF0, 0x10, 0xF0,	// 5
								0xF0, 0x80, 0xF0, 0x90, 0xF0,	// 6
								0xF0, 0x10, 0x20, 0x40, 0x40,	// 7
								0xF0, 0x90, 0xF0, 0x90, 0xF0,	// 8
								0xF0, 0x90, 0xF0, 0x10, 0xF0,	// 9
								0xF0, 0x90, 0xF0, 0x90, 0x90,	// A
								0xE0, 0x90, 0xE0, 0x90, 0xE0,	// B
								0xF0, 0x80, 0x80, 0x80, 0xF0,	// C
								0xE0, 0x90, 0x90, 0x90, 0xE0,	// D
								0xF0, 0x80, 0xF0, 0x80, 0xF0,	// E
								0xF0, 0x80, 0xF0, 0x80, 0x80	// F
								};
	
	//tambah menu utama, senang nak call nanti
	cyclonchip8::cyclonchip8()
	{
		init();
		
		// Clear screen once
		drawFlag = true;
		
		srand (time(NULL));
	}
	
	void cyclonchip8::init()
	{
		//Clear display, Total amount of pixels: 2048
		for(int i = 0; i < 2048; ++i)
			gfx[i] = 0;

		//for (int y = 0; y < 64; y++)
		//	for (int x = 0; x < 128; x++)
		//		gfx[x][y] = 0;
		
		opcode	= 0;			// Reset current opcode
		
		// V-regs (V0-VF), general purpose registers & VF carry flag
		for(int i = 0; i < 16; ++i)
			key[i] = V[i] = 0;		
		
		I		= 0;			// Reset index/address register
		pc		= 0x200;		// Program counter starts at 0x200 (Start adress program)
		sp		= 0;			// Reset stack pointer
		
		// Clear stack, Stack (16 levels)
		for(int i = 0; i < 16; ++i)
			stack[i] = 0;
			
		// Clear memory, Memory (size = 4k)
		for(int i = 0; i < 4096; ++i)
			memory[i] = 0;
		
		//tambah pixel_
		//SDL_FreeSurface(pixel_);
		//pixel_ = SDL_CreateRGBSurface(0, 10, 10, 32, 0, 0, 0, 0);
		//SDL_FillRect(pixel_, NULL, SDL_MapRGB(pixel_->format, 255, 255, 255));

		// Reset timers
		delay_timer = 0;
		sound_timer = 0;
		
		//lain-lain???
		
		// Load fontset
		for(int i = 0; i < 80; ++i)
			memory[i] = cyclonchip8_fontset[i];
			
		//di move ke main
		// Clear screen once
//		drawFlag = true;
		
//		srand (time(NULL));
		//stop = false;
	}

//untuk grapfik sprite
//void cyclonchip8::drawSprite(unsigned char X, unsigned char Y, unsigned char N)
//{	
//	V[0xF] = 0;
//			if (N == 0) N = 16;
//			for (int yline = 0; yline < N; yline++)
//			{
//				unsigned char data = memory[I + yline];
//				for (int xpix = 0; xpix < 8; xpix++)
//				{
//					if((data & (0x80 >> xpix)) != 0)
//					{
//						if ((V[X] + xpix) < 64 && (V[Y] + yline) < 32 && (V[X] + xpix) >= 0 && (V[Y] + yline) >= 0)
//						{
//							if (gfx[(V[X] + xpix)*2][(V[Y] + yline)*2] == 1) V[0xF] = 1;
//							gfx[(V[X] + xpix)*2][(V[Y] + yline)*2] ^= 1;
//							gfx[(V[X] + xpix)*2 + 1][(V[Y] + yline)*2] ^= 1;
//							gfx[(V[X] + xpix)*2][(V[Y] + yline)*2 + 1] ^= 1;
//							gfx[(V[X] + xpix)*2 + 1][(V[Y] + yline)*2 + 1] ^= 1;
//						}
//					}
//				}
//			}
//}

void cyclonchip8::emulateCycle()
{
	//Opcode table:
	//CHIP-8 has 35 opcodes, which are all two bytes long. The most significant byte is stored first.
	//The opcodes are listed below, in hexadecimal and with the following symbols:

	//NNN: address
	//NN: 8-bit constant
	//N: 4-bit constant
	//X and Y: 4-bit register identifier
	
	// Fetch opcode
	opcode = memory[pc] << 8 | memory[pc + 1];
	
	// Process opcode
	switch(opcode & 0xF000)
	{		
		//0NNN	Calls RCA 1802 program at address NNN.
		case 0x0000:
			switch(opcode & 0x000F)
			{
				case 0x0000: // 0x00E0: Clears the screen
					for(int i = 0; i < 2048; ++i)
						gfx[i] = 0x0;
				//	for (int i = 0; i < 64; i++)
				//		for (int j = 0; j < 128; j++)
				//			gfx[i][j] = 0;					

					drawFlag = true;
					pc += 2;
				break;

				case 0x000E: // 0x00EE: Returns from subroutine
					--sp;			// 16 levels of stack, decrease stack pointer to prevent overwrite
					pc = stack[sp];	// Put the stored return address from the stack back into the program counter					
					pc += 2;		// Don't forget to increase the program counter!
				break;

				default:
					printf ("Unknown opcode [0x0000]: 0x%X\n", opcode);					
			}
		break;

		case 0x1000: // 0x1NNN: Jumps to address NNN
			pc = opcode & 0x0FFF;
		break;

		case 0x2000: // 0x2NNN: Calls subroutine at NNN.
			stack[sp] = pc;			// Store current address in stack
			++sp;					// Increment stack pointer
			pc = opcode & 0x0FFF;	// Set the program counter to the address at NNN
		break;
		
		case 0x3000: // 0x3XNN: Skips the next instruction if VX equals NN
			if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
				pc += 4;
			else
				pc += 2;
		break;
		
		case 0x4000: // 0x4XNN: Skips the next instruction if VX doesn't equal NN
			if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
				pc += 4;
			else
				pc += 2;
		break;
		
		case 0x5000: // 0x5XY0: Skips the next instruction if VX equals VY.
			if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
				pc += 4;
			else
				pc += 2;
		break;
		
		case 0x6000: // 0x6XNN: Sets VX to NN.
			V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
			pc += 2;
		break;
		
		case 0x7000: // 0x7XNN: Adds NN to VX.
			V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
			pc += 2;
		break;
		
		case 0x8000:
			switch(opcode & 0x000F)
			{
				case 0x0000: // 0x8XY0: Sets VX to the value of VY
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;

				case 0x0001: // 0x8XY1: Sets VX to "VX OR VY"
					V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;

				case 0x0002: // 0x8XY2: Sets VX to "VX AND VY"
					V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;

				case 0x0003: // 0x8XY3: Sets VX to "VX XOR VY"
					V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;

				case 0x0004: // 0x8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't					
					if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) 
						V[0xF] = 1; //carry
					else 
						V[0xF] = 0;					
					V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
					pc += 2;					
				break;

				case 0x0005: // 0x8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
					if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) 
						V[0xF] = 0; // there is a borrow
					else 
						V[0xF] = 1;					
					V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;

				case 0x0006: // 0x8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift
					V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
					V[(opcode & 0x0F00) >> 8] >>= 1;
					pc += 2;
				break;

				case 0x0007: // 0x8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
					if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])	// VY-VX
						V[0xF] = 0; // there is a borrow
					else
						V[0xF] = 1;
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];				
					pc += 2;
				break;

				case 0x000E: // 0x8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift
					V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
					V[(opcode & 0x0F00) >> 8] <<= 1;
					pc += 2;
				break;

				default:
					printf ("Unknown opcode [0x8000]: 0x%X\n", opcode);
			}
		break;
		
		case 0x9000: // 0x9XY0: Skips the next instruction if VX doesn't equal VY
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
			V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
			pc += 2;
		break;
	
		case 0xD000: // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. 
					 // Each row of 8 pixels is read as bit-coded starting from memory location I; 
					 // I value doesn't change after the execution of this instruction. 
					 // VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, 
					 // and to 0 if that doesn't happen
		//draw sprite asal
		{
			unsigned short x = V[(opcode & 0x0F00) >> 8];
			unsigned short y = V[(opcode & 0x00F0) >> 4];
			unsigned short height = opcode & 0x000F;
			unsigned short pixel;

			V[0xF] = 0;
			for (int yline = 0; yline < height; yline++)
			{
				pixel = memory[I + yline];
				for(int xline = 0; xline < 8; xline++)
				{
					if((pixel & (0x80 >> xline)) != 0)
					{
						if(gfx[(x + xline + ((y + yline) * 64))] == 1)
						{
							V[0xF] = 1;                                    
						}
						gfx[x + xline + ((y + yline) * 64)] ^= 1;
					}
				}
			}
					
			drawFlag = true;			
			pc += 2;
		}
		//draw sprite lain
		//drawSprite(((opcode & 0x0F00)>>8), ((opcode & 0x00F0)>>4), (opcode & 0x000F));
		break;
			
		case 0xE000:
			switch(opcode & 0x00FF)
			{
				case 0x009E: // EX9E: Skips the next instruction if the key stored in VX is pressed
					if(key[V[(opcode & 0x0F00) >> 8]] != 0)
						pc += 4;
					else
						pc += 2;
				break;
				
				case 0x00A1: // EXA1: Skips the next instruction if the key stored in VX isn't pressed
					if(key[V[(opcode & 0x0F00) >> 8]] == 0)
						pc += 4;
					else
						pc += 2;
				break;

				default:
					printf ("Unknown opcode [0xE000]: 0x%X\n", opcode);
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
				{
					bool keyPress = false;

					for(int i = 0; i < 16; ++i)
					{
						if(key[i] != 0)
						{
							V[(opcode & 0x0F00) >> 8] = i;
							keyPress = true;
						}
					}

					// If we didn't received a keypress, skip this cycle and try again.
					if(!keyPress)						
						return;

					pc += 2;					
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
					if(I + V[(opcode & 0x0F00) >> 8] > 0xFFF)	// VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
						V[0xF] = 1;
					else
						V[0xF] = 0;
					I += V[(opcode & 0x0F00) >> 8];
					pc += 2;
				break;

				case 0x0029: // FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
					I = V[(opcode & 0x0F00) >> 8] * 0x5;
					pc += 2;
				break;

				case 0x0033: // FX33: Stores the Binary-coded decimal representation of VX at the addresses I, I plus 1, and I plus 2
					memory[I]     = V[(opcode & 0x0F00) >> 8] / 100;
					memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
					memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;					
					pc += 2;
				break;

				case 0x0055: // FX55: Stores V0 to VX in memory starting at address I					
					for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
						memory[I + i] = V[i];	

					// On the original interpreter, when the operation is done, I = I + X + 1.
					I += ((opcode & 0x0F00) >> 8) + 1;
					pc += 2;
				break;

				case 0x0065: // FX65: Fills V0 to VX with values from memory starting at address I					
					for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
						V[i] = memory[I + i];			

					// On the original interpreter, when the operation is done, I = I + X + 1.
					I += ((opcode & 0x0F00) >> 8) + 1;
					pc += 2;
				break;

				default:
					printf ("Unknown opcode [0xF000]: 0x%X\n", opcode);
			}
		break;

		default:
			printf ("Unknown opcode: 0x%X\n", opcode);
	}	

	//bawa ke luar
	// Update timers
	//if(delay_timer > 0)
	//	--delay_timer;

	//if(sound_timer > 0)
	//{
	//	if(sound_timer == 1)
	//		printf("BEEP!\n");
	//	--sound_timer;
	//}	
}

// Update timers
void cyclonchip8::decreaseTimers()
{
	if(delay_timer > 0)
		--delay_timer;

	if(sound_timer > 0)
	{
		if(sound_timer == 1)
			printf("BEEP!\n");
		--sound_timer;
	}	
}

void cyclonchip8::debugRender()
{
	// Draw
	for(int y = 0; y < 32; ++y)
	{
		for(int x = 0; x < 64; ++x)
		{
			if(gfx[(y*64) + x] == 0) 
				printf("O");
			else 
				printf(" ");
		}
		printf("\n");
	}
	printf("\n");
}

bool cyclonchip8::loadApplication(const char * filename)
{
	ifstream file(filename, ios::in | ios::binary | ios::ate);
	if (file.is_open())
	{
		ifstream::pos_type size;
		size = file.tellg();
		if (size > 0x0FFF - 0x200)
		{
			cerr << "Error: file '" << filename << "' is too large." << endl;
			return false;
		}
		file.seekg(0, ios::beg);
		file.read(reinterpret_cast<char*>(&memory[0x200]), size);
		file.close();
		
		cout << "File '" << filename << "' loaded." << endl;
		return true;
	}

	cerr << "Error: unable to open file '" << filename << "'" << endl;
	return false;
}

//fileopen asal
//bool cyclonchip8::loadApplication(const char * filename)
//{
//	init();
//	printf("Loading: %s\n", filename);
//		
	// Open file
//	FILE * pFile = fopen(filename, "rb");
//	if (pFile == NULL)
//	{
//		fputs ("File error", stderr); 
//		return false;
//	}

	// Check file size
//	fseek(pFile , 0 , SEEK_END);
//	long lSize = ftell(pFile);
//	rewind(pFile);
//	printf("Filesize: %d\n", (int)lSize);
	
	// Allocate memory to contain the whole file
//	char * buffer = (char*)malloc(sizeof(char) * lSize);
//	if (buffer == NULL) 
//	{
//		fputs ("Memory error", stderr); 
//		return false;
//	}

	// Copy the file into the buffer
//	size_t result = fread (buffer, 1, lSize, pFile);
//	if (result != lSize) 
//	{
//		fputs("Reading error",stderr); 
//		return false;
//	}

	// Copy buffer to Chip8 memory
//	if((4096-512) > lSize)
//	{
//		for(int i = 0; i < lSize; ++i)
//			memory[i + 512] = buffer[i];
//	}
//	else
//		printf("Error: ROM too big for memory");
	
	// Close file, free buffer
//	fclose(pFile);
//	free(buffer);

//	return true;
//}

/*###################################################*/
/*SDL PART                                           */
/*###################################################*/
SDL_Surface *screen_surf;

cyclonchip8 emu;
bool done;
int opcode_count;

bool gfx_init()
{
	// Initializes the video subsystem
	if(SDL_Init(SDL_INIT_VIDEO) != 0) {
		return false;
	}

	SDL_WM_SetCaption("cyclonchip8-SDL", "cyclonchip8-SDL");
	
	//saiz screen emulator
	//if((screen_surf = SDL_SetVideoMode(256, 128, 24, SDL_HWSURFACE | SDL_DOUBLEBUF)) == NULL) {
	if((screen_surf = SDL_SetVideoMode(640, 320, 32, SDL_HWSURFACE | SDL_DOUBLEBUF)) == NULL) {
		return false;
	}
	
	return true;
}

void draw()
{
	//SDL_FillRect(screen_surf, NULL, 0x000000);
	//int t = 0;
	//for (int y = 0; y < 64; y++)
	//	for (int x = 0; x < 128; x++)
	//	{
			//if (emu.gfx[x][y] == 1)
	//		if (emu.gfx[y * 64 + x] == 1)
	//		{
	//			SDL_Rect dest;
	//			dest.x = x*2;
	//			dest.y = y*2;
	//			dest.w = 2;
	//			dest.h = 2;
	//			SDL_FillRect(screen_surf, &dest, SDL_MapRGB(screen_surf->format, 0xff, 0xff, 0xff));
	//		}
	//	}

	//code draw lain
	    SDL_FillRect(screen_surf, NULL, SDL_MapRGB(screen_surf->format, 0, 0, 0));

    SDL_Rect pixel_pos;

    for (size_t x = 0; x < 64; ++x)
    {
        for (size_t y = 0; y < 32; ++y)
        {
            if (emu.gfx[y * 64 + x] == 1)
            {
                pixel_pos.x = x * 10;
                pixel_pos.y = y * 10;

                //SDL_BlitSurface(pixel_, NULL, screen_surf, &pixel_pos);				
            }
        }
    }

    SDL_Flip(screen_surf);
	//code draw lain tamat
}

void handle_input(SDL_Event &event)
{
	switch(event.type){
		case SDL_KEYDOWN:
			switch(event.key.keysym.sym){
				case SDLK_1: emu.key[0x1]=1; break;
				case SDLK_2: emu.key[0x2]=1; break;
				case SDLK_3: emu.key[0x3]=1; break;
				case SDLK_4: emu.key[0xc]=1; break;
				case SDLK_q: emu.key[0x4]=1; break;
				case SDLK_w: emu.key[0x5]=1; break;
				case SDLK_e: emu.key[0x6]=1; break;
				case SDLK_r: emu.key[0xd]=1; break;
				case SDLK_a: emu.key[0x7]=1; break;
				case SDLK_s: emu.key[0x8]=1; break;
				case SDLK_d: emu.key[0x9]=1; break;
				case SDLK_f: emu.key[0xe]=1; break;
				case SDLK_z: emu.key[0xa]=1; break; 
				case SDLK_x: emu.key[0x0]=1; break;
				case SDLK_c: emu.key[0xb]=1; break;
				case SDLK_v: emu.key[0xf]=1; break;
			}
			break;
		case SDL_KEYUP:
			switch(event.key.keysym.sym){
				case SDLK_1: emu.key[0x1]=0; break;
				case SDLK_2: emu.key[0x2]=0; break;
				case SDLK_3: emu.key[0x3]=0; break;
				case SDLK_4: emu.key[0xc]=0; break;
				case SDLK_q: emu.key[0x4]=0; break;
				case SDLK_w: emu.key[0x5]=0; break;
				case SDLK_e: emu.key[0x6]=0; break;
				case SDLK_r: emu.key[0xd]=0; break;
				case SDLK_a: emu.key[0x7]=0; break;
				case SDLK_s: emu.key[0x8]=0; break;
				case SDLK_d: emu.key[0x9]=0; break;
				case SDLK_f: emu.key[0xe]=0; break;
				case SDLK_z: emu.key[0xa]=0; break; 
				case SDLK_x: emu.key[0x0]=0; break;
				case SDLK_c: emu.key[0xb]=0; break;
				case SDLK_v: emu.key[0xf]=0; break;
			}
			break;
		case SDL_QUIT:
			done = true;
			break;
	}
}

int main(int argc, char *argv[])
{
	cout << "cyclonchip8-SDL " << endl << "Copyright (C) cyclonmaster <cyclonmaster@gmail.com>" << endl << endl;
	if (argc == 1){
		cerr << "Error: no input file specified" << endl;
		cerr << "Usage: " << argv[0] << " FILE" << endl;
		return 1;
	}
	
	if (gfx_init() == false)
	{
		cerr << "Unable to initialize SDL: " << SDL_GetError() << endl;
		exit(1);
	}
	
	emu.init();
	if (emu.loadApplication(argv[1])){
		SDL_Event event;
		done = false;
		Uint32 lasttick = SDL_GetTicks(); //for delay fps
		
		int opcode_count = 0;
        int cycles_per_second;
		while (!done)
		{
			//keyboard
			while (SDL_PollEvent(&event)) {
				handle_input(event);
			}
			//tukar2 mode chip8 & superchip8
            //if (emu.mode == 0)
                cycles_per_second = 10; // execute 600 opcodes per second
            //else
            //    cycles_per_second = 30; // 1800 opcodes per second

			if (opcode_count < cycles_per_second)
			{
				emu.emulateCycle();
				opcode_count++;
			}

			//decrease timers every 1/60sec and redraw screen
			if (SDL_GetTicks() - lasttick >= 1000/60)
			{
				emu.decreaseTimers();
				lasttick = SDL_GetTicks();
				
				draw();
				SDL_Flip(screen_surf);
				
				opcode_count = 0;
			}
			
			if (emu.stop) done = true;
		}
	}
	
	SDL_FreeSurface(screen_surf);
	SDL_QUIT;
	return 0;
}