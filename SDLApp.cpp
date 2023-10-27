#include "SDLApp.h"

SDL_Surface *SDLBase::loadImage(char *path) {
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

void SDLBase::renderer(SDL_Surface *surface, int x, int y, int w, int h, bool leftcenter) {
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
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renders, surface);
	if (texture == nullptr) {
		*plog << "Failed to load image as texture\n";
	}
	SDL_RenderCopy(renders, texture, nullptr, &TmpRect);
	SDL_DestroyTexture(texture);
}

SDL_Surface *SDLBase::get_sdl_surface(FIBITMAP *freeimage_bitmap, int is_grayscale) {
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
		0
	);
	if (sdl_surface == nullptr) {
		*plog << "Failed to create surface: " << SDL_GetError() << '\n';
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

void SDLBase::KillAll() {
	Mix_FreeChunk(quirk);
	quirk = nullptr;
	Mix_FreeChunk(quirk2);
	quirk2 = nullptr;
	*plog << "Destroyed sfx\n";
	for (int i = 0; i < (sizeof(Images) / sizeof(Images[0])); i++) {
		SDL_FreeSurface(Images[i]);
		Images[i] = nullptr;
	}
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
