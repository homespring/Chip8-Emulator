#include "chip8.h"
#include <cstdlib> // rand

void Chip8::opcode_00E0()
{
	// wyczysc ekran (CLS)

	clear_display();
}

void Chip8::opcode_00EE()
{
	// powrot z funkcji (RET)

	program_counter = stack.top();
	stack.pop();
}

void Chip8::opcode_1nnn(const WORD& opcode)
{
	// skocz do lokacji nnn (JP addr)

	program_counter = opcode & 0x0FFF;
}

void Chip8::opcode_2nnn(const WORD& opcode)
{
	// wywolaj funkcje pod adresem nnn (CALL addr)

	stack.push(program_counter);
	program_counter = opcode & 0x0FFF;
}

void Chip8::opcode_3xkk(const WORD& opcode)
{
	// omin nastepna instrukcje jesli Vx == kk (SE Vx, byte)

	const int regx = (opcode & 0x0F00) >> 8; // wyluskujemy numer rejestru
	const int kk = (opcode & 0x00FF); // wyluskujemy wartosc do porownania

	if (registers[regx] == kk)
		program_counter += sizeof(WORD);
}

void Chip8::opcode_4xkk(const WORD& opcode)
{
	// omin nastepna instrukcje jesli Vx != kk (SNE Vx, byte)

	const int regx = (opcode & 0x0F00) >> 8; // wyluskujemy numer rejestru
	const int kk = (opcode & 0x00FF); // wyluskujemy wartosc do porownania

	if (registers[regx] != kk)
		program_counter += sizeof(WORD);
}

void Chip8::opcode_5xy0(const WORD& opcode)
{
	// omin nastepna instrukcje jesli Vx == Vy (SE Vx, Vy)

	const int regx = (opcode & 0x0F00) >> 8;
	const int regy = (opcode & 0x00F0) >> 4;

	if(registers[regx] == registers[regy])
		program_counter += sizeof(WORD);
}

void Chip8::opcode_6xkk(const WORD& opcode)
{
	// zapisz wartosc kk do rejestru Vx (LD Vx, byte)

	const int regx = (opcode & 0x0F00) >> 8;
	const int kk = (opcode & 0x00FF);

	registers[regx] = kk;
}

void Chip8::opcode_7xkk(const WORD& opcode)
{
	// dodaj wartosc kk do rejestru Vx (ADD Vx, byte)

	const int regx = (opcode & 0x0F00) >> 8;
	const int kk = (opcode & 0x00FF);

	registers[regx] += kk;
}

void Chip8::opcode_8xy0(const WORD& opcode)
{
	// zapisz do rejestru Vx wartosc rejestru Vy (LD Vx, Vy)

	const int regx = (opcode & 0x0F00) >> 8;
	const int regy = (opcode & 0x00F0) >> 4;

	registers[regx] = registers[regy];
}

void Chip8::opcode_8xy1(const WORD& opcode)
{
	// zapisz do rejestru Vx wynik operacji Vx OR Vy (OR Vx, Vy)

	const int regx = (opcode & 0x0F00) >> 8;
	const int regy = (opcode & 0x00F0) >> 4;

	registers[regx] |= registers[regy];
}

void Chip8::opcode_8xy2(const WORD& opcode)
{
	// zapisz do rejestru Vx wynik operacji Vx AND Vy (AND Vx, Vy)

	const int regx = (opcode & 0x0F00) >> 8;
	const int regy = (opcode & 0x00F0) >> 4;

	registers[regx] &= registers[regy];
}

void Chip8::opcode_8xy3(const WORD& opcode)
{
	// zapisz do rejestru Vx wynik operacji Vx XOR Vy (XOR Vx, Vy)

	const int regx = (opcode & 0x0F00) >> 8;
	const int regy = (opcode & 0x00F0) >> 4;

	registers[regx] ^= registers[regy];
}

