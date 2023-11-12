#define SDL_MAIN_HANDLED
const int SWIDTH = 640;
const int SHEIGHT = 480;
const char App_Name[] = "Things Sorting Machine";

#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include <fstream>
#include <nfd/nfd.h>
#include "SDLApp.h"
#include "SortMachine.h"

using namespace std;

//basic functions
void separate(int size, int row, int columns); // size - amount of images, row - ysqrnum, columns - xsqrnum
char* generatePath(int num, char* to);
bool mouseinrange(int &mx, int &my, int w, int h, int posx, int posy); // is your mouse here?
void init_table(FIBITMAP* &img, int place, int width); // generate table with a number
vector<int> num_to_vector(int num);
FIBITMAP *num_to_image(int num);
void saveresized(vector<FIBITMAP *> &imgs);
int getlenfromfile();

//int LIST_Len; // SORT NUMBER (num of images)
ofstream plog;

void CreateResults(Sort& sort);
void CreateResultsNumeric(Sort &sort);

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
	int single_section();
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
	int LIST_Len;
	int output;
	Application a1;
	plog.open("log.txt", ios::app);
	setlocale(0, "");
	SDL_SetMainReady();
	try {
		FreeImage_Initialise();
	}
	catch (...) {
		plog << "FreeImage not working.\n";
		return 1;
	}
	plog << "Starting App...\n";
	do {
		try {
			LIST_Len = getlenfromfile();
		}
		catch (int i) {
			return 1;
		}
		output = a1.Init(LIST_Len, plog);
		if (output == 1) {
			break;
		}
		output = a1.Routine();
	} while (output == 1);
	plog.close();
	return 0;
}

