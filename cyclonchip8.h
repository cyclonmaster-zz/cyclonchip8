class chip8 {
	public:
		chip8();
		
		bool drawFlag;

		void emulateCycle();
		void debugRender();
		bool loadApplication(const char * filename);		

// Chip8
		unsigned char  gfx[64 * 32];	// Total amount of pixels: 2048
		unsigned char  key[16];			

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

		void init();
};