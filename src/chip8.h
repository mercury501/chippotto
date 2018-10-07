#include <ctime>


using namespace std;

class chip8 {
private:
	const unsigned char fontset[80] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0,		// 0
	0x20, 0x60, 0x20, 0x20, 0x70,		// 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0,		// 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0,		// 3
	0x90, 0x90, 0xF0, 0x10, 0x10,		// 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0,		// 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0,		// 6
	0xF0, 0x10, 0x20, 0x40, 0x40,		// 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0,		// 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0,		// 9
	0xF0, 0x90, 0xF0, 0x90, 0x90,		// A
	0xE0, 0x90, 0xE0, 0x90, 0xE0,		// B
	0xF0, 0x80, 0x80, 0x80, 0xF0,		// C
	0xE0, 0x90, 0x90, 0x90, 0xE0,		// D
	0xF0, 0x80, 0xF0, 0x80, 0xF0,		// E
	0xF0, 0x80, 0xF0, 0x80, 0x80		// F
	};
	
	unsigned short opcode; //2 bytes
	
	unsigned char memory[4096]; //ram
	/*0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
	0-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
	0-0xFFF - Program ROM and work RAM
	*/
	unsigned char v[16]; //registri 15
	
	unsigned short I; //address register
	unsigned short pc; //program counter
	
	
	

	unsigned char delay_timer;
	unsigned char sound_timer; //counters 60Hz decreasing

	unsigned short stack[16]; //16 bytes for stack
	unsigned short sp;

	
	
