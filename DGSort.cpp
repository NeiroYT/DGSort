//#define SDL_MAIN_HANDLED
#define SWIDTH 1280
#define SHEIGHT 720

#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/FreeImage.h>
#include <SDL2/SDL_mixer.h>
#include <fstream>

using namespace std;

//basic functions
void separate(int size, int row, int columns); // size - amount of images, row - ysqrnum, columns - xsqrnum
void Fill(int *arr, int len);
char* generatePath(int num, char* to);
bool inouttest();
bool mouseinrange(int &mx, int &my, int w, int h, int posx, int posy); // is your mouse here?

int LIST_Len; // SORT NUMBER (num of images)
ofstream plog;

class SortList {
public:
	SortList(int len = LIST_Len) {
		//SortList1 = (int*)malloc(LIST_Len * sizeof(int));
		SortList1 = new int[len];
		SortLen = 0;
		SortMaxLen = len;
	}
	SortList(const SortList &o) {
		int len = o.SortMaxLen;
		SortList1 = new int[len];
		SortLen = 0;
		SortMaxLen = len;
		for (int i = 0; i < o.SortLen; i++) {
			setelem(i, o.SortList1[i]);
		}
	}
	~SortList() {
		delete[] SortList1;
	}
	SortList& operator= (int *p);
	SortList& operator= (SortList list);
	int length() { return SortLen; }
	int getelem(int n) {
		if (n < SortLen && n >= 0) {
			return SortList1[n];
		}
		else {
			return -1;
		}
	}
	void setelem(int n, int elem) {
		if (n >= 0 && n < SortMaxLen) {
			if (n >= SortLen) {
				SortLen = n + 1;
			}
			SortList1[n] = elem;
		}
	}
	void print() {
		plog << "[";
		for (int i = 0; i < SortLen; i++) {
			if (i == SortLen - 1) {
				plog << SortList1[i] << "]";
			}
			else {
				plog << SortList1[i] << ", ";
			}
		}
	}
	SortList slice(int start, int end);
private:
	int *SortList1;
	int SortLen;
	int SortMaxLen;
};

SortList SortList::slice(int start, int end) {
	SortList res(end-start);
	for (int i = start; i < end; i++) {
		res.setelem(i - start, getelem(i));
	}
	return res;
}

SortList& SortList::operator= (int *p) {
	for (int i = 0; i < SortMaxLen; i++) {
		setelem(i, p[i]);
	}
	return *this;
}

SortList& SortList::operator= (SortList list) {
	for (int i = 0; i < list.length(); i++) {
		setelem(i, list.getelem(i));
	}
	return *this;
}

class Sort {
public:
	bool status;
	Sort() {
		status = 0;
		pointer = 1;
		sortlistlen = 1;
		TempList = new int[LIST_Len];
		RecordList = new int[LIST_Len];
		sortlist = new SortList[LIST_Len*2];
		parentlist = new int[LIST_Len*2];
		Fill(TempList, LIST_Len);
		for (int i = 0; i < LIST_Len; i++) {
			RecordList[i] = 0;
		}
		sortlist[0] = TempList;
		for (int i = 0; i < sortlistlen; i++) {
			if (sortlist[i].length() >= 2) {
				int Marker = ceil((double)(sortlist[i].length())/2);
				sortlist[pointer] = sortlist[i].slice(0, Marker);
				parentlist[pointer] = i;
				pointer++;
				sortlistlen++;
				// merging
				sortlist[pointer] = sortlist[i].slice(Marker, sortlist[i].length());
				parentlist[pointer] = i;
				pointer++;
				sortlistlen++;
			}
		}
		leftlist = sortlistlen - 2;
		rightlist = sortlistlen - 1;
		leftID = rightID = recordID = 0;
	}
	~Sort() {
		delete[] parentlist;
		delete[] sortlist;
		delete[] RecordList;
		delete[] TempList;
	}
	int returnleft() { return sortlist[leftlist].getelem(leftID); };
	int returnright() { return sortlist[rightlist].getelem(rightID); };
	void SortingIter(int choice);
	SortList getFinalSort() {
		return sortlist[0];
	}
	bool sort_undo();
	void printfulllist(int len);
	void printsortlist();
	void save();
	void load();
private:
	void countup(int choice); // 0 - left, 1 - right
	int pointer;
	int *TempList;
	SortList *sortlist;
	int sortlistlen;
	int *parentlist;
	int leftlist, rightlist;
	int leftID, rightID, recordID;
	int *RecordList;
};

