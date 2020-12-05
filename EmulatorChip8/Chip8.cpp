/*
	CHIP-8 Emulator

	Copyright (C) 2020 Maoliang Li

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see https://www.gnu.org/licenses/
*/
#include <cstring>
#include "Chip8.h"

void Chip8::initialize() {
	PC = 0x200;
	IR = 0;
	SP = 0;
	memset(memory, 0, sizeof(memory));
	memset(V, 0, sizeof(V));
	memset(screen, 0, sizeof(screen));
	memset(keys, 0, sizeof(keys));
	memset(stack, 0, sizeof(stack));
	memset(keymap, 0, sizeof(keymap));
	memcpy(memory, chip8_fontset, 80);
	timer_delay = timer_sound = 0;
	draw_flag = true;
	err_flag = false;
	srand(time(NULL)); // prepare for the random instruction

	keymap['1'] = 0x1;
	keymap['2'] = 0x2;
	keymap['3'] = 0x3;
	keymap['4'] = 0xC;

	keymap['q'] = keymap['Q'] = 0x4;
	keymap['w'] = keymap['W'] = 0x5;
	keymap['e'] = keymap['E'] = 0x6;
	keymap['r'] = keymap['R'] = 0xD;

	keymap['a'] = keymap['A'] = 0x7;
	keymap['s'] = keymap['S'] = 0x8;
	keymap['d'] = keymap['D'] = 0x9;
	keymap['f'] = keymap['F'] = 0xE;

	keymap['z'] = keymap['Z'] = 0xA;
	keymap['x'] = keymap['X'] = 0x0;
	keymap['c'] = keymap['C'] = 0xB;
	keymap['v'] = keymap['V'] = 0xF;
}

void Chip8::load_code(const LPBYTE code_buffer, const size_t buffer_size) {
	memcpy(memory + 0x200, code_buffer, buffer_size);
}

void Chip8::reset()
{
	PC = 0x200;
	IR = 0;
	SP = 0;
	memset(V, 0, sizeof(V));
	memset(screen, 0, sizeof(screen));
	memset(keys, 0, sizeof(keys));
	memset(stack, 0, sizeof(stack));
	memset(keymap, 0, sizeof(keymap));
	memcpy(memory, chip8_fontset, 80);
	timer_delay = timer_sound = 0;
	draw_flag = true;
	err_flag = false;
	srand(time(NULL));
}

