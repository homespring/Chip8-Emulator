#include "chip8.h"
#include "vld.h"

int main(int argc, char *argv[])
{
	Chip8 emu("../settings.ini");

	return emu.game_loop();
}