void Sort::save() {
	ofstream savefile;
	savefile.open("save.dat");
	int *data = new int[1024];
	int *ptrs[] = { &sortlistlen, &leftID, &rightID, &recordID, &leftlist, &rightlist };
	for (int i = 0; i < 6; i++) {
		data[i] = *(ptrs[i]);
	}
	int pnt = 6;
	for (int i = 0; i < sortlistlen; i++) {
		data[pnt] = sortlist[i].length();
		for (int j = 0; j < sortlist[i].length(); j++) {
			data[pnt + j + 1] = sortlist[i].getelem(j);
		}
		pnt += sortlist[i].length() + 1;
	}
	for (int i = 0; i < LIST_Len; i++) {
		data[pnt + i] = RecordList[i];
	}
	pnt += LIST_Len;
	data[pnt] = -1234;
	for (int i = 0; data[i] != -1234; i++) {
		savefile << data[i] << "\n";
	}
	savefile.close();
	delete[] data;
}

void Sort::load() {
	ifstream savefile;
	savefile.open("save.dat");
	if (!savefile) {
		return;
	}
	int pnt = 0;
	int i = 0;
	int *data = new int[1024];
	char temp[50];
	int *ptrs[] = { &sortlistlen, &leftID, &rightID, &recordID, &leftlist, &rightlist };
	while (!savefile.eof()) {
		savefile.getline(temp, 50);
		data[i] = atoi(temp);
		i++;
	} // get i lines from file
	if (i < 6) {
		return;
	}
	for (pnt = 0; pnt < 6; pnt++) {
		*(ptrs[pnt]) = data[pnt];
	} // basic ints
	for (int j = 0; j < sortlistlen; j++) {
		int len = data[pnt];
		pnt++;
		for (int j1 = 0; j1 < len; j1++) {
			sortlist[j].setelem(j1, data[pnt]);
			pnt++;
		}
	} // fill every of the sortlistlen lists
	for (int h = 0; h < LIST_Len; h++) {
		RecordList[h] = data[pnt];
		pnt++;
	} // recordlist filling
	plog << "Loaded:\n";
	for (int i1 = 0; i1 < 6; i1++) {
		plog << *(ptrs[i1]) << ' ';
	}
	printsortlist();
	for (int l = 0; l < LIST_Len; l++) {
		plog << RecordList[l] << ' ';
	}
	plog << '\n';
	savefile.close();
	delete[] data;
}

bool Sort::sort_undo() {
	if (leftID == 0 && rightID == 0) {
		return 0;
	}
	recordID--;
	if (RecordList[recordID] == sortlist[leftlist].getelem(leftID-1)) {
		leftID--;
	}
	else if (RecordList[recordID] == sortlist[rightlist].getelem(rightID-1)) {
		rightID--;
	}
	return 1;
}

void Sort::countup(int choice) {
	int ulist = (choice == 0) ? leftlist : rightlist;
	int uid = (choice == 0) ? leftID : rightID;
	RecordList[recordID] = sortlist[ulist].getelem(uid);
	if (choice == 0) {
		leftID++;
	}
	else {
		rightID++;
	}
	recordID++;
}

void Sort::printsortlist() {
	plog << "[";
	for (int i = 0; i < sortlistlen; i++) {
		sortlist[i].print();
		if (i != sortlistlen - 1) {
			plog << ", ";
		}
		else {
			plog << "]\n";
		}
	}
	plog << "Length: " << sortlistlen << '\n';
}

void Sort::printfulllist(int len) {
	plog << "[";
	for (int i = 0; i < len; i++) {
		if (i != len - 1) {
			plog << TempList[i] << ", ";
		}
		else {
			plog << TempList[i] << "]\n";
		}
	}
}

void Sort::SortingIter(int choice) {
	if (!status) {
		if (choice != -1) {
			countup(choice);
		}
		if (leftID < sortlist[leftlist].length() && rightID == sortlist[rightlist].length()) {
			while (leftID < sortlist[leftlist].length()) {
				countup(0);
			}
		}
		else if (leftID == sortlist[leftlist].length() && rightID < sortlist[rightlist].length()) {
			while (rightID < sortlist[rightlist].length()) {
				countup(1);
			}
		}
		if (leftID == sortlist[leftlist].length() && rightID == sortlist[rightlist].length()) {
			for (int i = 0; i < sortlist[leftlist].length() + sortlist[rightlist].length(); i++) {
				sortlist[parentlist[leftlist]].setelem(i, RecordList[i]);
			}
			sortlistlen -= 2;
			leftlist -= 2;
			rightlist -= 2;
			leftID = 0;
			rightID = 0;
			for (int i = 0; i < LIST_Len; i++) {
				RecordList[i] = 0;
			}
			recordID = 0;
		}
		status = (leftlist < 0) ? 1 : 0;
	}
	else {
		plog << "List is Sorted now\n";
	}
}

