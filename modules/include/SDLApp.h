#pragma once

#include <SDL3/SDL.h>
#include <FreeImage/FreeImage.h>
#include <SDL3/SDL_mixer.h>
#include <fstream>

extern const int SWIDTH;
extern const int SHEIGHT;
extern const char App_Name[];

class SDLBase {
public:
	SDL_Surface *Image1;
	SDL_Surface *Image2;
	SDL_Surface *Button1;
	SDL_Surface *ButtonSave;
	SDL_Surface *ButtonLoad;
	SDL_Surface *Images[5];
	SDL_Renderer *renders;
	Mix_Chunk *quirk;
	Mix_Chunk *quirk2;
	SDLBase(std::ofstream &plog) {
		window = nullptr;
		screenSurface = nullptr;
		Image1 = nullptr;
		Image2 = nullptr;
		Button1 = nullptr;
		ButtonSave = nullptr;
		ButtonLoad = nullptr;
		SDL_Surface *tmpImgs[5] = { Image1, Image2, Button1, ButtonSave, ButtonLoad };
		for (int i = 0; i < 5; i++) {
			Images[i] = tmpImgs[i];
		}
		renders = nullptr;
		quirk = nullptr;
		quirk2 = nullptr;
		success = true;
		this->plog = &plog;
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
			plog << "SDL_Error: %s\n", SDL_GetError();
			success = 0;
		}
		else {
			if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
				plog << "SDL_mixer Error: %s\n", Mix_GetError();
				success = 0;
			}
			else {
				window = SDL_CreateWindow(App_Name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SWIDTH, SHEIGHT, SDL_WINDOW_SHOWN);
				if (window == NULL) {
					plog << "SDL_Error: %s\n", SDL_GetError();
					success = 0;
				}
				else {
					renders = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
					if (renders == NULL) {
						plog << "Failed to create renderer: " << SDL_GetError() << '\n';
						success = 0;
					}
					else {
						screenSurface = SDL_GetWindowSurface(window);
					}
				}
			}
		}
		quirk = Mix_LoadWAV("images\\GUI\\1.wav");
		quirk2 = Mix_LoadWAV("images\\GUI\\2.wav");
		if (!quirk || !quirk2) {
			plog << "Error: Can't load sfx\n";
			success = 0;
		}
	}
	~SDLBase() {
		KillAll();
	}
	bool get_success_state() const {
		return success;
	}
	SDL_Surface *loadImage(char *path);
	SDL_Surface *loadImage(FIBITMAP *freeimage_bitmap);
	void renderer(SDL_Surface *surface, int x = 0, int y = 0, int w = 0, int h = 0, bool leftcenter = 0); // placing images
	void KillAll();
private:
	std::ofstream *plog;
	SDL_Window *window;
	SDL_Surface *screenSurface;
	bool success;
	SDL_Surface *get_sdl_surface(FIBITMAP *freeimage_bitmap, int is_grayscale); // freeimage -> sdlsurface
};
