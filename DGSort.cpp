//#define SDL_MAIN_HANDLED
const int SWIDTH = 640;
const int SHEIGHT = 480;
const char App_Name[] = "Things Sorting Machine";

#include <string>
#include <SDL2/SDL.h>
#include <SDL2/FreeImage.h>
#include <SDL2/SDL_mixer.h>
#include <fstream>
#include "SDLApp.h"
#include "SortMachine.h"

using namespace std;

//basic functions
void separate(int size, int row, int columns); // size - amount of images, row - ysqrnum, columns - xsqrnum
char* generatePath(int num, char* to);
bool inouttest();
bool mouseinrange(int &mx, int &my, int w, int h, int posx, int posy); // is your mouse here?

//int LIST_Len; // SORT NUMBER (num of images)
ofstream plog;

void CreateResults(Sort& sort);

class Application {
public:
	Application() {
		status = 0; quit = 0; mx = 0; my = 0; buttonwidth = 0; buttonheight = 0;
		DR = nullptr; App1 = nullptr;
		e = nullptr; plog = nullptr;
		path[0] = '\0';
	}
	int Init(int LIST_Len, ofstream &plog);
	int Routine();
private:
	int menu_section();
	int sheet_section();
	int sort_section();
	int res_section();
	int status;
	bool quit;
	int mx, my;
	int buttonwidth;
	int buttonheight;
	char path[20];
	Sort *DR;
	SDL_Event *e;
	SDLBase *App1;
	ofstream *plog;
};

int main(int argc, char* argv[]) {
	char read[10];
	int LIST_Len;
	int output;
	Application a1;
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
		output = a1.Init(LIST_Len, plog);
		if (output == 1) {
			break;
		}
		output = a1.Routine();
	} while (output == 1);
	plog.close();
	return 0;
}

int Application::Init(int LIST_Len, ofstream &plog) {
	status = 0; quit = 0; mx = 0; my = 0; buttonwidth = 0; buttonheight = 0;
	this->plog = &plog;
	try {
		if (DR != nullptr) {
			delete DR;
		}
		DR = new Sort(LIST_Len, plog);
		if (e != nullptr) {
			delete e;
		}
		e = new SDL_Event;
	}
	catch (...) {
		plog << "Application: smth happened" << endl;
		return 1;
	}
	return 0;
}

int Application::Routine() {
	int code;
	if (App1 == nullptr) {
		App1 = new SDLBase(*plog);
	}
	if (!App1->get_success_state()) {
		*plog << "Error: Application cannot be run\n";
		return 0;
	}
	try {
		// menu section
		code = menu_section();
		// sheet section
		if (status == 1) {
			code = sheet_section();
		}
		// sort section
		if (status == 2) {
			code = sort_section();
			if (DR->get_status()) {
				code = res_section();
			}
		}
	}
	catch (int i) { // 0 - close, 1 - restart
		return i;
	}
	App1->KillAll();
	return 0;
}