int getlenfromfile() {
	int LIST_Len;
	char read[10];
	fstream file1;
	plog << "\n---Neiro's Things Sorting Machine---\n";
	file1.open("config.ini");
	if (!file1 || !plog) {
		plog << "Error: config.ini and log.txt need to be with your .exe";
		fstream file2;
		file2.open("config.ini", ios::app);
		file2.close();
		throw 1;
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
	return LIST_Len;
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
		// sort section
		if (status == 1) {
			code = sort_section();
			if (DR->get_status()) {
				code = res_section();
			}
		}
		// sort section
		if (status == 2) {
			code = sheet_section();
		}
		if (status == 3) {
			code = single_section();
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
				if (my >= 148 && my <= 203) {
					status = 1;
					Mix_PlayChannel(-1, App1->quirk, 0);
				}
				else if (my >= 248 && my <= 303) {
					status = 2;
					Mix_PlayChannel(-1, App1->quirk, 0);
				}
				else if (my >= 348 && my <= 403) {
					status = 3;
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
	if (r && c && (r*c>=DR->get_size())) {
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
	if (DR->get_size() < 2) {
		throw 1;
	}
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
		//default:
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
	CreateResultsNumeric(*DR);
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

int Application::single_section() {
	int i = 0;
	nfdpathset_t *paths = new nfdpathset_t;
	nfdresult_t ret;
	vector<FIBITMAP *> images;
	SDL_SetRenderDrawColor(App1->renders, 255, 255, 255, SDL_ALPHA_OPAQUE);
	FIBITMAP *num = num_to_image(i);
	App1->Image2 = App1->loadImage(num);
	App1->Image1 = App1->loadImage((char *)"images\\GUI\\single.png");
	App1->ButtonLoad = App1->loadImage((char *)"images\\GUI\\loadbutton.png");
	App1->Button1 = App1->loadImage((char *)"images\\GUI\\bigokbutton.png");
	if (!App1->Image1 || !App1->Button1 || !App1->ButtonLoad) {
		*plog << "Failed sheet image (?)\n";
		throw 0;
	}
	else {
		buttonwidth = App1->Button1->w;
		buttonheight = App1->Button1->h;
	}
	while (!quit) {
		SDL_PollEvent(e);
		switch (e->type) {
		case SDL_QUIT:
			quit = 1;
		case SDL_MOUSEBUTTONDOWN:
			SDL_GetMouseState(&mx, &my);
			if (mouseinrange(mx, my, buttonwidth, buttonheight, SWIDTH/2, SHEIGHT/2)) {
				Mix_PlayChannel(-1, App1->quirk, 0);
				ret = NFD_OpenDialogMultiple("png", "C:", paths);
				if (ret == 1) {
					string tmppath;
					wchar_t *inputstring;
					for (int j = 0; j < NFD_PathSet_GetCount(paths); j++) {
						// convert to wchar for other languages (windows only)
						tmppath = NFD_PathSet_GetPath(paths, j);
						inputstring = new wchar_t[tmppath.length()+1];
						size_t wstring_len;
						wstring_len = MultiByteToWideChar(CP_UTF8, 0, &tmppath[0], tmppath.length(), inputstring, tmppath.length());
						inputstring[wstring_len] = L'\0';
						try {
							images.push_back(FreeImage_LoadU(FIF_PNG, inputstring, PNG_DEFAULT));
						}
						catch (const exception &e) {
							*plog << "Exception: " << e.what() << endl;
						}
						delete[] inputstring;
					}
					Mix_PlayChannel(-1, App1->quirk, 0);
					i += NFD_PathSet_GetCount(paths);
					FreeImage_Unload(num);
					num = num_to_image(i);
					App1->Image2 = App1->loadImage(num);
				}
			}
			if (mouseinrange(mx, my, buttonwidth/2, buttonheight/2, 555, 444)) {
				Mix_PlayChannel(-1, App1->quirk, 0);
				ofstream readfile;
				readfile.open("config.ini");
				if (!readfile) {
					*plog << "Error: config.ini error";
					for (int j = 0; j < images.size(); j++) {
						FreeImage_Unload(images[j]);
					}
					throw 0;
				}
				readfile << i << endl;
				readfile.close();
				saveresized(images);
				for (int j = 0; j < images.size(); j++) {
					FreeImage_Unload(images[j]);
				}
				throw 1;
			}
		default:
			if (!(SDL_GetTicks() % 15)) { // realization with (time mod [1000/fps]) ~62.5 fps
				SDL_RenderClear(App1->renders);
				App1->renderer(App1->Image1);
				App1->renderer(App1->ButtonLoad, SWIDTH/2, SHEIGHT/2, buttonwidth, buttonheight, 1);
				App1->renderer(App1->Button1, 555, 444, buttonwidth/2, buttonheight/2, 1);
				App1->renderer(App1->Image2, 355, 328, App1->Image2->w/2, App1->Image2->h/2, 1);
				SDL_RenderPresent(App1->renders);
			}
		}
	}
	for (int j = 0; j < images.size(); j++) {
		FreeImage_Unload(images[j]);
	}
	delete paths;
}

void saveresized(vector<FIBITMAP *> &imgs) {
	FIBITMAP *tmp;
	int sum_size = 0;
	int tmpw, tmph;
	char path[50];
	if (imgs.size() == 0) {
		return;
	}
	for (int i = 0; i < imgs.size(); i++) {
		tmpw = FreeImage_GetWidth(imgs[i]);
		tmph = FreeImage_GetHeight(imgs[i]);
		if (tmpw < tmph) {
			sum_size += tmpw;
		}
		else {
			sum_size += tmph;
		}
	}
	sum_size /= imgs.size();
	try {
		for (int i = 0; i < imgs.size(); i++) {
			tmp = FreeImage_Rescale(imgs[i], sum_size, sum_size, FILTER_BILINEAR);
			generatePath(i + 1, path);
			FreeImage_Save(FIF_PNG, tmp, path);
			FreeImage_Unload(tmp);
		}
	}
	catch (...) {
		plog << "Saveresized caught exception" << endl;
	}
}

void CreateResults(Sort &sort) {
	int LIST_Len = sort.get_size();
	int w = 0; int h = 0;
	int sqr = 1 + (int)sqrt(LIST_Len-1);
	char path[50];
	int basew = 0, baseh = 0;
	FIBITMAP *res;
	try {
		FIBITMAP **all = new FIBITMAP * [LIST_Len];
		for (int i = 0; i < LIST_Len; i++) {
			generatePath(i + 1, path);
			all[i] = FreeImage_Load(FIF_PNG, path, PNG_DEFAULT);
			if (!w || !h) {
				basew = FreeImage_GetWidth(all[i]);
				w = FreeImage_GetWidth(all[i]) * sqr;
				baseh = FreeImage_GetHeight(all[i]);
				h = FreeImage_GetHeight(all[i]) * sqr;
			}
		}
		res = FreeImage_Allocate(w, h, 32, 0, 0, 0);
		SortList Res = sort.getFinalSort();
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

void CreateResultsNumeric(Sort &sort) {
	int LIST_Len = sort.get_size();
	int w = 0; int h = 0;
	int sqr = 1 + (int)sqrt(LIST_Len-1);
	char path[50];
	int basew = 0, baseh = 0;
	int tablw = 0, tablh = 0;
	FIBITMAP *res;
	try {
		FIBITMAP **all = new FIBITMAP * [LIST_Len];
		FIBITMAP **alltabl = new FIBITMAP * [LIST_Len];
		for (int i = 0; i < LIST_Len; i++) {
			generatePath(i + 1, path);
			all[i] = FreeImage_Load(FIF_PNG, path, PNG_DEFAULT);
			if (!w || !h) {
				w = FreeImage_GetWidth(all[i]) * sqr;
				basew = FreeImage_GetWidth(all[i]);
				h = FreeImage_GetHeight(all[i]) * sqr;
				baseh = FreeImage_GetHeight(all[i]);
			}
		}
		tablw = basew;
		try {
			for (int i = 0; i < LIST_Len; i++) {
				init_table(alltabl[i], i + 1, tablw);
				if (!tablh) {
					tablh = FreeImage_GetHeight(alltabl[i]);
				}
			}
		}
		catch (...) {
			plog << "Table Geneation: Smth happened!" << endl;
		}
		h += sqr * tablh;
		baseh += tablh;
		res = FreeImage_Allocate(w, h, 32, 0, 0, 0);
		SortList Res = sort.getFinalSort();
		for (int i = 0; i < LIST_Len; i++) {
			//res.composite(all[Res.getelem(i)], all[0].columns() * (i % sqr), (i / sqr) * all[0].rows());
			plog << Res.getelem(i) << " - " << FreeImage_Paste(res, alltabl[i], basew * (i % sqr), baseh * (i / sqr), 255) << " - ";
			plog << FreeImage_Paste(res, all[Res.getelem(i)], basew * (i % sqr), baseh * (i / sqr) + tablh, 255) << endl;
		}
		FreeImage_Save(FIF_PNG, res, "result.png", PNG_DEFAULT);
		plog << "Result.png was created!\n";
		for (int i = 0; i < LIST_Len; i++) {
			FreeImage_Unload(alltabl[i]);
			FreeImage_Unload(all[i]);
		}
		delete[] alltabl;
		delete[] all;
		FreeImage_Unload(res);
	}
	catch (...) {
		plog << "Error: Problem with either \"images/N.png\" files or \"result\".png!\n";
	}
}

void init_table(FIBITMAP* &img, int place, int width) {
	vector<int> truenum;
	FIBITMAP *nums = FreeImage_Load(FIF_PNG, "images\\GUI\\numbers.png", PNG_DEFAULT);
	FIBITMAP *block_of_nums;
	int numsw = FreeImage_GetWidth(nums);
	double numsw_each = numsw / 10;
	int numsh = FreeImage_GetHeight(nums);
	int tablw = numsw * 1.056; // special
	int tablh = numsh / 0.65; // proportion
	FIBITMAP *tempmask = FreeImage_Allocate(tablw, tablh, 32, 0, 0, 0);
	FIBITMAP *res = FreeImage_Load(FIF_PNG, "images\\GUI\\boxexample.png", PNG_DEFAULT);
	res = FreeImage_Rescale(res, tablw, tablh, FILTER_BILINEAR);
	int coordy = (tablh - numsh) / 2; // center
	truenum = num_to_vector(place);
	int digits_count = truenum.size();
	int coordx = (tablw - digits_count * numsw_each) / 2;
	block_of_nums = num_to_image(place);
	FreeImage_Paste(tempmask, block_of_nums, coordx, coordy, 255);
	res = FreeImage_Composite(tempmask, 0, 0, res);
	res = FreeImage_Rescale(res, width, (int)tablh * width / tablw, FILTER_BILINEAR);
	FreeImage_Unload(block_of_nums);
	FreeImage_Unload(tempmask);
	FreeImage_Unload(nums);
	img = res;
}

FIBITMAP *num_to_image(int num) {
	FIBITMAP *nums = FreeImage_Load(FIF_PNG, "images\\GUI\\numbers.png", PNG_DEFAULT);
	FIBITMAP *curdigit;
	double numsw_each = FreeImage_GetWidth(nums)/10;
	int numsh = FreeImage_GetHeight(nums);
	FIBITMAP *res;
	vector<int> truenum = num_to_vector(num);
	int digits_count = truenum.size();
	if (digits_count > 10) {
		plog << "Table: Problem with sizing" << endl;
	}
	res = FreeImage_Allocate(digits_count * numsw_each, numsh, 32, 0, 0, 0);
	for (int i = 0; i < digits_count; i++) {
		curdigit = FreeImage_Copy(nums, numsw_each * truenum[i], 0, numsw_each * (truenum[i] + 1), numsh);
		FreeImage_Paste(res, curdigit, numsw_each * i, 0, 255);
		FreeImage_Unload(curdigit);
	}
	FreeImage_Unload(nums);
	return res;
}

vector<int> num_to_vector(int num) {
	vector<int> res;
	int pos_10s_prev = 1;
	int pos_10s = 10;
	int mod1;
	if (num == 0) {
		return vector<int>(1,0);
	}
	while (num != 0) {
		mod1 = num % pos_10s;
		res.push_back(mod1/pos_10s_prev);
		num -= mod1;
		pos_10s *= 10;
		pos_10s_prev *= 10;
	}
	int N = res.size();
	for (int i = 0; i < N / 2; i++) {
		int tmp = res[i];
		res[i] = res[N - 1 - i];
		res[N - 1 - i] = tmp;
	}
	return res;
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