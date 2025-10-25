#pragma once

#include "SDL.h"
#include "FreeImage/FreeImage.h"
#include "SDL_mixer.h"
#include <fstream>
#include <string>
#include <vector>

extern const int SWIDTH;
extern const int SHEIGHT;
extern const char App_Name[];

SDL_Surface* get_sdl_surface(FIBITMAP* freeimage_bitmap, int is_grayscale); // freeimage -> sdlsurface
SDL_Surface* loadImage(char* path);
SDL_Surface* loadImage(FIBITMAP* freeimage_bitmap);

class RegularObject {
public:
	RegularObject() : Image(nullptr), name("none"), rect({ 0, 0, 0, 0 }) {}
	RegularObject(char* path, std::string name, int x = 0, int y = 0, int w = 0, int h = 0, bool left_or_center = false) {
		SDL_Surface* surface = loadImage(path);
		if (w <= 0 || h <= 0) {
			w = surface->w;
			h = surface->h;
		}
		if (x < 0 || y < 0) {
			x = 0;
			y = 0;
		}
		if (left_or_center) { // your (x,y) is a center of image
			x -= w / 2;
			y -= h / 2;
		}
		rect = { x, y, w, h };
		Image = surface;
		this->name = name;
	}
	RegularObject(FIBITMAP* freeimage_bitmap, std::string name, int x = 0, int y = 0, int w = 0, int h = 0, bool left_or_center = false) {
		SDL_Surface* surface = loadImage(freeimage_bitmap);
		if (w <= 0 || h <= 0) {
			w = surface->w;
			h = surface->h;
		}
		if (x < 0 || y < 0) {
			x = 0;
			y = 0;
		}
		if (left_or_center) { // your (x,y) is a center of image
			x -= w / 2;
			y -= h / 2;
		}
		rect = { x, y, w, h };
		Image = surface;
		this->name = name;
	}
	SDL_Surface* get_surface() { return Image; }
	void set_surface(char* path) {
		Image = loadImage(path);
	}
	void set_surface(FIBITMAP* freeimage_bitmap) {
		Image = loadImage(freeimage_bitmap);
	}
	SDL_Rect get_rect() const { return rect; }
	std::string get_name() const { return name; }
	void set_rect(int x, int y, int w, int h, bool left_or_center) {
		if (w <= 0 || h <= 0) {
			w = Image->w;
			h = Image->h;
		}
		if (x < 0 || y < 0) {
			x = 0;
			y = 0;
		}
		if (left_or_center) { // your (x,y) is a center of image
			x -= w / 2;
			y -= h / 2;
		}
		rect = { x, y, w, h };
	}
private:
	SDL_Surface* Image;
	std::string name;
	SDL_Rect rect;
};

class SDLBase {
public:
	std::vector<RegularObject> objects;
	SDL_Renderer* renders;
	Mix_Chunk* quirk;
	Mix_Chunk* quirk2;
	SDLBase(std::ofstream& plog) {
		window = nullptr;
		screenSurface = nullptr;
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
	bool append_object(RegularObject& new_object) {
		if (new_object.get_surface() == nullptr) {
			return false;
		}
		objects.push_back(new_object);
		return true;
	}
	bool remove_object(std::string name) {
		auto it = std::find_if(objects.begin(), objects.end(),
			[&](const RegularObject& obj) {
				return obj.get_name() == name;
			}
		);
		if (it == objects.end()) {
			*plog << "Regular " << name << " not found\n";
			return false;
		}
		SDL_FreeSurface((*it).get_surface());
		objects.erase(it);
		return true;
	}
	void clear_objects() {
		for (int i = 0; i < objects.size(); i++) {
			SDL_FreeSurface(objects[i].get_surface());
		}
		objects.clear();
	}
	RegularObject& get_regular(std::string name) {
		auto it = std::find_if(objects.begin(), objects.end(),
			[&](const RegularObject& obj) {
				return obj.get_name() == name;
			}
		);
		if (it == objects.end()) {
			*plog << "Regular " << name << " not found\n";
			return RegularObject();
		}
		return *it;
	}
	void render_single(SDL_Surface* surface, int x = 0, int y = 0, int w = 0, int h = 0, bool leftcenter = 0); // placing image
	void render();
	void KillAll();
private:
	std::ofstream* plog;
	SDL_Window* window;
	SDL_Surface* screenSurface;
	bool success;
};
