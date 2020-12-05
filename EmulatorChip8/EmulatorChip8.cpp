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

/* Credits:
* http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter
* Chip-8 implementation guide
* http://www.lazyfoo.net/
* SDL tutorial
*/

#include "Chip8.h"
#include "Grapher.h"
#include "Timer.h"
#include "Utils.h"

#include <iostream>

int main(int argc, char **argv)
{
    int FPS = 200;          // frame per second
    int TPS = 1000 / FPS;   // ticks per frame
    
    // Set Console size
    SMALL_RECT srect = { 0, 0, 400, 300 };
    SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &srect);
    // This may fail on certain version of Windows

    //-------------------------------------- Load configuration ----------------------------------
    Configure conf;
    conf.load_config();
    _tprintf(TEXT("Emulator will start with\n")
        TEXT("FPS       = %d\n")
        TEXT("KeyRemap  = %s\n")
        TEXT("ROM       = %s\n"), 
        conf.get_fps(), 
        conf.get_keymap_stat() == nullptr ? TEXT("None") : conf.get_keymap_stat(), 
        conf.get_default_rom() == nullptr ? TEXT("Not specified") : conf.get_default_rom());
    
    TCHAR* rom_path = conf.get_default_rom();

    if (!rom_path) {
        rom_path = Open_file_dialog();
    }
    if (!rom_path) {
        std::cout << "ROM not set. Exit." << std::endl;
        return 1;
    }

    // ------------------------------------------- Load rom file -----------------------------------

    BYTE* buffer = nullptr; 
    int filesize = 0;
    buffer = load_application(rom_path, filesize);
    if (buffer == nullptr || filesize < 0) return 1;

    //----------------------------------------------------------------------------------------------
    std::cout << "Initializing Emulator..." << std::endl;

    Chip8 chip;
    chip.initialize();
    chip.load_code(buffer, filesize);
    delete[] buffer;
    if (conf.get_keymap_on()) {
        chip.keymap_remap(conf.get_keymap());
    }

    std::cout << "Emulator Ready." << std::endl;
    //------------------------------------------------------------------------------------------------
    std::cout << "Initializing Displayer..." << std::endl;

    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::cerr << "Failure at Displayer[SDL] initialization." << std::endl;
        return -1;
    }

    SDL_Window* gfx_window = 
        SDL_CreateWindow("Chip-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);

    if (gfx_window == nullptr) {
        std::cerr << "Failure at Displayer[SDL Window] initialization." << std::endl;
        return -2;
    }

    SDL_Renderer* gfx_renderer = SDL_CreateRenderer(gfx_window, -1, SDL_RENDERER_ACCELERATED);
    if (gfx_renderer == nullptr) {
        std::cerr << "Failure at Displayer[SDL Renderer] initialization." << std::endl;
        SDL_DestroyWindow(gfx_window);
        return -3;
    }        

    FPS = conf.get_fps();
    TPS = 1000 / FPS;

    std::cout << "Displayer ready." << std::endl;

    //------------------------------------------------------------------------------------------------

    BOOL is_to_quit = FALSE;
    SDL_Event gfx_event;    

    std::cout << "Main Loop Start." << std::endl;

    Timer fpsTimer;
    Timer capTimer;
    int countedFrames = 0;
    fpsTimer.start();

    while (!is_to_quit) {
        capTimer.start();
        //Sleep(100);
        if (SDL_PollEvent(&gfx_event)) {
            switch (gfx_event.type)
            {
            case SDL_QUIT:
                is_to_quit = TRUE;
                break;
            case SDL_KEYDOWN:
                switch (gfx_event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    is_to_quit = TRUE;
                    break;
                case SDLK_UP:
                    FPS += 5;
                    TPS = 1000 / FPS;
                    if (FPS >= 1000) TPS = 1;
                    std::cout << "FPS UP:" << FPS << "\n";
                    break;
                case SDLK_DOWN:
                    FPS -= 5;
                    if (FPS <= 10) FPS = 10;
                    TPS = 1000 / FPS;
                    std::cout << "FPS DOWN:" << FPS << "\n";
                    break;
                case SDLK_MINUS:
                    std::cout << "CPU Reset.\n";
                    chip.reset();
                    break;
                default:
                    chip.turnon_key(gfx_event.key.keysym.sym);                
                    break;
                }
                break;
            case SDL_KEYUP:
                chip.turnoff_key(gfx_event.key.keysym.sym);                
                break;
            }
            continue;
        }

        chip.emulate_cycle();
        if (chip.has_error()) break;
        if (chip.need_draw()) {
            sdl_draw(chip.screen, gfx_renderer);            
        }

        ++countedFrames;

        int frameTicks = capTimer.getTicks();
        if (frameTicks < TPS)
        {
            SDL_Delay(TPS - frameTicks);
        }
        // Control FPS, 'cause modern computers are merely too fast for chip8
    }

    std::cout << "User Termination. Clearing Up..." << std::endl;

    if (gfx_renderer) {
        SDL_DestroyRenderer(gfx_renderer);
    }
    if (gfx_window) {
        SDL_DestroyWindow(gfx_window);
    }
    SDL_Quit();
    system("pause");

    return 0;
}