class SDLBase {
public:
	SDL_Window *window = NULL;
	SDL_Surface *screenSurface = NULL;
	SDL_Surface *Image1 = NULL;
	SDL_Surface *Image2 = NULL;
	SDL_Surface *Button1 = NULL;
	SDL_Surface *ButtonSave = NULL;
	SDL_Surface *ButtonLoad = NULL;
	SDL_Surface *Images[5] = {Image1, Image2, Button1, ButtonSave, ButtonLoad};
	SDL_Renderer *renders = NULL;
	Mix_Chunk *quirk = NULL;
	Mix_Chunk *quirk2 = NULL;
	bool success = 1;
	SDLBase() {
		success = true;
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0) {
			plog << "SDL_Error: %s\n", SDL_GetError();
			success = 0;
		}
		else {
			if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
				plog << "SDL_mixer Error: %s\n", Mix_GetError();
				success = 0;
			}
			else {
				window = SDL_CreateWindow("Things Sorting Machine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SWIDTH, SHEIGHT, SDL_WINDOW_SHOWN);
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
	SDL_Surface *get_sdl_surface(FIBITMAP *freeimage_bitmap, int is_grayscale); // freeimage -> sdlsurface
	SDL_Surface *loadImage(char *path);
	void renderer(SDL_Surface *surface, int x = 0, int y = 0, int w = 0, int h = 0, bool leftcenter = 0); // placing images
	void KillAll();
};

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
	SDL_Rect TmpRect = {x, y, w, h};
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renders, surface);
	if (texture == NULL) {
		plog << "Failed to load image as texture\n";
	}
	SDL_RenderCopy(renders, texture, NULL, &TmpRect);
	SDL_DestroyTexture(texture);
}

