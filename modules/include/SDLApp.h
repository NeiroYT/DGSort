#pragma once

#include "SDL.h"
#include "FreeImage/FreeImage.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"
#include <fstream>
#include <string>
#include <vector>

extern const int SWIDTH;
extern const int SHEIGHT;
extern const int CURWIDTH;
extern const int CURHEIGHT;
extern const char App_Name[];
const bool keep_ratio = true;

SDL_Surface* get_sdl_surface(FIBITMAP* freeimage_bitmap, int is_grayscale); // freeimage -> sdlsurface
SDL_Surface* loadImage(char* path);
SDL_Surface* loadImage(FIBITMAP* freeimage_bitmap);

class RegularObject {
public:
	RegularObject() : Image(nullptr), ttffont(nullptr), name("none"), ttftext(""), rect({0, 0, 0, 0}) {}
	RegularObject(char* path, std::string name, int x = 0, int y = 0, int w = 0, int h = 0, bool left_or_center = false) {
		ttffont = nullptr;
		ttftext = "";
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
	RegularObject(std::string text, std::string font, std::string name, int size, SDL_Color color, int x = 0, int y = 0, int w = 0, bool left_or_center = false) {
		this->color = color;
		this->ttffont = TTF_OpenFont(font.c_str(), size);
		this->wrap_width = w;
		this->ttftext = text;
		if (ttffont == nullptr) {
			throw std::exception("Invalid font");
		}
		SDL_Surface* surface;
		if (w <= 0) {
			surface = TTF_RenderText_Solid(ttffont, text.c_str(), color);
			w = surface->w;
		}
		else {
			surface = TTF_RenderText_Solid_Wrapped(ttffont, text.c_str(), color, w);
		}
		if (x < 0 || y < 0) {
			x = 0;
			y = 0;
		}
		if (left_or_center) { // your (x,y) is a center of image
			x -= surface->w < w ? (surface->w / 2) : (w / 2);
			y -= surface->h / 2;
		}
		rect = { x, y, surface->w, surface->h };
		Image = surface;
		this->name = name;
	}
	RegularObject(FIBITMAP* freeimage_bitmap, std::string name, int x = 0, int y = 0, int w = 0, int h = 0, bool left_or_center = false) {
		ttffont = nullptr;
		ttftext = "";
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
	TTF_Font* get_font() { return ttffont; }
	void set_surface(char* path) {
		if (ttftext != "") {
			return;
		}
		SDL_FreeSurface(Image);
		Image = loadImage(path);
	}
	void set_surface_text(std::string new_text, bool left_or_center) {
		this->ttftext = new_text;
		int old_w = Image->w;
		SDL_Surface* surface = TTF_RenderText_Solid_Wrapped(ttffont, ttftext.c_str(), color, wrap_width);
		SDL_FreeSurface(Image);
		Image = surface;
		if (!left_or_center) {
			set_rect_text(rect.x, rect.y, wrap_width, false);
		}
		else {
			set_rect_text(rect.x + old_w / 2, rect.y + Image->h / 2, wrap_width, true);
		}
	}
	void set_surface(FIBITMAP* freeimage_bitmap) {
		SDL_FreeSurface(Image);
		Image = loadImage(freeimage_bitmap);
	}
	SDL_Rect get_rect() const { return rect; }
	std::string get_name() const { return name; }
	void set_rect(int x, int y, int w, int h, bool left_or_center) {
		if (ttftext != "") {
			return;
		}
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
	void set_rect_text(int x, int y, int w, bool left_or_center) {
		if (w <= 0) {
			w = wrap_width;
		}
		if (x < 0 || y < 0) {
			x = 0;
			y = 0;
		}
		if (w != wrap_width) {
			SDL_Surface* surface = TTF_RenderText_Solid_Wrapped(ttffont, ttftext.c_str(), color, w);
			SDL_FreeSurface(Image);
			Image = surface;
			wrap_width = w;
		}
		if (left_or_center) { // your (x,y) is a center of image
			x -= Image->w < w ? (Image->w / 2) : (w / 2);
			y -= Image->h / 2;
		}
		rect = { x, y, Image->w, Image->h };
	}
private:
	SDL_Surface* Image;
	std::string name;
	SDL_Rect rect;
	TTF_Font* ttffont;
	std::string ttftext;
	int wrap_width;
	SDL_Color color;
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
				window = SDL_CreateWindow(App_Name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, CURWIDTH, CURHEIGHT, SDL_WINDOW_SHOWN);
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
		SDL_RenderSetScale(renders, (float)CURWIDTH / SWIDTH, (float)CURHEIGHT / SHEIGHT);
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
