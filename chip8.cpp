#define _CRT_SECURE_NO_WARNINGS

#include "chip8.h"
#include "INIReader.h"
#include <iostream>
#include <fstream>
#include <iomanip> // cout hex value
#include <cstdlib> // srand
#include <ctime> // time
#include "elapsedtimer.h"

using namespace std;

Chip8::Chip8(std::string cfg_filepath)
{
	INIReader cfg(cfg_filepath);

	if (cfg.ParseError() == 0)
	{
		// tutaj mamy poprawnie wczytany plik konfiguracyjny

		rom_path = cfg.Get("", "rom_path", rom_path);
		pixel_size = static_cast<int>(cfg.GetInteger("", "pixel_size", pixel_size));
	}

	srand(static_cast<unsigned int>(time(nullptr)));

	init_display();
	init();
}

Chip8::~Chip8()
{
	if (renderer)
		SDL_DestroyRenderer(renderer);

	if (win)
		SDL_DestroyWindow(win);

	SDL_Quit();
}

int Chip8::game_loop()
{
	if (!video_ok)
	{
		cerr << "Couldn't initialize video! Quitting...\n";
		getchar();
		return -1;
	}

	if (!loaded)
	{
		cerr << "Game file not loaded! Quitting...\n";
		getchar();
		return -1;
	}

	cout << "\nStarting the game...\n\n";

	WORD opcode = 0;
	ElapsedTimer tim(1000 / 60);
	
	while (alive)
	{
		opcode = fetch_opcode();
		decode_opcode(opcode);

		if (tim.elapsed())
		{
			draw();
			decrement_timers();
			read_keys();
			tim.tic();
		}

		sdl_events();

		SDL_Delay(2);
	}

	cout << "Game Over.\n";
	
	return 0;
}

void Chip8::init()
{
	// wyzerowanie pamieci przeznaczonej na gre
	memset(game_memory, 0, ram_size);

	// wyzerowanie rejestrow
	memset(registers, 0, reg_size);

	// wyzerowanie rejestru I
	address_I = 0;

	// ustawienie zmiennej program_counter na adres poczatku gry
	program_counter = game_start_addr;

	// czyscimy stos
	while(!stack.empty())
		stack.pop();

	// czyscimy ekran
	clear_display();

	// resetujemy stan klawiatury
	memset(key, 0, keys_number);

	// zerujemy timery
	delay_timer = 0;
	sound_timer = 0;

	// inicjalizujemy sprite'y cyfr w pamieci gry
	init_digit_sprites();

	// wczytujemy plik z gra

	ifstream file(rom_path, ifstream::binary);

	if (file)
	{
		cout << "Reading file " << rom_path << endl;

		// wczytujemy rozmiar pliku
		file.seekg(0, file.end);
		const auto len = file.tellg();
		file.seekg(0, file.beg);

		cout << "File size is " << len << " bytes.\n";

		if (len > (ram_size - game_start_addr))
		{
			cerr << "Game file is too big!\n";
			loaded = false;
			file.close();
			return;
		}

		file.read((char*)&game_memory[game_start_addr], len);

		if (file)
		{
			cout << "File loaded successfully.\n";
			loaded = true;
		}
		else
		{
			cerr << "Error: loaded only " << file.gcount() << " bytes.\n";
			loaded = false;
		}

		file.close();
	}
	else
	{
		cerr << "Can't read file " << rom_path << endl;
		loaded = false;
	}
}