void Chip8::opcode_8xy4(const WORD& opcode)
{
	// dodaj rejestry Vx i Vy a wynik zapisz w Vx. Ustaw flage VF gdy wynik > 255 (ADD Vx, Vy)

	const int regx = (opcode & 0x0F00) >> 8;
	const int regy = (opcode & 0x00F0) >> 4;
	const int sum = registers[regx] + registers[regy];

	if (sum > 0xFF)
		registers[0xF] = 1;
	else
		registers[0xF] = 0;

	registers[regx] = sum & 0xFF;
}

void Chip8::opcode_8xy5(const WORD& opcode)
{
	// odejmij od rejestru Vx rejestr Vy a wynik zapisz w Vx. Ustaw flage VF gdy Vx > Vy (SUB Vx, Vy)

	const int regx = (opcode & 0x0F00) >> 8;
	const int regy = (opcode & 0x00F0) >> 4;
	const int diff = registers[regx] - registers[regy];

	if (registers[regx] > registers[regy])
		registers[0xF] = 1;
	else
		registers[0xF] = 0;

	registers[regx] = diff & 0xFF;
}

void Chip8::opcode_8xy6(const WORD& opcode)
{
	// VF = najmniej znaczacy bit Vx, Vx >> 1

	const int regx = (opcode & 0x0F00) >> 8;

	registers[0xF] = registers[regx] & 0x01;
	registers[regx] >>= 1;
}

void Chip8::opcode_8xy7(const WORD& opcode)
{
	// odejmij od rejestru Vy rejestr Vx a wynik zapisz w Vx. Ustaw flage VF gdy Vy > Vx (SUBN Vx, Vy)

	const int regx = (opcode & 0x0F00) >> 8;
	const int regy = (opcode & 0x00F0) >> 4;
	const int diff = registers[regy] - registers[regx];

	if (registers[regy] > registers[regx])
		registers[0xF] = 1;
	else
		registers[0xF] = 0;

	registers[regx] = diff & 0xFF;
}

void Chip8::opcode_8xyE(const WORD& opcode)
{
	// VF = najbardziej znaczacy bit Vx, Vx << 1

	const int regx = (opcode & 0x0F00) >> 8;

	registers[0xF] = registers[regx] & 0x80;
	registers[regx] <<= 1;
}

void Chip8::opcode_9xy0(const WORD& opcode)
{
	// omin nastepna instrukcje gdy Vx != Vy (SNE Vx, Vy)

	const int regx = (opcode & 0x0F00) >> 8;
	const int regy = (opcode & 0x00F0) >> 4;

	if (registers[regx] != registers[regy])
		program_counter += sizeof(WORD);
}

void Chip8::opcode_Annn(const WORD& opcode)
{
	// zapisz nnn do rejestru I (LD I, addr)

	address_I = opcode & 0x0FFF;
}

void Chip8::opcode_Bnnn(const WORD& opcode)
{
	// skocz do lokacji nnn + V0 (JP V0, addr)

	program_counter = (opcode & 0x0FFF) + registers[0];
}

void Chip8::opcode_Cxkk(const WORD& opcode)
{
	// Vx = (random byte) AND kk

	const int regx = (opcode & 0x0F00) >> 8;
	const int kk = (opcode & 0x00FF);
	const int rb = rand() % 256;

	registers[regx] = (rb & kk) & 0xFF;
}

void Chip8::opcode_Dxyn(const WORD& opcode)
{
	// wyswietl n-bajtowego sprite'a zaczynajac od pamieci wskazywanej przez rejestr I w punkcie (Vx, Vy). Ustaw VF, gdy jakis pixel zmienia stan z 1 na 0

	const int regx = (opcode & 0x0F00) >> 8;
	const int regy = (opcode & 0x00F0) >> 4;
	const int n = opcode & 0x000F;

	registers[0xF] = 0;

	for (int row = 0; row < n; row++)
	{
		const BYTE sprite = game_memory[address_I + row];

		for (int col = 0; col < 8; col++)
		{
			// XOR-ujemy tylko ustawione pixele sprite'a
			if (sprite & (1 << (7 - col)))
			{
				const int x = (registers[regx] + col) % width;
				const int y = (registers[regy] + row) % height;

				// test na zmiane stanu z 1 na 0
				if (screen[x][y])
					registers[0xF] = 1;
				
				// zmiana stanu piksela ekranu
				screen[x][y] ^= 1;
			}
		}
	}
}