void Chip8::emulate_cycle()
{
	draw_flag = false;
	op = memory[PC] << 8 | memory[PC + 1];
	switch (op & 0xF000)
	{
	case 0x0000:  // IR just ignored the 0x0NNN op
		switch (op & 0x000F)
		{
		case 0x0000: // 0x00E0 Clear screen
			_clear_screen();
			draw_flag = true;
			break;
		case 0x000E: // 0x00EE subroutine return
			PC = stack[--SP]; // remember that sp refers to above the top
			break;
		default:
			_error_op(0x0000);
			break;
		}
		PC += 2; // remember to continue with the next op
		break;
	case 0x1000:
		PC = op & 0x0FFF; // we shouldn't add 2 to PC here		
		break;
	case 0x2000: // Call subroutine
		stack[SP++] = PC;
		PC = op & 0x0FFF;
		break;
	case 0x3000: // skip the next inst if V[X] == NN
		if (V[BIT2(op)] == (op & 0x00FF)) {
			PC += 2;
		}
		PC += 2;
		break;
	case 0x4000: // just contrary to the last op
		if (V[BIT2(op)] != (op & 0x00FF)) {
			PC += 2;
		}
		PC += 2;
		break;
	case 0x5000:
		if (V[BIT2(op)] == V[BIT1(op)]) {
			PC += 2;
		}
		PC += 2;
		break;
	case 0x6000:
		V[BIT2(op)] = op & 0x00FF;
		PC += 2;
		break;
	case 0x7000: // No carry flag add
		V[BIT2(op)] += op & 0x00FF;
		PC += 2;
		break;
	case 0x8000:
		switch (op & 0x000F) 
		{
		case 0x0:
			V[BIT2(op)] = V[BIT1(op)];
			PC += 2;
			break;
		case 0x1:
			V[BIT2(op)] |= V[BIT1(op)];
			PC += 2;
			break;
		case 0x2:
			V[BIT2(op)] &= V[BIT1(op)];
			PC += 2;
			break;
		case 0x3:
			V[BIT2(op)] ^= V[BIT1(op)];
			PC += 2;
			break;
		case 0x4:
			if (V[BIT1(op)] > (0xFF - V[BIT2(op)])) // overflow, so set CF
				V[0xF] = 1;
			else
				V[0xF] = 0;
			V[BIT2(op)] += V[BIT1(op)];
			PC += 2;
			break;
		case 0x5:
			if (V[BIT1(op)] > V[BIT2(op)]) // overflow, so set CF
				V[0xF] = 0;
			else
				V[0xF] = 1;
			V[BIT2(op)] -= V[BIT1(op)];
			PC += 2;
			break;
		case 0x6: // more info => refer to the specification
			V[0xF] = V[BIT2(op)] & 0x1;
			V[BIT2(op)] >>= 1;
			PC += 2;
			break;
		case 0x7: // more info => refer to the specification
			if (V[BIT2(op)] > V[BIT1(op)]) // overflow, so set CF
				V[0xF] = 0;
			else
				V[0xF] = 1;
			V[BIT2(op)] = V[BIT1(op)] - V[BIT2(op)];
			PC += 2;			
			break;
		case 0xE:
			V[0xF] = V[BIT2(op)] >> 7;
			V[BIT2(op)] <<= 1;
			PC += 2;
			break;
		default:
			_error_op(0x8000);
			break;
		}
		break;
	case 0x9000:
		if (V[BIT2(op)] != V[BIT1(op)]) {
			PC += 2;
		}
		PC += 2;
		break;
	case 0xA000:
		IR = op & 0x0FFF;
		PC += 2;
		break;
	case 0xB000:
		PC = (op & 0x0FFF) + V[0];
		break;
	case 0xC000:
		V[BIT2(op)] = (rand() % 0xFF) & (op & 0x00FF);
		PC += 2;
		break;
	case 0xD000: {
		WORD x = V[BIT2(op)];
		WORD y = V[BIT1(op)];
		WORD h = BIT0(op);
		WORD pix;

		V[0xF] = 0;
		for (int i = 0; i < h; ++i) {
			pix = memory[IR + i];
			for (int j = 0; j < 8; ++j) {
				if (pix & (0x80 >> j)) { // check the status of each bit
					if (screen[x + j][y + i] == 1)
						V[0xF] = 1;
					screen[x + j][y + i] ^= 1;
				}
			}
		}
		draw_flag = true;
		PC += 2;
		break;
	}
	case 0xE000:
		switch (op & 0x00FF) // note that here is the last two bits
		{
		case 0x009E: // EX9E: Skips the next instruction if the key stored in VX is pressed
			if (keys[V[BIT2(op)]] != 0) {
				PC += 2;
			}
			PC += 2;
			break;

		case 0x00A1: // EXA1: Skips the next instruction if the key stored in VX isn't pressed
			if (keys[V[BIT2(op)]] == 0) {
				PC += 2;
			}
			PC += 2;
			break;

		default:
			_error_op(0xE000);
		}
		break;
	case 0xF000:
		switch (op & 0x00FF) // note that here is the last two bits
		{
		case 0x0007: // FX07: Sets VX to the value of the delay timer
			V[BIT2(op)] = timer_delay;
			PC += 2;
			break;

		case 0x000A: // FX0A: A key press is awaited, and then stored in VX		
		{
			BOOL keyPress = false;
			for (int i = 0; i < 16; ++i)
			{
				if (keys[i] != 0)
				{
					V[BIT2(op)] = i;
					keyPress = true;
				}
			}
			// If we didn't received a keypress, skip this cycle and try again.
			//(Blocking Operation. All instruction halted until next key event)
			if (!keyPress) return;
			PC += 2;
			break;
		}

		case 0x0015: // FX15: Sets the delay timer to VX
			timer_delay = V[BIT2(op)];
			PC += 2;
			break;

		case 0x0018: // FX18: Sets the sound timer to VX
			timer_sound = V[BIT2(op)];
			PC += 2;
			break;

		case 0x001E: // FX1E: Adds VX to IR
			if (IR + V[BIT2(op)] > 0xFFF)	// VF is set to 1 when range overflow
				V[0xF] = 1;
			else
				V[0xF] = 0;
			IR += V[BIT2(op)];
			PC += 2;
			break;

		case 0x0029: // FX29: Sets IR to the location of the sprite for the character in VX
			IR = V[BIT2(op)] * 0x5;
			PC += 2;
			break;

		case 0x0033: // FX33: Stores the Binary-coded decimal representation of VX at the addresses IR, IR plus 1, and IR plus 2
			memory[IR] = V[BIT2(op)] / 100;
			memory[IR + 1] = (V[BIT2(op)] / 10) % 10;
			memory[IR + 2] = (V[BIT2(op)] % 100) % 10;
			PC += 2;
			break;

		case 0x0055: // FX55: Stores V0 to VX in memory starting at address IR					
			for (int i = 0; i <= (BIT2(op)); ++i)
				memory[IR + i] = V[i];
			// On the original interpreter, when the operation is done, IR = IR + X + 1.
			IR += (BIT2(op)) + 1;
			PC += 2;
			break;

		case 0x0065: // FX65: Fills V0 to VX with values from memory starting at address IR					
			for (int i = 0; i <= (BIT2(op)); ++i)
				V[i] = memory[IR + i];
			// On the original interpreter, when the operation is done, IR = IR + X + 1.
			IR += (BIT2(op)) + 1;
			PC += 2;
			break;

		default:
			_error_op(0xF000);
		}
		break;
	default:
		_error_op(0xCAFE);
		break;
	}

	if (timer_delay > 0) timer_delay--;
	if (timer_sound > 0) {
		if (timer_sound == 1) {
			_beep();
		}
		timer_sound--;
	}
}

LPBYTE load_application(const std::string& filename, int & filesize) {
	std::cout << "Loading: " << filename << "..." << std::endl;
	std::ifstream ifs;
	ifs.open(filename, std::ios::binary | std::ios::in);
	if (!ifs.is_open()) {
		std::cerr << "Failed to open the file." << std::endl;
		return nullptr;
	}
	ifs.seekg(0, std::ios::end);
	filesize = ifs.tellg();
	std::cout << "File size: " << filesize << std::endl;
	ifs.seekg(0, std::ios::beg);
	LPBYTE buffer = new BYTE[filesize];
	if (buffer == nullptr) {
		std::cerr << "Failed to allocate memory." << std::endl;
		return nullptr;
	}
	
	ifs.read(reinterpret_cast<char *>(buffer), filesize);
	if (ifs.bad() && !ifs.eof()) {
		std::cerr << "Warning: Error may occur in the readin process." << std::endl;
	}
	ifs.close();
	if (filesize >= 4096 - 512) {
		std::cerr << "Error: ROM too large for memory." << std::endl;
		return nullptr;
	}
	std::cout << "ROM loaded." << std::endl;
	return buffer;
}