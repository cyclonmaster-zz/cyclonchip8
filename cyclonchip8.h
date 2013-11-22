#include <string>

class cyclonchip8 {
// Chip8
	private:	
		unsigned short opcode;			// Current opcode		
		unsigned char  V[16];			// V-regs (V0-VF), general purpose registers & VF carry flag
		unsigned short I;				// Index/address register
		unsigned short pc;				// Program counter
		unsigned short sp;				// Stack pointer
		
		unsigned short stack[16];		// Stack (16 levels)
		unsigned char  memory[4096];	// Memory (size = 4k)		
				
		unsigned char  delay_timer;		// Delay timer
		unsigned char  sound_timer;		// Sound timer
		//void drawSprite(unsigned char X, unsigned char Y, unsigned char N);

	public:
		unsigned char  gfx[64 * 32];	// Total amount of pixels: 2048
		//unsigned char gfx[128][64];
		//unsigned char  gfx[128 * 64];
		unsigned char  key[16];	
		
		bool stop;						// for 00FD opcode

		cyclonchip8();
		
		bool drawFlag;

		void init();
		void debugRender();
		bool loadApplication(const char * filename);
		void emulateCycle();
		void decreaseTimers();
};