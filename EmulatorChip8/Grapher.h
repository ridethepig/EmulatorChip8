#pragma once

#include <Windows.h>
#include <conio.h>
#include <SDL.h>

const int scaler = 15;
const int HEIGHT = 32 * scaler;
const int WIDTH = 64 * scaler;

static HANDLE hcon = GetStdHandle(STD_OUTPUT_HANDLE);
static char scr_buffer[64][32];

void graph_draw(BYTE scr[64][32]) {
	SetConsoleCursorPosition(hcon, { 0,0 });
	for (int y = 0; y < 32; ++y) {
		for (int x = 0; x < 64; ++x) {
			scr_buffer[x][y] = scr[x][y] == 1 ? '#' : ' ';
		}
	}
	for (int y = 0; y < 32; ++y) {
		for (int x = 0; x < 64; ++x) {
			putchar(scr_buffer[x][y]);
		}
		puts("");
	}
}	

void sdl_draw(BYTE scr[64][32], SDL_Renderer * renderer, BOOL line_scan_on = FALSE) {
	SDL_Rect pix_rect = { 0, 0, scaler, scaler };

	for (int y = 0; y < 32; ++y) {
		for (int x = 0; x < 64; ++x) {			
			if (scr[x][y] != scr_buffer[x][y]) {
				if (scr[x][y] == 1) {
					SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
				}
				else {
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				}
				pix_rect.x = x * scaler;
				pix_rect.y = y * scaler;
				SDL_RenderFillRect(renderer, &pix_rect);
				scr_buffer[x][y] = scr[x][y];
			}
		}
		if (line_scan_on)
			SDL_RenderPresent(renderer);
	}
	if (!line_scan_on)
		SDL_RenderPresent(renderer);
}