void Chip8::init_digit_sprites()
{
	int cur_addr = 0; // wypelniamy pamiec gry od samego poczatku

	// "0"
	digit_sprite_addr[0] = cur_addr;
	game_memory[cur_addr++] = 0xF0;
	game_memory[cur_addr++] = 0x90;
	game_memory[cur_addr++] = 0x90;
	game_memory[cur_addr++] = 0x90;
	game_memory[cur_addr++] = 0xF0;

	// "1"
	digit_sprite_addr[1] = cur_addr;
	game_memory[cur_addr++] = 0x20;
	game_memory[cur_addr++] = 0x60;
	game_memory[cur_addr++] = 0x20;
	game_memory[cur_addr++] = 0x20;
	game_memory[cur_addr++] = 0x70;

	// "2"
	digit_sprite_addr[2] = cur_addr;
	game_memory[cur_addr++] = 0xF0;
	game_memory[cur_addr++] = 0x10;
	game_memory[cur_addr++] = 0xF0;
	game_memory[cur_addr++] = 0x80;
	game_memory[cur_addr++] = 0xF0;

	// "3"
	digit_sprite_addr[3] = cur_addr;
	game_memory[cur_addr++] = 0xF0;
	game_memory[cur_addr++] = 0x10;
	game_memory[cur_addr++] = 0xF0;
	game_memory[cur_addr++] = 0x10;
	game_memory[cur_addr++] = 0xF0;

	// "4"
	digit_sprite_addr[4] = cur_addr;
	game_memory[cur_addr++] = 0x90;
	game_memory[cur_addr++] = 0x90;
	game_memory[cur_addr++] = 0xF0;
	game_memory[cur_addr++] = 0x10;
	game_memory[cur_addr++] = 0x10;

	// "5"
	digit_sprite_addr[5] = cur_addr;
	game_memory[cur_addr++] = 0xF0;
	game_memory[cur_addr++] = 0x80;
	game_memory[cur_addr++] = 0xF0;
	game_memory[cur_addr++] = 0x10;
	game_memory[cur_addr++] = 0xF0;

	// "6"
	digit_sprite_addr[6] = cur_addr;
	game_memory[cur_addr++] = 0xF0;
	game_memory[cur_addr++] = 0x80;
	game_memory[cur_addr++] = 0xF0;
	game_memory[cur_addr++] = 0x90;
	game_memory[cur_addr++] = 0xF0;

	// "7"
	digit_sprite_addr[7] = cur_addr;
	game_memory[cur_addr++] = 0xF0;
	game_memory[cur_addr++] = 0x10;
	game_memory[cur_addr++] = 0x20;
	game_memory[cur_addr++] = 0x40;
	game_memory[cur_addr++] = 0x40;

	// "8"
	digit_sprite_addr[8] = cur_addr;
	game_memory[cur_addr++] = 0xF0;
	game_memory[cur_addr++] = 0x90;
	game_memory[cur_addr++] = 0xF0;
	game_memory[cur_addr++] = 0x90;
	game_memory[cur_addr++] = 0xF0;

	// "9"
	digit_sprite_addr[9] = cur_addr;
	game_memory[cur_addr++] = 0xF0;
	game_memory[cur_addr++] = 0x90;
	game_memory[cur_addr++] = 0xF0;
	game_memory[cur_addr++] = 0x10;
	game_memory[cur_addr++] = 0xF0;

	// "A"
	digit_sprite_addr[0xA] = cur_addr;
	game_memory[cur_addr++] = 0xF0;
	game_memory[cur_addr++] = 0x90;
	game_memory[cur_addr++] = 0xF0;
	game_memory[cur_addr++] = 0x90;
	game_memory[cur_addr++] = 0x90;

	// "B"
	digit_sprite_addr[0xB] = cur_addr;
	game_memory[cur_addr++] = 0xE0;
	game_memory[cur_addr++] = 0x90;
	game_memory[cur_addr++] = 0xE0;
	game_memory[cur_addr++] = 0x90;
	game_memory[cur_addr++] = 0xE0;

	// "C"
	digit_sprite_addr[0xC] = cur_addr;
	game_memory[cur_addr++] = 0xF0;
	game_memory[cur_addr++] = 0x80;
	game_memory[cur_addr++] = 0x80;
	game_memory[cur_addr++] = 0x80;
	game_memory[cur_addr++] = 0xF0;

	// "D"
	digit_sprite_addr[0xD] = cur_addr;
	game_memory[cur_addr++] = 0xE0;
	game_memory[cur_addr++] = 0x90;
	game_memory[cur_addr++] = 0x90;
	game_memory[cur_addr++] = 0x90;
	game_memory[cur_addr++] = 0xE0;

	// "E"
	digit_sprite_addr[0xE] = cur_addr;
	game_memory[cur_addr++] = 0xF0;
	game_memory[cur_addr++] = 0x80;
	game_memory[cur_addr++] = 0xF0;
	game_memory[cur_addr++] = 0x80;
	game_memory[cur_addr++] = 0xF0;

	// "F"
	digit_sprite_addr[0xF] = cur_addr;
	game_memory[cur_addr++] = 0xF0;
	game_memory[cur_addr++] = 0x80;
	game_memory[cur_addr++] = 0xF0;
	game_memory[cur_addr++] = 0x80;
	game_memory[cur_addr++] = 0x80;
}

void Chip8::init_display()
{
	video_ok = false;

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		return;

	win = SDL_CreateWindow(
		"CHIP-8 Emulator",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width * pixel_size,
		height * pixel_size,
		SDL_WINDOW_SHOWN);

	if (!win)
	{
		cerr << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
		return;
	}

	renderer = SDL_CreateRenderer(
		win,
		-1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (!renderer)
	{
		cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << endl;
		return;
	}

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
	
	video_ok = true;
}

void Chip8::clear_display()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			screen[x][y] = 0;
		}
	}
}

void Chip8::draw()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	SDL_Rect r;
	r.w = pixel_size;
	r.h = pixel_size;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if (screen[x][y])
			{
				r.x = x * pixel_size;
				r.y = y * pixel_size;

				SDL_RenderFillRect(renderer, &r);
			}
		}
	}

	SDL_RenderPresent(renderer);
}

WORD Chip8::fetch_opcode()
{
	WORD ret = game_memory[program_counter++] << 8;
	ret |= game_memory[program_counter++];

	return ret;
}

