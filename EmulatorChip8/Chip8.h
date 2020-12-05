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
#pragma once

#include <windows.h>

#include <iostream>
#include <fstream>
#include <string>

#define BIT0(op) (op & 0x000F)
#define BIT1(op) ((op & 0x00F0) >> 4)
#define BIT2(op) ((op & 0x0F00) >> 8)

static BYTE chip8_fontset[80] 
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

LPBYTE load_application(const std::string& filename, int& filesize);

class Chip8 {
public:
	Chip8() : draw_flag(false), err_flag(false),
		timer_delay(0), timer_sound(0), IR(0), PC(0x200), SP(0), op(0) {}
	~Chip8() {}
	void initialize();
	void load_code(const LPBYTE code_buffer, const size_t buffer_size);
	void emulate_cycle();
	void reset();
	BOOL has_error() { return err_flag; }
	BOOL need_draw() { return draw_flag; }

	void turnon_key(char key) { 
		keys[keymap[key]] = 1; 
	}
	void turnoff_key(char key) { keys[keymap[key]] = 0; }

	void keymap_remap(BYTE remap[256]) {
		memcpy(remap, keymap, sizeof(remap));
	}

public:
	BYTE screen[64][32];

private:
	BOOL draw_flag;
	BOOL err_flag;
	BYTE memory[4096];
	// Chip8 has 4KB memory

	// And a graphics system of 64 x 32 pixels
	BYTE V[16];
	// Chip8 has 15 universal register and 1 carry flag register
	// Namely V0, V1 ... VE and CF
	BYTE keys[16];
	// Shabby keyboard of chip8 only has 16 keys
	BYTE
		timer_delay,
		timer_sound;
	// There's no hard interrupt but 2 timers counting at 60Hz
	WORD IR;
	WORD PC;
	WORD SP;
	// Respectively index register and program counter and stack pointer
	WORD stack[16];
	// Only 16 levels of stack
	WORD op;

	BYTE keymap[256] = { 0 };


private:
	void _clear_screen() {
		memset(screen, 0, sizeof(screen));
		std::cout << "Clear !" << std::endl;
	}

	void _error_op(WORD section) {
		std::cerr << "Unknown OpCode "
			<< "[section " << section << "] : "
			<< std::hex << std::uppercase << op
			<< std::endl;
		err_flag = true;
	}

	void _beep() {
		std::cout << "Beep" << std::endl;
	}
};