//#define SDL_MAIN_HANDLED
const int SWIDTH = 640;
const int SHEIGHT = 480;
const char App_Name[] = "Things Sorting Machine";

#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/FreeImage.h>
#include <SDL2/SDL_mixer.h>
#include <fstream>
#include <vector>
#include "SDLApp.h"

using namespace std;

//basic functions
void separate(int size, int row, int columns); // size - amount of images, row - ysqrnum, columns - xsqrnum
void Fill(int *arr, int len);
char* generatePath(int num, char* to);
bool inouttest();
bool mouseinrange(int &mx, int &my, int w, int h, int posx, int posy); // is your mouse here?

//int LIST_Len; // SORT NUMBER (num of images)
ofstream plog;

class SortList {
public:
	SortList(int len) {
		SortList1 = new int[len];
		SortLen = 0;
		SortMaxLen = len;
		for (int i = 0; i < SortMaxLen; i++) {
			SortList1[i] = -1;
		}
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
	SortList& operator= (const SortList& list);
	bool operator==(const SortList &list) const {
		if (list.SortLen != SortLen || list.SortMaxLen != SortMaxLen) {
			return 0;
		}
		for (int i = 0; i < SortLen; i++) {
			if (SortList1[i] != list.SortList1[i]) {
				return 0;
			}
		}
		return 1;
	}
	int length() const { return SortLen; }
	int getelem(int n) const {
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
	void print(ofstream &plog) const {
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
	SortList slice(int start, int end) const;
private:
	int *SortList1;
	int SortLen;
	int SortMaxLen;
};

SortList SortList::slice(int start, int end) const {
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

SortList& SortList::operator= (const SortList& list) {
	if (list == *this) {
		return *this;
	}
	if (SortMaxLen != list.SortMaxLen) {
		delete[] SortList1;
		SortMaxLen = list.SortMaxLen;
		SortList1 = new int[SortMaxLen];
	}
	SortLen = 0;
	for (int i = 0; i < list.length(); i++) {
		setelem(i, list.getelem(i));
	}
	return *this;
}

class Sort {
public:
	Sort(int LIST_Len, ofstream& plog): sortlist(LIST_Len*2, SortList(LIST_Len)) {
		status = 0;
		pointer = 1;
		sortlistlen = 1;
		TempList = new int[LIST_Len];
		RecordList = new int[LIST_Len];
		parentlist = new int[LIST_Len*2];
		basic_size = LIST_Len;
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
		sortlist.clear();
		delete[] RecordList;
		delete[] TempList;
	}
	int returnleft() { return sortlist[leftlist].getelem(leftID); };
	int returnright() { return sortlist[rightlist].getelem(rightID); };
	void SortingIter(int choice);
	SortList getFinalSort() const {
		return sortlist[0];
	}
	bool sort_undo();
	void printfulllist(int len) const;
	void printsortlist() const;
	void save();
	void load();
	bool get_status() const {
		return status;
	}
	int get_size() const {
		return basic_size;
	}
private:
	void countup(int choice); // 0 - left, 1 - right
	void Fill(int *arr, int len) {
		for (int i = 0; i < len; i++) {
			*arr = i;
			arr++;
		}
	}
	int pointer;
	int *TempList;
	vector<SortList> sortlist;
	int sortlistlen;
	int *parentlist;
	int leftlist, rightlist;
	int leftID, rightID, recordID;
	int *RecordList;
	bool status;
	int basic_size;
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
	for (int i = 0; i < basic_size; i++) {
		data[pnt + i] = RecordList[i];
	}
	pnt += basic_size;
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
	for (int h = 0; h < basic_size; h++) {
		RecordList[h] = data[pnt];
		pnt++;
	} // recordlist filling
	plog << "Loaded:\n";
	for (int i1 = 0; i1 < 6; i1++) {
		plog << *(ptrs[i1]) << ' ';
	}
	printsortlist();
	for (int l = 0; l < basic_size; l++) {
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

void Sort::printsortlist() const {
	plog << "[";
	for (int i = 0; i < sortlistlen; i++) {
		sortlist[i].print(plog);
		if (i != sortlistlen - 1) {
			plog << ", ";
		}
		else {
			plog << "]\n";
		}
	}
	plog << "Length: " << sortlistlen << '\n';
}

void Sort::printfulllist(int len) const {
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
			for (int i = 0; i < basic_size; i++) {
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

void CreateResults(Sort& sort);
SDLBase *call_app(SDLBase *curapp, int LIST_Len);

int main(int argc, char* argv[]) {
	char read[10];
	int LIST_Len;
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
		output = call_app(output, LIST_Len);
	} while (output != nullptr);
	plog.close();
	return 0;
}

SDLBase* call_app(SDLBase* curapp, int LIST_Len) {
	int menu = 0;
	bool quit = 0;
	int mx, my;
	int buttonwidth;
	int buttonheight;
	char path[20];
	Sort DR(LIST_Len, plog);
	SDL_Event e;
	SDLBase *App1;
	if (curapp == nullptr) {
		App1 = new SDLBase(plog);
	}
	else {
		App1 = curapp;
	}
	if (!App1->get_success_state()) {
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
				if (!DR.get_status()) {
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
		if (DR.get_status()) {
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