void Chip8::decode_opcode(const WORD& opcode)
{
	switch (opcode & 0xF000)
	{
	case 0x0000:
	{
		switch (opcode & 0x00FF)
		{
		case 0x00E0: opcode_00E0(); break;
		case 0x00EE: opcode_00EE(); break;
		default:
			cout << "Warning: unexpected opcode 0x" << hex << uppercase << opcode << dec << endl;
			break;
		}
	}
		break;
	case 0x1000: opcode_1nnn(opcode); break;
	case 0x2000: opcode_2nnn(opcode); break;
	case 0x3000: opcode_3xkk(opcode); break;
	case 0x4000: opcode_4xkk(opcode); break;
	case 0x5000: opcode_5xy0(opcode); break;
	case 0x6000: opcode_6xkk(opcode); break;
	case 0x7000: opcode_7xkk(opcode); break;
	case 0x8000:
	{
		switch (opcode & 0x000F)
		{
		case 0x0000: opcode_8xy0(opcode); break;
		case 0x0001: opcode_8xy1(opcode); break;
		case 0x0002: opcode_8xy2(opcode); break;
		case 0x0003: opcode_8xy3(opcode); break;
		case 0x0004: opcode_8xy4(opcode); break;
		case 0x0005: opcode_8xy5(opcode); break;
		case 0x0006: opcode_8xy6(opcode); break;
		case 0x0007: opcode_8xy7(opcode); break;
		case 0x000E: opcode_8xyE(opcode); break;
		default:
			cout << "Warning: unexpected opcode 0x" << hex << uppercase << opcode << dec << endl;
			break;
		}
	}
		break;
	case 0x9000: opcode_9xy0(opcode); break;
	case 0xA000: opcode_Annn(opcode); break;
	case 0xB000: opcode_Bnnn(opcode); break;
	case 0xC000: opcode_Cxkk(opcode); break;
	case 0xD000: opcode_Dxyn(opcode); break;
	case 0xE000:
	{
		switch (opcode & 0x00FF)
		{
		case 0x009E: opcode_Ex9E(opcode); break;
		case 0x00A1: opcode_ExA1(opcode); break;
		default:
			cout << "Warning: unexpected opcode 0x" << hex << uppercase << opcode << dec << endl;
			break;
		}
	}
		break;
	case 0xF000:
	{
		switch (opcode & 0x00FF)
		{
		case 0x0007: opcode_Fx07(opcode); break;
		case 0x000A: opcode_Fx0A(opcode); break;
		case 0x0015: opcode_Fx15(opcode); break;
		case 0x0018: opcode_Fx18(opcode); break;
		case 0x001E: opcode_Fx1E(opcode); break;
		case 0x0029: opcode_Fx29(opcode); break;
		case 0x0033: opcode_Fx33(opcode); break;
		case 0x0055: opcode_Fx55(opcode); break;
		case 0x0065: opcode_Fx65(opcode); break;
		default:
			cout << "Warning: unexpected opcode 0x" << hex << uppercase << opcode << dec << endl;
			break;
		}
	}
		break;
	default:
		cout << "Warning: unknown opcode 0x" << hex << uppercase << opcode << dec << endl;
		break; // nieobslugiwany opcode
	}
}

BYTE Chip8::wait_key()
{
	while (alive)
	{
		read_keys();

		for (BYTE i = 0; i < keys_number; i++)
		{
			if (key[i])
				return i;
		}

		sdl_events();
		SDL_Delay(50);
	}

	return 0x0;
}

void Chip8::decrement_timers()
{
	if (delay_timer > 0)
		delay_timer--;

	if (sound_timer > 0)
		sound_timer--;
}

void Chip8::read_keys()
{
	const Uint8* kb = SDL_GetKeyboardState(nullptr);

	key[0] = kb[SDL_SCANCODE_KP_0] ? 1 : 0;
	key[1] = kb[SDL_SCANCODE_KP_1] ? 1 : 0;
	key[2] = kb[SDL_SCANCODE_KP_2] ? 1 : 0;
	key[3] = kb[SDL_SCANCODE_KP_3] ? 1 : 0;
	key[4] = kb[SDL_SCANCODE_KP_4] ? 1 : 0;
	key[5] = kb[SDL_SCANCODE_KP_5] ? 1 : 0;
	key[6] = kb[SDL_SCANCODE_KP_6] ? 1 : 0;
	key[7] = kb[SDL_SCANCODE_KP_7] ? 1 : 0;
	key[8] = kb[SDL_SCANCODE_KP_8] ? 1 : 0;
	key[9] = kb[SDL_SCANCODE_KP_9] ? 1 : 0;
	key[0xA] = kb[SDL_SCANCODE_A] ? 1 : 0;
	key[0xB] = kb[SDL_SCANCODE_B] ? 1 : 0;
	key[0xC] = kb[SDL_SCANCODE_C] ? 1 : 0;
	key[0xD] = kb[SDL_SCANCODE_D] ? 1 : 0;
	key[0xE] = kb[SDL_SCANCODE_E] ? 1 : 0;
	key[0xF] = kb[SDL_SCANCODE_F] ? 1 : 0;
	alive = kb[SDL_SCANCODE_ESCAPE] ? false : true;
}

void Chip8::sdl_events()
{
	static SDL_Event evt;

	while (SDL_PollEvent(&evt))
	{
		switch (evt.type)
		{
		case SDL_QUIT:
			alive = false;
			break;
		default:
			break;
		}
	}
}