SDL_Surface* SDLBase::get_sdl_surface(FIBITMAP *freeimage_bitmap, int is_grayscale) {
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
	if (sdl_surface == NULL) {
		plog << "Failed to create surface: " << SDL_GetError() << '\n';
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
	quirk = NULL;
	Mix_FreeChunk(quirk2);
	quirk2 = NULL;
	plog << "Destroyed sfx\n";
	for (int i = 0; i < (sizeof(Images) / sizeof(Images[0])); i++) {
		SDL_FreeSurface(Images[i]);
		Images[i] = NULL;
	}
	plog << "Destroyed images\n";
	SDL_SetRenderDrawColor(renders, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(renders);
	SDL_DestroyRenderer(renders);
	plog << "Destroyed renderer\n";
	SDL_FreeSurface(screenSurface);
	SDL_DestroyWindow(window);
	window = NULL;
	plog << "Destroyed window\n";
	SDL_Quit();
	plog << "Destroyed SDL Init\n";
}

void CreateResults(Sort& sort);
SDLBase *call_app(SDLBase *curapp);

int main(int argc, char* argv[]) {
	char read[10];
	SDLBase *output = nullptr;
	plog.open("log.txt", ios::app);
	fstream file1;
	plog << "\n---Neiro's Things Sorting Machine---\n";
	file1.open("config.ini");
	if (!file1 || !plog) {
		plog << "Error: config.ini and log.txt needs to be with your .exe";
		return 1;
	}
	file1.getline(read, 10);
	file1.close();
	LIST_Len = atoi(read);
	if (!LIST_Len) {
		file1.open("config.ini", ios::out);
		file1.write("2\n2\n1\nPlease enter your number of images, rows, columns here (instead of this)", 79);
		file1.close();
		LIST_Len = 2;
	}
	setlocale(0, "");
	SDL_SetMainReady();
	try {
		FreeImage_Initialise();
	}
	catch (...) {
		plog << "FreeImage not working.\n";
		return 1;
	}
	if (inouttest()) {
		plog << "Error: Program doesn't have access to giantimage.png OR can't create files for you\n";
		return 1;
	}
	do {
		plog << "Starting App...\n";
		output = call_app(output);
	} while (output != nullptr);
	plog.close();
	return 0;
}

SDLBase* call_app(SDLBase* curapp) {
	int menu = 0;
	bool quit = 0;
	int mx, my;
	int buttonwidth;
	int buttonheight;
	char path[20];
	Sort DR;
	SDL_Event e;
	SDLBase *App1;
	if (curapp == nullptr) {
		App1 = new SDLBase;
	}
	else {
		App1 = curapp;
	}
	if (!App1->success) {
		plog << "Error: Application cannot be run\n";
	}
	// menu section
	App1->Image1 = App1->loadImage((char*)"images\\GUI\\menu.png");
	if (!App1->Image1) {
		plog << "Error: Failed menu (?)\n";
	}
	SDL_SetRenderDrawColor(App1->renders, 255, 255, 255, SDL_ALPHA_OPAQUE);
	while (!quit && !menu) {
		SDL_PollEvent(&e);
		switch (e.type) {
		case SDL_QUIT:
			quit = 1;
			break;
		case SDL_MOUSEBUTTONDOWN:
			SDL_GetMouseState(&mx, &my);
			if (mx >= 386 && mx <= 614) {
				if (my >= 173 && my <= 228) {
					menu = 1;
					Mix_PlayChannel(-1, App1->quirk, 0);
				}
				else if (my >= 273 && my <= 328) {
					menu = 2;
					Mix_PlayChannel(-1, App1->quirk, 0);
				}
			}
			break;
		default:
			if (!(SDL_GetTicks() % 15)) { // realization with (time mod [1000/fps]) ~62.5 fps
				SDL_RenderClear(App1->renders);
				App1->renderer(App1->Image1);
				SDL_RenderPresent(App1->renders);
			}
		}
	}
	if (quit) {
		App1->KillAll();
		return nullptr;
	}
	// sheet section
	if (menu == 2) {
		char rows[10]; char cols[10]; int r; int c;
		ifstream readfile;
		readfile.open("config.ini");
		if (!readfile) {
			plog << "Error: config.ini needs to be with your .exe";
			return nullptr;
		}
		readfile.ignore(1000, '\n');
		readfile.getline(rows, 10);
		readfile.getline(cols, 10);
		readfile.close();
		r = atoi(rows); c = atoi(cols);
		if (r && c) {
			separate(LIST_Len, r, c);
			plog << LIST_Len << " files were created in \"images\" folder\n";
			return App1;
		}
		App1->Image1 = App1->loadImage((char *)"images\\GUI\\sheetalg.png");
		App1->Button1 = App1->loadImage((char *)"images\\GUI\\bigokbutton.png");
		if (!App1->Image1 || !App1->Button1) {
			plog << "Failed sheet image (?)\n";
			return nullptr;
		}
		else {
			buttonwidth = App1->Button1->w / 2;
			buttonheight = App1->Button1->h / 2;
		}
		while (!quit) {
			SDL_PollEvent(&e);
			switch (e.type) {
			case SDL_QUIT:
				quit = 1;
			case SDL_MOUSEBUTTONDOWN:
				SDL_GetMouseState(&mx, &my);
				if (mouseinrange(mx, my, buttonwidth, buttonheight, 573, 461)) {
					Mix_PlayChannel(-1, App1->quirk, 0);
					return App1;
				}
			default:
				if (!(SDL_GetTicks() % 15)) { // realization with (time mod [1000/fps]) ~62.5 fps
					SDL_RenderClear(App1->renders);
					App1->renderer(App1->Image1);
					App1->renderer(App1->Button1, 573, 461, buttonwidth, buttonheight, 1);
					SDL_RenderPresent(App1->renders);
				}
			}
		}
		App1->KillAll();
		return nullptr;
	}
	// sort section
	App1->Image1 = App1->loadImage(generatePath(DR.returnleft()+1, path));
	App1->Image2 = App1->loadImage(generatePath(DR.returnright()+1, path));
	App1->Button1 = App1->loadImage((char*)"images\\GUI\\bigundobutton.png");
	App1->ButtonSave = App1->loadImage((char *)"images\\GUI\\savebutton.png");
	App1->ButtonLoad = App1->loadImage((char *)"images\\GUI\\loadbutton.png");
	buttonwidth = App1->Button1->w / 2;
	buttonheight = App1->Button1->h / 2;
	int choice = -1;
	if (!App1->Image1 || !App1->Image2 || !App1->Button1 || !App1->ButtonSave || !App1->ButtonLoad) {
		plog << "Error: Can't open images\n";
	}
	else {
		while (!quit) {
			SDL_PollEvent(&e);
			switch (e.type) {
			case SDL_QUIT:
				quit = 1;
				break;
			case SDL_MOUSEBUTTONDOWN:
				SDL_GetMouseState(&mx, &my);
				if (mouseinrange(mx, my, buttonwidth, buttonheight, SWIDTH / 2, SHEIGHT - 35)) {
					plog << "Button detected\n";
					if (DR.sort_undo()) {
						Mix_PlayChannel(-1, App1->quirk2, 0);
					}
				}
				else if (mouseinrange(mx, my, buttonwidth, buttonheight, SWIDTH / 4, SHEIGHT - 35)) {
					plog << "Save button detected\n";
					DR.save();
					Mix_PlayChannel(-1, App1->quirk2, 0);
				}
				else if (mouseinrange(mx, my, buttonwidth, buttonheight, 3 * SWIDTH / 4, SHEIGHT - 35)) {
					plog << "Load button detected\n";
					DR.load();
					Mix_PlayChannel(-1, App1->quirk2, 0);
				}
				else {
					if (mx < SWIDTH / 2 - 5 && my < SHEIGHT-75) {
						choice = 0;
						Mix_PlayChannel(-1, App1->quirk, 0);
					}
					else if (mx > SWIDTH / 2 + 5 && my < SHEIGHT-75) {
						choice = 1;
						Mix_PlayChannel(-1, App1->quirk, 0);
					}
				}
				DR.SortingIter(choice);
				choice = -1;
				if (!DR.status) {
					App1->Image1 = App1->loadImage(generatePath(DR.returnleft() + 1, path));
					App1->Image2 = App1->loadImage(generatePath(DR.returnright() + 1, path));
					plog << "Doing (" << DR.returnleft() << "), (" << DR.returnright() << ") choice\n";
				}
				else { quit = 1; }
				break;
			default:
				if (!(SDL_GetTicks() % 15)) { // realization with (time mod [1000/fps]) ~62.5 fps
					SDL_SetRenderDrawColor(App1->renders, 255, 255, 255, SDL_ALPHA_OPAQUE);
					SDL_RenderClear(App1->renders);
					SDL_SetRenderDrawColor(App1->renders, 0, 0, 0, SDL_ALPHA_OPAQUE);
					SDL_RenderDrawLine(App1->renders, SWIDTH / 2, 0, SWIDTH / 2, SHEIGHT - 75);
					SDL_RenderDrawLine(App1->renders, 0, SHEIGHT - 75, SWIDTH, SHEIGHT - 75);
					App1->renderer(App1->Image1, SWIDTH / 4, SHEIGHT / 2, 300, 300, 1);
					App1->renderer(App1->Image2, 3 * SWIDTH / 4, SHEIGHT / 2, 300, 300, 1);
					App1->renderer(App1->Button1, SWIDTH / 2, SHEIGHT - 35, buttonwidth, buttonheight, 1);
					App1->renderer(App1->ButtonSave, SWIDTH / 4, SHEIGHT - 35, buttonwidth, buttonheight, 1);
					App1->renderer(App1->ButtonLoad, 3 * SWIDTH / 4, SHEIGHT - 35, buttonwidth, buttonheight, 1);
					SDL_RenderPresent(App1->renders);
				}
			}
		}
		// end of sorting, results
		if (DR.status) {
			CreateResults(DR);
			quit = 0;
			SDL_SetRenderDrawColor(App1->renders, 255, 255, 255, SDL_ALPHA_OPAQUE);
			App1->Image1 = App1->loadImage((char*)"images\\GUI\\success.png");
			App1->Button1 = App1->loadImage((char *)"images\\GUI\\bigokbutton.png");
			while (!quit) {
				SDL_PollEvent(&e);
				switch (e.type) {
				case SDL_QUIT:
					quit = 1;
				case SDL_MOUSEBUTTONDOWN:
					SDL_GetMouseState(&mx, &my);
					if (mouseinrange(mx, my, buttonwidth, buttonheight, 573, 461)) {
						Mix_PlayChannel(-1, App1->quirk, 0);
						return App1;
					}
				default:
					if (!(SDL_GetTicks() % 15)) { // realization with (time mod [1000/fps]) ~62.5 fps
						SDL_RenderClear(App1->renders);
						App1->renderer(App1->Image1);
						App1->renderer(App1->Button1, 573, 461, buttonwidth, buttonheight, 1);
						SDL_RenderPresent(App1->renders);
					}
				}
			}
		}
	}
	App1->KillAll();
	return nullptr;
}

bool inouttest() {
	FIBITMAP *img1;
	try {
		img1 = FreeImage_Load(FIF_PNG, "giantimage.png", PNG_DEFAULT);
		FreeImage_Save(FIF_PNG, img1, "images\\GUI\\test.png", PNG_DEFAULT);
		img1 = FreeImage_Load(FIF_PNG, "images\\GUI\\test.png", PNG_DEFAULT);
	}
	catch (...) {
		plog << "PNG Problems: " << endl;
		return 1;
	}
	return 0;
}

void CreateResults(Sort &sort) {
	int w = 0; int h = 0;
	int sqr = 1 + (int)sqrt(LIST_Len);
	char path[50];
	int basew = 0, baseh = 0;
	FIBITMAP *res;
	try {
		FIBITMAP **all = new FIBITMAP * [LIST_Len];
		for (int i = 0; i < LIST_Len; i++) {
			generatePath(i + 1, path);
			all[i] = FreeImage_Load(FIF_PNG, path, PNG_DEFAULT);
			if (!w || !h) {
				w = FreeImage_GetWidth(all[i]) * sqr;
				h = FreeImage_GetHeight(all[i]) * sqr;
			}
		}
		res = FreeImage_Allocate(w, h, 32, 0, 0, 0);
		SortList Res = sort.getFinalSort();
		basew = FreeImage_GetWidth(all[0]);
		baseh = FreeImage_GetHeight(all[0]);
		for (int i = 0; i < LIST_Len; i++) {
			//res.composite(all[Res.getelem(i)], all[0].columns() * (i % sqr), (i / sqr) * all[0].rows());
			cout << FreeImage_Paste(res, all[Res.getelem(i)], basew * (i % sqr), baseh * (i / sqr), 255);
		}
		FreeImage_Save(FIF_PNG, res, "result.png", PNG_DEFAULT);
		plog << "Result.png was created!\n";
		for (int i = 0; i < LIST_Len; i++) {
			FreeImage_Unload(all[i]);
		}
		delete[] all;
		FreeImage_Unload(res);
	}
	catch (...) {
		plog << "Error: Problem with either \"images/N.png\" files or \"result\".png!\n";
	}
}

bool mouseinrange(int &mx, int &my, int w, int h, int posx, int posy) {
	if (mx <= posx + w / 2 && mx >= posx - w / 2 && my >= posy - h / 2 && my <= posy + h / 2) {
		return 1;
	}
	return 0;
}

char* generatePath(int num, char* to) {
	char temp[20];
	string path = "images\\";
	path.append(to_string(num));
	path.append(".png");
	strcpy_s(temp, path.c_str());
	for (int i = 0; i < 20; i++) {
		to[i] = temp[i];
	}
	return to;
}

void Fill(int *arr, int len) {
	for (int i = 0; i < len; i++) {
		*arr = i;
		arr++;
	}
}

void separate(int size, int row, int columns) {
	FIBITMAP **imgset = new FIBITMAP * [size];
	FIBITMAP *from = FreeImage_Load(FIF_PNG, "giantimage.png", PNG_DEFAULT);
	if (columns == 0 || row == 0) {
		plog << "Cols, rows cannot be 0" << endl;
		return;
	}
	int width = FreeImage_GetWidth(from) / columns;
	int height = FreeImage_GetHeight(from) / row;
	if (width == 0 || height == 0) {
		plog << "Imgsize cannot be 0" << endl;
		return;
	}
	try {
		char path[50];
		for (int i = 0; i < size; i++) {
			imgset[i] = FreeImage_Copy(from, width * (i % columns), (i / columns) * height, width * (i % columns) + width, (i / columns) * height + height);
			generatePath(i + 1, path);
			FreeImage_Save(FIF_PNG, imgset[i], path, PNG_DEFAULT);
		}
	}
	catch (...)
	{
		plog << "Caught exception. " << endl;
	}
	FreeImage_Unload(from);
	for (int i = 0; i < size; i++) {
		FreeImage_Unload(imgset[i]);
	}
	delete[] imgset;
}