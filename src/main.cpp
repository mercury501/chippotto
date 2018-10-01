#include <iostream>
#include"chip8.h"
#include <string>
#include <cstring>
#include "include/SDL.h"

using namespace std;
int w = 1024;
int h = 512;

SDL_Window* window = NULL;

uint8_t keymap[16] = {
	SDLK_x,
	SDLK_1,
	SDLK_2,
	SDLK_3,
	SDLK_q,
	SDLK_w,
	SDLK_e,
	SDLK_a,
	SDLK_s,
	SDLK_d,
	SDLK_z,
	SDLK_c,
	SDLK_4,
	SDLK_r,
	SDLK_f,
	SDLK_v,
};

int wmain() {
	// crude graphics, space is white and  is black
	bool loaded = 0;
	chip8 achip8;
	bool finished = 0;



	//setup graphics
	SDL_Init(SDL_INIT_EVERYTHING);

	window = SDL_CreateWindow(
		"CHIP-8 Emulator",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		w, h, SDL_WINDOW_SHOWN
	);

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_RenderSetLogicalSize(renderer, w, h);

	SDL_Texture* sdlTexture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		64, 32);
	//setup input

	//initialize cpu
	//load rom
	
	

	//TODO
	//sound, draw routine, subroutines, timers

	achip8.initialize();
	

	do {
		cout << "Insert rom file path:  ";
		string filepath;
		cin >> filepath;

		loaded = achip8.load_binary(filepath);
		if (!loaded)
			cout << endl << "Insert a valid file path" << endl;
	} while (!loaded);
	while (1) {

		SDL_Event ke;  //event container
		
		while (SDL_PollEvent(&ke)) {
			if (ke.type == SDL_KEYDOWN) {
				/*if (e.key.keysym.sym == SDLK_ESCAPE)
					exit(0);

				if (e.key.keysym.sym == SDLK_F1)
					goto load;   */   // *gasp*, a goto statement!
					// Used to reset/reload ROM

				for (int i = 0; i < 16; ++i) {
					if (ke.key.keysym.sym == keymap[i]) {
						achip8.key[i] = 1;
					}
				}
			}
		}

		// Process keyup events
		if (ke.type == SDL_KEYUP) {
			for (int i = 0; i < 16; ++i) {
				if (ke.key.keysym.sym == keymap[i]) {
					achip8.key[i] = 0;
				}
			}
		}




		

		achip8.emucycle();
	

		if (achip8.drawflag) {
			uint32_t pixels[2048];

			for (int i = 0; i < 2048; ++i) {
				uint8_t pixel = achip8.gfx[i];
				pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;

			}

			// Update SDL texture
			SDL_UpdateTexture(sdlTexture, NULL, pixels, 64 * sizeof(Uint32));
			// Clear screen and render
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
			SDL_RenderPresent(renderer);
		}
}

	system("PAUSE");
	return 0;


}