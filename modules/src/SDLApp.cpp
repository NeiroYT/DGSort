#include "SDLApp.h"

SDL_Surface *loadImage(char *path) {
	FREE_IMAGE_FORMAT filetype = FreeImage_GetFileType(path, 0);
	FIBITMAP *freeimage_bitmap = FreeImage_Load(filetype, path, 0);
	int is_grayscale = 0;
	if (FreeImage_GetColorType(freeimage_bitmap) == FIC_MINISBLACK) {
		// Single channel so ensure image is compressed to 8-bit.
		is_grayscale = 1;
		FIBITMAP *tmp_bitmap = FreeImage_ConvertToGreyscale(freeimage_bitmap);
		FreeImage_Unload(freeimage_bitmap);
		freeimage_bitmap = tmp_bitmap;
	}
	SDL_Surface *sdl_surface = get_sdl_surface(freeimage_bitmap, is_grayscale);
	return sdl_surface;
}

SDL_Surface *loadImage(FIBITMAP* freeimage_bitmap) {
	int is_grayscale = 0;
	if (FreeImage_GetColorType(freeimage_bitmap) == FIC_MINISBLACK) {
		// Single channel so ensure image is compressed to 8-bit.
		is_grayscale = 1;
		FIBITMAP *tmp_bitmap = FreeImage_ConvertToGreyscale(freeimage_bitmap);
		FreeImage_Unload(freeimage_bitmap);
		freeimage_bitmap = tmp_bitmap;
	}
	SDL_Surface *sdl_surface = get_sdl_surface(freeimage_bitmap, is_grayscale);
	return sdl_surface;
}

SDL_Surface *get_sdl_surface(FIBITMAP *freeimage_bitmap, int is_grayscale) {
	FreeImage_FlipVertical(freeimage_bitmap);
	SDL_Surface *sdl_surface = SDL_CreateRGBSurfaceFrom(
		FreeImage_GetBits(freeimage_bitmap),
		FreeImage_GetWidth(freeimage_bitmap),
		FreeImage_GetHeight(freeimage_bitmap),
		FreeImage_GetBPP(freeimage_bitmap),
		FreeImage_GetPitch(freeimage_bitmap),
		FreeImage_GetRedMask(freeimage_bitmap),
		FreeImage_GetGreenMask(freeimage_bitmap),
		FreeImage_GetBlueMask(freeimage_bitmap),
		4278190080
	);
	if (sdl_surface == nullptr) {
		return sdl_surface;
	}
	if (is_grayscale) {
		// To display a grayscale image we need to create a custom palette.
		SDL_Color colors[256];
		int i;
		for (i = 0; i < 256; i++) {
			colors[i].r = colors[i].g = colors[i].b = i;
		}
		SDL_SetPaletteColors(sdl_surface->format->palette, colors, 0, 256);
	}
	return sdl_surface;
}

FIBITMAP* get_fibitmap(SDL_Surface* sdl_surface) {
	SDL_Surface* gray_to_rgb;
	FIBITMAP* fibitmap;
	if (sdl_surface->format->palette->ncolors == 256) {
		gray_to_rgb = SDL_ConvertSurfaceFormat(sdl_surface, SDL_PIXELFORMAT_BGRA32, 1);
		fibitmap = FreeImage_ConvertFromRawBits((BYTE*)gray_to_rgb->pixels, gray_to_rgb->w, gray_to_rgb->h, gray_to_rgb->pitch,
			gray_to_rgb->format->BitsPerPixel, gray_to_rgb->format->Rmask,
			gray_to_rgb->format->Gmask, gray_to_rgb->format->Bmask);
	}
	else {
		fibitmap = FreeImage_ConvertFromRawBits((BYTE*)sdl_surface->pixels, sdl_surface->w, sdl_surface->h, sdl_surface->pitch,
			sdl_surface->format->BitsPerPixel, sdl_surface->format->Rmask,
			sdl_surface->format->Gmask, sdl_surface->format->Bmask);
	}
	if (fibitmap == nullptr) {
		return nullptr;
	}
	FreeImage_FlipVertical(fibitmap);
	return fibitmap;
}

void SDLBase::render_single(SDL_Surface* surface, int x, int y, int w, int h, bool leftcenter) {
	if (w <= 0 || h <= 0) {
		w = surface->w;
		h = surface->h;
	}
	if (x < 0 || y < 0) {
		x = 0;
		y = 0;
	}
	if (leftcenter) { // your (x,y) is a center of image
		x -= w / 2;
		y -= h / 2;
	}
	SDL_Rect TmpRect = { x, y, w, h };
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renders, surface);
	if (texture == nullptr) {
		*plog << "Failed to load image as texture\n";
	}
	SDL_RenderCopy(renders, texture, nullptr, &TmpRect);
	SDL_DestroyTexture(texture);
}

void SDLBase::render() {
	for (int i = 0; i < objects.size(); i++) {
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renders, objects[i].get_surface());
		if (texture == nullptr) {
			*plog << "Failed to load image as texture\n";
		}
		SDL_RenderCopy(renders, texture, nullptr, &objects[i].get_rect());
		SDL_DestroyTexture(texture);
	}
}

void SDLBase::KillAll() {
	Mix_FreeChunk(quirk);
	quirk = nullptr;
	Mix_FreeChunk(quirk2);
	quirk2 = nullptr;
	*plog << "Destroyed sfx\n";
	for (int i = 0; i < objects.size(); i++) {
		SDL_FreeSurface(objects[i].get_surface());
		if (objects[i].get_font() != nullptr) {
			TTF_CloseFont(objects[i].get_font());
;		}
	}
	objects.clear();
	*plog << "Destroyed images\n";
	SDL_SetRenderDrawColor(renders, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(renders);
	SDL_DestroyRenderer(renders);
	*plog << "Destroyed renderer\n";
	SDL_FreeSurface(screenSurface);
	SDL_DestroyWindow(window);
	window = nullptr;
	*plog << "Destroyed window\n";
	SDL_Quit();
	*plog << "Destroyed SDL Init\n";
}