void Chip8::opcode_Ex9E(const WORD& opcode)
{
	// omin nastepna instrukcje jesli klawisz o numerze Vx jest wcisniety (SKP Vx)

	const int regx = (opcode & 0x0F00) >> 8;

	if (key[registers[regx]])
		program_counter += sizeof(WORD);
}

void Chip8::opcode_ExA1(const WORD& opcode)
{
	// omin nastepna instrukcje jesli klawisz o numerze Vx nie jest wcisniety (SKNP Vx)

	const int regx = (opcode & 0x0F00) >> 8;

	if (!key[registers[regx]])
		program_counter += sizeof(WORD);
}

void Chip8::opcode_Fx07(const WORD& opcode)
{
	// ustaw Vx = wartosc delay timera (LD Vx, DT)

	const int regx = (opcode & 0x0F00) >> 8;

	registers[regx] = delay_timer;
}

void Chip8::opcode_Fx0A(const WORD& opcode)
{
	// czekaj az do wcisniecia klawisza, nastepnie ustaw Vx = numer wcisnietego klawisza (LD Vx, K)

	const int regx = (opcode & 0x0F00) >> 8;

	registers[regx] = wait_key();
}

void Chip8::opcode_Fx15(const WORD& opcode)
{
	// ustaw wartosc delay timera = Vx (LD DT, Vx)

	const int regx = (opcode & 0x0F00) >> 8;

	delay_timer = registers[regx];
}

void Chip8::opcode_Fx18(const WORD& opcode)
{
	// ustaw wartosc sound timera = Vx (LD ST, Vx)

	const int regx = (opcode & 0x0F00) >> 8;

	sound_timer = registers[regx];
}

void Chip8::opcode_Fx1E(const WORD& opcode)
{
	// zwieksz wartosc rejestru I o wartosc rejestru Vx (ADD I, Vx)

	const int regx = (opcode & 0x0F00) >> 8;

	address_I += registers[regx];
}

void Chip8::opcode_Fx29(const WORD& opcode)
{
	// zapisz do rejestru I adres sprite'a dla cyfry wskazywanej przez wartosc Vx (LD F, Vx)

	const int regx = (opcode & 0x0F00) >> 8;

	address_I = digit_sprite_addr[registers[regx]];
}

void Chip8::opcode_Fx33(const WORD& opcode)
{
	// zapisz Vx za pomoca reprezentacji BCD w pamieci pod adresami I, I+1, I+2

	const int regx = (opcode & 0x0F00) >> 8;

	game_memory[address_I] = registers[regx] / 100;
	game_memory[address_I + 1] = (registers[regx] / 10) % 10;
	game_memory[address_I + 2] = registers[regx] % 10;
}

void Chip8::opcode_Fx55(const WORD& opcode)
{
	// kopiuj wartosci od V0 do Vx lacznie do pamieci zaczynajac od adresu I

	const int regx = (opcode & 0x0F00) >> 8;

	for (int i = 0; i <= regx; i++)
	{
		address_I += i;
		game_memory[address_I] = registers[i];
	}
}

void Chip8::opcode_Fx65(const WORD& opcode)
{
	// wypelnij rejestry od V0 do Vx lacznie wartosciami zaczynajac od adresu I

	const int regx = (opcode & 0x0F00) >> 8;

	for (int i = 0; i <= regx; i++)
	{
		address_I += i;
		registers[i] = game_memory[address_I];
	}
}
