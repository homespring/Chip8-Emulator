#ifndef CHIP8_H
#define CHIP8_H

#include <string>
#include <stack>
#include "SDL.h"

typedef unsigned char BYTE;
typedef unsigned short WORD;

class Chip8
{
private:
	std::string			rom_path;
	int					pixel_size		= 20;
	bool				loaded			= false;
	bool				video_ok		= false;
	bool				alive			= true;

	// zmienne dla systemu CHIP-8
	static const WORD	ram_size		= 0xFFF;
	static const BYTE	reg_size		= 16;
	static const WORD	game_start_addr = 0x200;
	static const int	width			= 64;
	static const int	height			= 32;
	static const int	keys_number		= 16;

	BYTE				game_memory[ram_size];
	BYTE				registers[reg_size];
	WORD				address_I;
	WORD				program_counter;
	std::stack<WORD>	stack;
	BYTE				screen[width][height];
	BYTE				key[keys_number];
	BYTE				delay_timer;
	BYTE				sound_timer;
	BYTE				digit_sprite_addr[0xF + 1];

	// rzeczy od SDLa
	SDL_Window*			win				= nullptr;
	SDL_Renderer*		renderer		= nullptr;

public:
	Chip8(std::string cfg_filepath = "");
	~Chip8();

	int game_loop();

private:
	void init();
	void init_digit_sprites();
	void init_display();
	void clear_display();
	void draw();
	WORD fetch_opcode();
	void decode_opcode(const WORD& opcode);
	BYTE wait_key();
	void decrement_timers();
	void read_keys();
	void sdl_events();

	// opcody

	void opcode_00E0();
	void opcode_00EE();
	void opcode_1nnn(const WORD& opcode);
	void opcode_2nnn(const WORD& opcode);
	void opcode_3xkk(const WORD& opcode);
	void opcode_4xkk(const WORD& opcode);
	void opcode_5xy0(const WORD& opcode);
	void opcode_6xkk(const WORD& opcode);
	void opcode_7xkk(const WORD& opcode);
	void opcode_8xy0(const WORD& opcode);
	void opcode_8xy1(const WORD& opcode);
	void opcode_8xy2(const WORD& opcode);
	void opcode_8xy3(const WORD& opcode);
	void opcode_8xy4(const WORD& opcode);
	void opcode_8xy5(const WORD& opcode);
	void opcode_8xy6(const WORD& opcode);
	void opcode_8xy7(const WORD& opcode);
	void opcode_8xyE(const WORD& opcode);
	void opcode_9xy0(const WORD& opcode);
	void opcode_Annn(const WORD& opcode);
	void opcode_Bnnn(const WORD& opcode);
	void opcode_Cxkk(const WORD& opcode);
	void opcode_Dxyn(const WORD& opcode);
	void opcode_Ex9E(const WORD& opcode);
	void opcode_ExA1(const WORD& opcode);
	void opcode_Fx07(const WORD& opcode);
	void opcode_Fx0A(const WORD& opcode);
	void opcode_Fx15(const WORD& opcode);
	void opcode_Fx18(const WORD& opcode);
	void opcode_Fx1E(const WORD& opcode);
	void opcode_Fx29(const WORD& opcode);
	void opcode_Fx33(const WORD& opcode);
	void opcode_Fx55(const WORD& opcode);
	void opcode_Fx65(const WORD& opcode);
};

#endif