public:
	char key[16];  //hex keypad, 0x0 - 0xf

	bool drawflag = 0;
	unsigned char gfx[64 * 32];  //graphics memory, 2048 bits

	void initialize(void) {
				
		for (int i = 0; i < 4096; i++) //initialize memory
			memory[i] = 0;
		
		
		for (int i = 0; i < 80; i++)  //load font
			memory[i] = fontset[i];

		soft_reset();

	}

	//void print_mem(void) {

	//	for (int i = 0; i < 100; i++) {  //print memory, debug
	//		cout << int(memory[i]) << " ";
	//			if ((i % 20 && i!=0) == 0)
	//				cout << endl;
	//	}


	//}
	void soft_reset(void) {
		
		I = 0;
		pc = 0x200;
		opcode = 0;
		sp = 0;

		for (int i = 0; i < 16; i++) {  //initialize v stack key
			v[i] = 0;
			stack[i] = 0;
			key[i] = 0;
		}

		for (int i = 0; i < 64 * 32; i++)   //initialize graphics memory
			gfx[i] = 0;



		delay_timer = 0;
		sound_timer = 0;

		srand(time(NULL));

	}

	bool load_binary(string path) { //load rom starting at 0x200
		
		char cpath [200];
		strcpy_s(cpath, path.c_str()); 

		return load_bin_char(cpath);
		
	}

	bool load_bin_char(char strong[]) {
		unsigned char buffer[4000];

		FILE* rom = fopen(strong, "rb");

		fseek(rom, 0, SEEK_END); //seek the end of rom
		long rom_size = ftell(rom); //store the position in the file stream as its size
		rewind(rom); //rewind the rom like a VHS

		fread(buffer, rom_size, 1, rom);

		//load buffer in memory
		if (rom_size > 4096 - 512) {
			cout << endl << "Rom too big" << endl;
			return 0;
		}


		for (int i = 0; i < rom_size; i++)
			memory[i + 512] = buffer[i];

		fclose(rom);
		//free(buffer);

		return 1;


	}

	void emucycle() {

		opcode = memory[pc] << 8 | memory[pc + 1];

		
		
		switch (opcode & 0xf000) { //decode opcode masking lower byte and a half
		
			
		
		case 0x0000:
			switch (opcode & 0x000f) {
			case  0x0:  //clear screen
				for (int i = 0; i < 64 * 32; i++) {
					gfx[i] = 0;
				}
				drawflag = 1;
				pc += 2;
				break;

			case 0xe:  //RTS
				sp--;
				pc = stack[sp];
				break;

			default:
				cout << "Unknown opcode";
				break;

			}
			break;


			
		case 0x1000: //jump to 0x0nnn
			pc = opcode & 0x0fff;
			break;

		case 0x2000: //2NNN	Flow	*(0xNNN)()	Calls subroutine at NNN.
			stack[sp] = pc + 2;
			sp++;
			pc = opcode & 0x0fff;
			break;

		case 0x3000: //skips next instruction if vX == nn   (0x3xnn)
			if ((v[(opcode & 0x0f00) >> 8]) == (opcode & 0x00ff))
				pc += 4;
			else
				pc += 2;
			break;

		case 0x4000: //skips next instruction if vX != nn   (0x4xnn)
			if ((v[(opcode & 0x0f00) >> 8]) != (opcode & 0x00ff))
				pc += 4;
			else
				pc += 2;
			break;

		case 0x5000: // skips next instruction if vX == vY   0x5xy0
			if (v[(opcode & 0x0f00) >>8] == v[(opcode & 0x00f0) >>4])
				pc += 4;
			else
				pc += 2;
			break;

		case 0x6000:  //6XNN	Const	Vx = NN	Sets VX to NN.
			v[(opcode & 0x0f00) >> 8] = opcode & 0x00ff;
			pc += 2;
			break;

		case 0x8000: 
			switch (opcode & 0x000f) {
				case 0x0: //8XY0	Assign	Vx=Vy	Sets VX to the value of VY.

					v[(opcode & 0x0f00) >> 8] = v[(opcode & 0x00f0) >> 4];
					pc += 2;
					break;
				case 0x1:  //8XY1	BitOp	Vx = Vx | Vy	Sets VX to VX or VY. (Bitwise OR operation)
					v[(opcode & 0x0f00) >> 8] = v[(opcode & 0x0f00) >> 8] | v[(opcode & 0x00f0) >> 4];
					pc += 2;
					break;

				case 0x2: //8XY2	BitOp	Vx=Vx&Vy	Sets VX to VX and VY. (Bitwise AND operation)
					v[(opcode & 0x0f00) >> 8] = v[(opcode & 0x0f00) >> 8] & v[(opcode & 0x00f0) >> 4];
					pc += 2;
					break;

				case 0x3:  //8XY3	BitOp	Vx=Vx^Vy	Sets VX to VX xor VY.
					v[(opcode & 0x0f00) >> 8] = v[(opcode & 0x0f00) >> 8] != v[(opcode & 0x00f0) >> 4];
					pc += 2;
					break;

				case 0x4:  //8XY4	Math	Vx += Vy	Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
					if(v[(opcode & 0x00f0) >> 4] >(0xff - v[(opcode & 0x0f00) >> 8]))
						v[0xf]=1; //carry
					else
						v[0xf]=0; //not carry
					
					v[(opcode & 0x0f00) >> 8] += v[(opcode & 0x00f0) >> 4];
					pc += 2;
					break;

				case 0x5:  //8XY5	Math	Vx -= Vy	VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
					if (v[(opcode & 0x00f0) >> 4] > v[(opcode & 0x0f00) >> 8])
						v[0xf] = 0; //borrow
					else
						v[0xf] = 1; //not borrow

					v[(opcode & 0x0f00) >> 8] -= v[(opcode & 0x00f0) >> 4];
					pc += 2;
					break;

				case 0x6:  //8XY6	BitOp	Vx>>=1	Stores the least significant bit of VX in VF and then shifts VX to the right by 1.[2]
					v[0xf] = (v[(opcode & 0x0f00) >> 8] & 0x1);
					v[(opcode & 0x0f00) >> 8] >>= 1;
					pc += 2;
					break;

				case 0x7: //8XY7	Math	Vx=Vy-Vx	Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
					if (v[(opcode & 0x0f00) >> 8]> v[(opcode & 0x00f0) >> 4])
						v[0xf] = 0; //borrow
					else
						v[0xf] = 1; //not borrow

					v[(opcode & 0x0f00) >> 8] = v[(opcode & 0x00f0) >> 4] - v[(opcode & 0x0f00) >> 8];
					pc += 2;
					break;

				case 0xe:  //8XYE	BitOp	Vx<<=1	Stores the most significant bit of VX in VF and then shifts VX to the left by 1.[3]
					v[0xf] = v[(opcode & 0x0f00) >> 8] >> 7;
					v[(opcode & 0x0f00) >> 8] <<= 1;
					pc += 2;
					break;
				default:
					cout << "Unknown opcode" << endl;
					break;
			}
			break;
		

		case 0x7000: //7XNN	Const	Vx += NN	Adds NN to VX. (Carry flag is not changed)
			v[(opcode & 0x0f00) >> 8] += opcode & 0x00ff;
			pc += 2;
			break;


		case 0x9000: //9XY0	Cond	if(Vx!=Vy)	Skips the next instruction if VX doesn't equal VY. (Usually the next instruction is a jump to skip a code block)
			if (v[(opcode & 0x0f00) >> 8] != v[(opcode & 0x00f0) >> 4])
				pc += 4;
			else
				pc += 2;
			break;
		

		case 0xa000: //ANNN	MEM	I = NNN	Sets I to the address NNN.
			I = opcode & 0x0fff;
			pc += 2;
			break;

		case 0xb000:  //BNNN	Flow	PC=V0+NNN	Jumps to the address NNN plus V0.
			pc = v[0x0] + (opcode & 0x0fff);
			break;

		case 0xc000: //CXNN	Rand	Vx=rand()&NN	Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
			v[(opcode & 0x0f00) >> 8] = (rand() % (0xFF + 1)) & (opcode & 0x00FF);
			pc += 2;
			break;

		case 0xd000: { //DXYN	Disp	draw(Vx,Vy,N)	Draws a sprite at coordinate (VX, VY) 
			unsigned short x = v[(opcode & 0x0f00) >> 8];
			unsigned short y = v[(opcode & 0x00f0) >> 4];
			unsigned short height = opcode & 0x000F;
			unsigned short pixel;

			v[0xF] = 0;

			for (int yline = 0; yline < height; yline++)
			{
				pixel = memory[I + yline];
				for (int xline = 0; xline < 8; xline++)
				{
					if ((pixel & (0x80 >> xline)) != 0)
					{
						if (gfx[(x + xline + ((y + yline) * 64))] == 1)
						{
							v[0xF] = 1;
						}
						gfx[x + xline + ((y + yline) * 64)] ^= 1;
					}
				}
			}

			drawflag = 1;
			pc += 2;
		}
			break;

		case 0xe000:
			switch (opcode & 0x000f) {
				case 0xe:  //EX9E	KeyOp	if(key()==Vx)	Skips the next instruction if the key stored in VX is pressed.
					if (key[v[(opcode & 0x0f00) >> 8]] != 0)
						pc += 4;
					else
						pc += 2;
					break;

				case 0x1: //EXA1	KeyOp	if(key()!=Vx)	Skips the next instruction if the key stored in VX isn't pressed.
					if (key[v[(opcode & 0x0f00) >> 8]] == 0)
						pc += 4;
					else
						pc += 2;
					break;

				default:
					cout << "Unknown opcode";
					break;

			}
			break;

		case 0xf000:
			switch (opcode & 0x00ff) {

				case 0x07:  //FX07	Timer	Vx = get_delay()	Sets VX to the value of the delay timer.
					v[(opcode & 0x0f00) >> 8] = delay_timer;
					pc += 2;
					break;

				case 0x0a: { //FX0A	KeyOp	Vx = get_key()	A key press is awaited, and then stored in VX.
					unsigned short kpress = 0;
					for (int i = 0; i < 16; i++) {
						if (key[i] != 0)
							kpress = key[i];
					}
					
					if (kpress != 0) {
						v[(opcode & 0x0f00) >> 8] = kpress;
						pc += 2;
					}
					
				}
					break;

				case 0x15:  //FX15	Timer	delay_timer(Vx)	Sets the delay timer to VX.
					delay_timer = v[(opcode & 0x0f00) >> 8];
					pc += 2;
					break;

				case 0x18:  //FX18	Sound	sound_timer(Vx)	Sets the sound timer to VX.
					sound_timer = v[(opcode & 0x0f00) >> 8];
					pc += 2;
					break;

				case 0x1e : //FX1E	MEM	I +=Vx	Adds VX to I.[4]
					I += v[(opcode & 0x0f00) >> 8];
					pc += 2;
					break;

				case 0x29: //FX29	MEM	I=sprite_addr[Vx]	Sets I to the location of the sprite for the character in VX.
					I = v[(opcode & 0x0f00) >> 8] * 0x5;
					pc += 2;
					break;

				case 0x33: //FX33	BCD	set_BCD(Vx);
					memory[I] = v[(opcode & 0x0f00) >> 8] / 100;
					memory[I + 1] = (v[(opcode & 0x0f00) >> 8] / 10) % 10;
					memory[I + 2] = (v[(opcode & 0x0f00) >> 8] % 100) % 10;
					pc += 2;
					break;

				case 0x55: // FX55	MEM	reg_dump(Vx,&I)	Stores V0 to VX (including VX) in memory starting at address I.
					for (int i = 0; i <= ((opcode & 0x0f00) >> 8); i++)
						memory[I + i] = v[i];

					pc += 2;
					break;

				case 0x65: //FX65	MEM	reg_load(Vx,&I)	Fills V0 to VX (including VX) with values from memory starting at address I.
					for (int i = 0; i <= ((opcode & 0x0f00) >> 8); i++)
						v[i] = memory[I + i];

					pc += 2;
					break;

				default:
					cout << "Unknown opcode";
					break;
			}
			break;

		default :
			cout << endl << "Unknown opcode" << endl;
			break;
		}  
			
		
		if (delay_timer > 0)
			--delay_timer;
		if (sound_timer == 1) {
			--sound_timer;  //TODO sound
			cout << endl << "BEEEEEEEEEEP" << endl;
		}
			
		


	


	}

};