int Application::menu_section() {
	App1->Image1 = App1->loadImage((char *)"images\\GUI\\menu.png");
	if (!App1->Image1) {
		*plog << "Error: Failed menu (?)\n";
	}
	SDL_SetRenderDrawColor(App1->renders, 255, 255, 255, SDL_ALPHA_OPAQUE);
	while (!quit && status == 0) {
		SDL_PollEvent(e);
		switch (e->type) {
		case SDL_QUIT:
			quit = 1;
			break;
		case SDL_MOUSEBUTTONDOWN:
			SDL_GetMouseState(&mx, &my);
			if (mx >= 386 && mx <= 614) {
				if (my >= 173 && my <= 228) {
					status = 2;
					Mix_PlayChannel(-1, App1->quirk, 0);
				}
				else if (my >= 273 && my <= 328) {
					status = 1;
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
		throw 0;
	}
	return 2;
}
int Application::sheet_section() {
	char rows[10]; char cols[10]; int r; int c;
	ifstream readfile;
	readfile.open("config.ini");
	if (!readfile) {
		*plog << "Error: config.ini needs to be with your .exe";
		throw 0;
	}
	readfile.ignore(1000, '\n');
	readfile.getline(rows, 10);
	readfile.getline(cols, 10);
	readfile.close();
	r = atoi(rows); c = atoi(cols);
	if (r && c) {
		separate(DR->get_size(), r, c);
		*plog << DR->get_size() << " files were created in \"images\" folder\n";
		throw 1;
	}
	App1->Image1 = App1->loadImage((char *)"images\\GUI\\sheetalg.png");
	App1->Button1 = App1->loadImage((char *)"images\\GUI\\bigokbutton.png");
	if (!App1->Image1 || !App1->Button1) {
		*plog << "Failed sheet image (?)\n";
		throw 0;
	}
	else {
		buttonwidth = App1->Button1->w / 2;
		buttonheight = App1->Button1->h / 2;
	}
	while (!quit) {
		SDL_PollEvent(e);
		switch (e->type) {
		case SDL_QUIT:
			quit = 1;
		case SDL_MOUSEBUTTONDOWN:
			SDL_GetMouseState(&mx, &my);
			if (mouseinrange(mx, my, buttonwidth, buttonheight, 573, 461)) {
				Mix_PlayChannel(-1, App1->quirk, 0);
				throw 1;
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
	throw 0;
}
int Application::sort_section() {
	App1->Image1 = App1->loadImage(generatePath(DR->returnleft() + 1, path));
	App1->Image2 = App1->loadImage(generatePath(DR->returnright() + 1, path));
	App1->Button1 = App1->loadImage((char *)"images\\GUI\\bigundobutton.png");
	App1->ButtonSave = App1->loadImage((char *)"images\\GUI\\savebutton.png");
	App1->ButtonLoad = App1->loadImage((char *)"images\\GUI\\loadbutton.png");
	buttonwidth = App1->Button1->w / 2;
	buttonheight = App1->Button1->h / 2;
	int choice = -1;
	if (!App1->Image1 || !App1->Image2 || !App1->Button1 || !App1->ButtonSave || !App1->ButtonLoad) {
		*plog << "Error: Can't open images\n";
		throw 0;
	}
	while (!quit) {
		SDL_PollEvent(e);
		switch (e->type) {
		case SDL_QUIT:
			quit = 1;
			break;
		case SDL_MOUSEBUTTONDOWN:
			SDL_GetMouseState(&mx, &my);
			if (mouseinrange(mx, my, buttonwidth, buttonheight, SWIDTH / 2, SHEIGHT - 35)) {
				*plog << "Button detected\n";
				if (DR->sort_undo()) {
					Mix_PlayChannel(-1, App1->quirk2, 0);
				}
			}
			else if (mouseinrange(mx, my, buttonwidth, buttonheight, SWIDTH / 4, SHEIGHT - 35)) {
				*plog << "Save button detected\n";
				DR->save();
				Mix_PlayChannel(-1, App1->quirk2, 0);
			}
			else if (mouseinrange(mx, my, buttonwidth, buttonheight, 3 * SWIDTH / 4, SHEIGHT - 35)) {
				*plog << "Load button detected\n";
				DR->load();
				Mix_PlayChannel(-1, App1->quirk2, 0);
			}
			else {
				if (mx < SWIDTH / 2 - 5 && my < SHEIGHT - 75) {
					choice = 0;
					Mix_PlayChannel(-1, App1->quirk, 0);
				}
				else if (mx > SWIDTH / 2 + 5 && my < SHEIGHT - 75) {
					choice = 1;
					Mix_PlayChannel(-1, App1->quirk, 0);
				}
			}
			DR->SortingIter(choice);
			choice = -1;
			if (!DR->get_status()) {
				App1->Image1 = App1->loadImage(generatePath(DR->returnleft() + 1, path));
				App1->Image2 = App1->loadImage(generatePath(DR->returnright() + 1, path));
				*plog << "Doing (" << DR->returnleft() << "), (" << DR->returnright() << ") choice\n";
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
	return 2;
}
int Application::res_section() {
	CreateResults(*DR);
	quit = 0;
	SDL_SetRenderDrawColor(App1->renders, 255, 255, 255, SDL_ALPHA_OPAQUE);
	App1->Image1 = App1->loadImage((char *)"images\\GUI\\success.png");
	App1->Button1 = App1->loadImage((char *)"images\\GUI\\bigokbutton.png");
	while (!quit) {
		SDL_PollEvent(e);
		switch (e->type) {
		case SDL_QUIT:
			quit = 1;
		case SDL_MOUSEBUTTONDOWN:
			SDL_GetMouseState(&mx, &my);
			if (mouseinrange(mx, my, buttonwidth, buttonheight, 573, 461)) {
				Mix_PlayChannel(-1, App1->quirk, 0);
				throw 1;
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
	return 2;
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
	int LIST_Len = sort.get_size();
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
			plog << Res.getelem(i) << " - " << FreeImage_Paste(res, all[Res.getelem(i)], basew * (i % sqr), baseh * (i / sqr), 255) << endl;
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