#define SDL_MAIN_HANDLED
const int SWIDTH = 640;
const int CURWIDTH = 1280;
const int SHEIGHT = 480;
const int CURHEIGHT = 960;
const int MAX_PATH_LEN = 50;
const char App_Name[] = "Things Sorting Machine";

#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include <fstream>
#include "nfd/nfd.h"
#include "SDLApp.h"
#include "SortMachine.h"

using namespace std;

//basic functions
void separate(int size, int row, int columns); // size - amount of images, row - ysqrnum, columns - xsqrnum
char* generate_path(int num, char* to);
char* generate_path_song(std::string song, char* to);
bool mouse_is_in_range_abs(int &mx, int &my, int x1, int x2, int y1, int y2); // is your mouse here?
bool mouse_is_in_range(int& mx, int& my, const RegularObject& obj); // is your mouse here?
void init_table(FIBITMAP* &img, int place, int width); // generate table with a number
vector<int> num_to_vector(int num);
FIBITMAP *num_to_image(int num);
void save_resized(vector<FIBITMAP *> &imgs);
int get_len_from_file();

//int LIST_Len; // SORT NUMBER (num of images)
ofstream plog;

struct SingleElement {
public:
	std::string song;
	std::string image;
	std::wstring text;
};

void init_text(FIBITMAP**& img, const std::vector<SingleElement>& data, std::string font, int w, int h); // generate nice textbox

void CreateResults(Sort& sort);
void CreateResultsNumeric(Sort &sort);
void CreateResultsNumericText(Sort& sort, std::string font, const std::vector<SingleElement>& data);

std::vector<SingleElement> get_songs_from_file(const char* path, int length) {
	ifstream readfile;
	char song_file[260];
	char text[260];
	wchar_t* inputstring;
	std::vector<SingleElement> result(length);
	readfile.open(path);
	if (!readfile) {
		throw 0;
	}
	for (int i = 0; i < length; i++) {
		readfile.getline(song_file, 260);
		readfile.getline(text, 260);
		readfile.ignore(1000, '\n');
		std::string text_s = text;
		// text may be unicode
		inputstring = new wchar_t[text_s.length() + 1];
		size_t wstring_len;
		wstring_len = MultiByteToWideChar(CP_UTF8, 0, &text_s[0], text_s.length(), inputstring, text_s.length());
		inputstring[wstring_len] = L'\0';
		//
		SingleElement temp;
		temp.image = std::to_string(i + 1);
		temp.song = song_file;
		temp.text = inputstring;
		delete[] inputstring;
		result[i] = temp;
	}
	readfile.close();
	return result;
}

class Application {
public:
	Application() {
		DR = nullptr; App1 = nullptr;
		e = nullptr; plog = nullptr;
		path[0] = '\0';
		path2[0] = '\0';
	}
	int Init(int LIST_Len, const std::vector<SingleElement>& elems, ofstream &plog);
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
	char path[MAX_PATH_LEN];
	char path2[2 * MAX_PATH_LEN];
	std::vector<SingleElement> elems;
	Sort *DR;
	SDL_Event *e;
	SDLBase *App1;
	ofstream *plog;
};

int main(int argc, char* argv[]) {
	int LIST_Len;
	std::vector<SingleElement> elems;
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
	try {
		TTF_Init();
	}
	catch (...) {
		plog << "TTF not working.\n";
		return 1;
	}
	plog << "Starting App...\n";
	do {
		try {
			LIST_Len = get_len_from_file();
			elems = get_songs_from_file("songs.ini", LIST_Len);
		}
		catch (int i) {
			if (i == 1) {
				elems = std::vector<SingleElement>();
				return 1;
			}
		}
		output = a1.Init(LIST_Len, elems, plog);
		if (output == 1) {
			break;
		}
		output = a1.Routine();
	} while (output == 1);
	plog.close();
	return 0;
}

int get_len_from_file() {
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

int Application::Init(int LIST_Len, const std::vector<SingleElement>& elems, ofstream &plog) {
	status = 0; quit = 0; mx = 0; my = 0;
	this->elems = elems;
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
	App1->clear_objects();
	bool menu_correct = App1->append_object(RegularObject((char*)"images\\GUI\\menu.png", "menu", 0, 0));
	if (!menu_correct) {
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
			// png menu
			if (mouse_is_in_range_abs(mx, my, 386, 614, 148, 203)) {
				status = 1;
				Mix_PlayChannel(-1, App1->quirk, 0);
			}
			else if (mouse_is_in_range_abs(mx, my, 386, 614, 248, 303)) {
				status = 2;
				Mix_PlayChannel(-1, App1->quirk, 0);
			}
			else if (mouse_is_in_range_abs(mx, my, 386, 614, 348, 403)) {
				status = 3;
				Mix_PlayChannel(-1, App1->quirk, 0);
			}
			//
			break;
		default:
			if (!(SDL_GetTicks() % 15)) { // realization with (time mod [1000/fps]) ~62.5 fps
				SDL_RenderClear(App1->renders);
				App1->render();
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
	App1->clear_objects();
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
	bool sheet_correct = App1->append_object(RegularObject((char*)"images\\GUI\\sheetalg.png", "sheet algo"));
	bool button_correct = App1->append_object(RegularObject((char*)"images\\GUI\\bigokbutton.png", "ok button"));
	if (!sheet_correct || !button_correct) {
		*plog << "Failed sheet image (?)\n";
		throw 0;
	}
	RegularObject& button = App1->get_regular("ok button");
	int buttonwidth = button.get_surface()->w / 2;
	int buttonheight = button.get_surface()->h / 2;
	button.set_rect(573, 461, buttonwidth, buttonheight, true);
	while (!quit) {
		SDL_PollEvent(e);
		switch (e->type) {
		case SDL_QUIT:
			quit = 1;
		case SDL_MOUSEBUTTONDOWN:
			SDL_GetMouseState(&mx, &my);
			if (mouse_is_in_range(mx, my, button)) {
				Mix_PlayChannel(-1, App1->quirk, 0);
				throw 1;
			}
		default:
			if (!(SDL_GetTicks() % 15)) { // realization with (time mod [1000/fps]) ~62.5 fps
				SDL_RenderClear(App1->renders);
				App1->render();
				SDL_RenderPresent(App1->renders);
			}
		}
	}
	App1->KillAll();
	throw 0;
}
int Application::sort_section() {
	//SDL_Text
	App1->clear_objects();
	if (DR->get_size() < 2) {
		throw 1;
	}
	std::string font = "times.ttf";
	SDL_Color black = { 0, 0, 0 };
	bool left_correct = App1->append_object(RegularObject(generate_path(DR->returnleft() + 1, path), "left picture", SWIDTH / 4, 5 * SHEIGHT / 12, 300, 300, true));
	bool right_correct = App1->append_object(RegularObject(generate_path(DR->returnright() + 1, path), "right picture", 3 * SWIDTH / 4, 5 * SHEIGHT / 12, 300, 300, true));
	bool undo_correct = App1->append_object(RegularObject((char*)"images\\GUI\\bigundobutton.png", "undo"));
	bool save_correct = App1->append_object(RegularObject((char*)"images\\GUI\\savebutton.png", "save"));
	bool load_correct = App1->append_object(RegularObject((char*)"images\\GUI\\loadbutton.png", "load"));
	bool text1_correct = true;
	bool text2_correct = true;
	if (elems.size() >= DR->get_size()) {
		text1_correct = App1->append_object(RegularObject(elems[DR->returnleft()].text, font, "text1", 24, black));
		text2_correct = App1->append_object(RegularObject(elems[DR->returnright()].text, font, "text2", 24, black));
	}
	int choice = -1;
	if (!left_correct || !right_correct || !undo_correct || !save_correct || !load_correct || !text1_correct || !text2_correct) {
		*plog << "Error: Can't open images or text\n";
		throw 0;
	}
	RegularObject& button = App1->get_regular("undo");
	int buttonwidth = button.get_surface()->w / 2;
	int buttonheight = button.get_surface()->h / 2;
	button.set_rect(SWIDTH / 2, SHEIGHT - 35, buttonwidth, buttonheight, true);
	App1->get_regular("save").set_rect(SWIDTH / 4, SHEIGHT - 35, buttonwidth, buttonheight, true);
	App1->get_regular("load").set_rect(3 * SWIDTH / 4, SHEIGHT - 35, buttonwidth, buttonheight, true);
	App1->get_regular("text1").set_rect_text(SWIDTH / 4, SHEIGHT - 95, SWIDTH / 2, true);
	App1->get_regular("text2").set_rect_text(3 * SWIDTH / 4, SHEIGHT - 95, SWIDTH / 2, true);
	while (!quit) {
		SDL_PollEvent(e);
		switch (e->type) {
		case SDL_QUIT:
			quit = 1;
			break;
		case SDL_MOUSEBUTTONDOWN:
		//default:
			SDL_GetMouseState(&mx, &my);
			if (mouse_is_in_range(mx, my, button)) {
				*plog << "Button detected\n";
				if (DR->sort_undo()) {
					Mix_PlayChannel(-1, App1->quirk2, 0);
				}
			}
			else if (mouse_is_in_range(mx, my, App1->get_regular("save"))) {
				*plog << "Save button detected\n";
				DR->save();
				Mix_PlayChannel(-1, App1->quirk2, 0);
			}
			else if (mouse_is_in_range(mx, my, App1->get_regular("load"))) {
				*plog << "Load button detected\n";
				DR->load();
				Mix_PlayChannel(-1, App1->quirk2, 0);
			}
			else {
				if (mouse_is_in_range_abs(mx, my, 0, SWIDTH / 2 - 5, 0, SHEIGHT - 55)) {
					if (e->button.button == 1) {
						choice = 0;
						Mix_PlayChannel(-1, App1->quirk, 0);
					}
					else if (elems.size() >= DR->get_size() && elems[DR->returnleft()].song != "none") {
						HINSTANCE result = ShellExecute(NULL, "open", generate_path_song(elems[DR->returnleft()].song, path2), NULL, NULL, SW_SHOWNORMAL);
					}
				}
				else if (mouse_is_in_range_abs(mx, my, SWIDTH / 2 + 5, SWIDTH, 0, SHEIGHT - 55)) {
					if (e->button.button == 1) {
						choice = 1;
						Mix_PlayChannel(-1, App1->quirk, 0);
					}
					else if (elems.size() >= DR->get_size() && elems[DR->returnright()].song != "none") {
						HINSTANCE result = ShellExecute(NULL, "open", generate_path_song(elems[DR->returnright()].song, path2), NULL, NULL, SW_SHOWNORMAL);
					}
				}
			}
			DR->SortingIter(choice);
			choice = -1;
			if (!DR->get_status()) {
				App1->get_regular("left picture").set_surface(generate_path(DR->returnleft() + 1, path));
				App1->get_regular("right picture").set_surface(generate_path(DR->returnright() + 1, path));
				if (elems.size() >= DR->get_size()) {
					App1->get_regular("text1").set_surface_text(elems[DR->returnleft()].text, true);
					App1->get_regular("text2").set_surface_text(elems[DR->returnright()].text, true);
				}
				*plog << "Doing (" << DR->returnleft() << "), (" << DR->returnright() << ") choice\n";
			}
			else { quit = 1; }
			break;
		default:
			if (!(SDL_GetTicks() % 15)) { // realization with (time mod [1000/fps]) ~62.5 fps
				SDL_SetRenderDrawColor(App1->renders, 255, 255, 255, SDL_ALPHA_OPAQUE);
				SDL_RenderClear(App1->renders);
				SDL_SetRenderDrawColor(App1->renders, 0, 0, 0, SDL_ALPHA_OPAQUE);
				SDL_RenderDrawLine(App1->renders, SWIDTH / 2, 0, SWIDTH / 2, SHEIGHT - 55);
				SDL_RenderDrawLine(App1->renders, 0, SHEIGHT - 55, SWIDTH, SHEIGHT - 55);
				App1->render();
				SDL_RenderPresent(App1->renders);
			}
		}
	}
	return 2;
}
int Application::res_section() {
	App1->clear_objects();
	if (elems.size() >= DR->get_size()) {
		CreateResultsNumericText(*DR, "times.ttf", elems);
	}
	else {
		CreateResultsNumeric(*DR);
	}
	quit = 0;
	SDL_SetRenderDrawColor(App1->renders, 255, 255, 255, SDL_ALPHA_OPAQUE);
	bool success_correct = App1->append_object(RegularObject((char*)"images\\GUI\\success.png", "success"));
	bool button_correct = App1->append_object(RegularObject((char*)"images\\GUI\\bigokbutton.png", "ok button"));
	if (!success_correct || !button_correct) {
		*plog << "Failed success image (?)\n";
		throw 0;
	}
	RegularObject& button = App1->get_regular("ok button");
	int buttonwidth = button.get_surface()->w / 2;
	int buttonheight = button.get_surface()->h / 2;
	button.set_rect(573, 461, buttonwidth, buttonheight, true);
	while (!quit) {
		SDL_PollEvent(e);
		switch (e->type) {
		case SDL_QUIT:
			quit = 1;
		case SDL_MOUSEBUTTONDOWN:
			SDL_GetMouseState(&mx, &my);
			if (mouse_is_in_range(mx, my, button)) {
				Mix_PlayChannel(-1, App1->quirk, 0);
				throw 1;
			}
		default:
			if (!(SDL_GetTicks() % 15)) { // realization with (time mod [1000/fps]) ~62.5 fps
				SDL_RenderClear(App1->renders);
				App1->render();
				SDL_RenderPresent(App1->renders);
			}
		}
	}
	return 2;
}

int Application::single_section() {
	App1->clear_objects();
	int i = 0;
	nfdpathset_t *paths = new nfdpathset_t;
	nfdresult_t ret;
	vector<FIBITMAP *> images;
	SDL_SetRenderDrawColor(App1->renders, 255, 255, 255, SDL_ALPHA_OPAQUE);
	FIBITMAP *num = num_to_image(i);
	bool single_correct = App1->append_object(RegularObject((char*)"images\\GUI\\single.png", "single"));
	bool num_correct = App1->append_object(RegularObject(num, "number"));
	bool load_correct = App1->append_object(RegularObject((char*)"images\\GUI\\loadbutton.png", "load button"));
	bool ok_correct = App1->append_object(RegularObject((char*)"images\\GUI\\bigokbutton.png", "ok button"));
	bool fill_correct = App1->append_object(RegularObject((char*)"images\\GUI\\fillempty.png", "fill button"));
	if (!num_correct || !single_correct || !load_correct || !ok_correct || !fill_correct) {
		*plog << "Failed images (?)\n";
		throw 0;
	}
	RegularObject& button = App1->get_regular("ok button");
	RegularObject& load_button = App1->get_regular("load button");
	RegularObject& fill_button = App1->get_regular("fill button");
	RegularObject& number = App1->get_regular("number");
	int buttonwidth = button.get_surface()->w / 2;
	int buttonheight = button.get_surface()->h / 2;
	button.set_rect(555, 444, buttonwidth / 2, buttonheight / 2, true);
	fill_button.set_rect(555, 424, buttonwidth / 2, buttonheight / 2, true);
	number.set_rect(355, 328, number.get_surface()->w / 2, number.get_surface()->h / 2, true);
	load_button.set_rect(SWIDTH / 2, SHEIGHT / 2, buttonwidth, buttonheight, true);
	while (!quit) {
		SDL_PollEvent(e);
		switch (e->type) {
		case SDL_QUIT:
			quit = 1;
		case SDL_MOUSEBUTTONDOWN:
			SDL_GetMouseState(&mx, &my);
			if (mouse_is_in_range(mx, my, load_button)) {
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
					App1->get_regular("number").set_surface(num);
				}
			}
			if (mouse_is_in_range(mx, my, button)) {
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
				save_resized(images);
				for (int j = 0; j < images.size(); j++) {
					if (j == 0) {
						FreeImage_Unload(images[j]);
					}
					else if (images[j] != images[j - 1]) {
						FreeImage_Unload(images[j]);
					}
				}
				throw 1;
			}
			if (mouse_is_in_range(mx, my, fill_button)) {
				Mix_PlayChannel(-1, App1->quirk, 0);
				i = DR->get_size() < i ? i : DR->get_size();
				int id = images.size() - 1;
				while (images.size() < i) {
					images.push_back(images[id]);
				}
				FreeImage_Unload(num);
				num = num_to_image(i);
				App1->get_regular("number").set_surface(num);
			}
		default:
			if (!(SDL_GetTicks() % 15)) { // realization with (time mod [1000/fps]) ~62.5 fps
				SDL_RenderClear(App1->renders);
				App1->render();
				SDL_RenderPresent(App1->renders);
			}
		}
	}
	for (int j = 0; j < images.size(); j++) {
		if (j == 0) {
			FreeImage_Unload(images[j]);
		}
		else if (images[j] != images[j - 1]) {
			FreeImage_Unload(images[j]);
		}
	}
	delete paths;
}

void save_resized(vector<FIBITMAP *> &imgs) {
	FIBITMAP *tmp;
	int sum_size = 0;
	int tmpw, tmph;
	char path[MAX_PATH_LEN];
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
			generate_path(i + 1, path);
			FreeImage_Save(FIF_PNG, tmp, path);
			FreeImage_Unload(tmp);
		}
	}
	catch (...) {
		plog << "Save_resized caught exception" << endl;
	}
}

void CreateResults(Sort &sort) {
	int LIST_Len = sort.get_size();
	int w = 0; int h = 0;
	int sqr = 1 + (int)sqrt(LIST_Len-1);
	char path[MAX_PATH_LEN];
	int basew = 0, baseh = 0;
	FIBITMAP *res;
	try {
		FIBITMAP **all = new FIBITMAP * [LIST_Len];
		for (int i = 0; i < LIST_Len; i++) {
			generate_path(i + 1, path);
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
	char path[MAX_PATH_LEN];
	int basew = 0, baseh = 0;
	int tablw = 0, tablh = 0;
	FIBITMAP *res;
	try {
		FIBITMAP **all = new FIBITMAP * [LIST_Len];
		FIBITMAP **alltabl = new FIBITMAP * [LIST_Len];
		for (int i = 0; i < LIST_Len; i++) {
			generate_path(i + 1, path);
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
			plog << "Table Generation: Smth happened!" << endl;
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

void CreateResultsNumericText(Sort& sort, std::string font, const std::vector<SingleElement>& data) {
	int LIST_Len = sort.get_size();
	int w = 0; int h = 0;
	int sqr = 1 + (int)sqrt(LIST_Len - 1);
	char path[MAX_PATH_LEN];
	int basew = 0, baseh = 0;
	int imgh = 0;
	int texth = 0;
	int tablw = 0, tablh = 0;
	FIBITMAP* res;
	try {
		FIBITMAP** all = new FIBITMAP * [LIST_Len];
		FIBITMAP** alltabl = new FIBITMAP * [LIST_Len];
		FIBITMAP** alltext = new FIBITMAP * [LIST_Len];
		for (int i = 0; i < LIST_Len; i++) {
			generate_path(i + 1, path);
			all[i] = FreeImage_Load(FIF_PNG, path, PNG_DEFAULT);
			if (!w || !h) {
				w = FreeImage_GetWidth(all[i]) * sqr;
				basew = FreeImage_GetWidth(all[i]);
				h = FreeImage_GetHeight(all[i]) * sqr;
				baseh = FreeImage_GetHeight(all[i]);
				imgh = FreeImage_GetHeight(all[i]);
				texth = baseh / 2;
			}
		}
		tablw = basew;
		try {
			init_text(alltext, data, font, tablw, texth);
			for (int i = 0; i < LIST_Len; i++) {
				init_table(alltabl[i], i + 1, tablw);
				if (!tablh) {
					tablh = FreeImage_GetHeight(alltabl[i]);
				}
			}
		}
		catch (...) {
			plog << "Table Generation: Smth happened!" << endl;
		}
		h += sqr * (tablh + texth);
		baseh += tablh + texth;
		res = FreeImage_Allocate(w, h, 32, 0, 0, 0);
		SortList Res = sort.getFinalSort();
		for (int i = 0; i < LIST_Len; i++) {
			//res.composite(all[Res.getelem(i)], all[0].columns() * (i % sqr), (i / sqr) * all[0].rows());
			plog << Res.getelem(i) << " - " << FreeImage_Paste(res, alltabl[i], basew * (i % sqr), baseh * (i / sqr), 255) << " - ";
			plog << FreeImage_Paste(res, all[Res.getelem(i)], basew * (i % sqr), baseh * (i / sqr) + tablh, 255) << endl;
			plog << FreeImage_Paste(res, alltext[Res.getelem(i)], basew * (i % sqr), baseh * (i / sqr) + tablh + imgh, 255) << endl;
		}
		FreeImage_Save(FIF_PNG, res, "result.png", PNG_DEFAULT);
		plog << "Result.png was created!\n";
		for (int i = 0; i < LIST_Len; i++) {
			FreeImage_Unload(alltext[i]);
			FreeImage_Unload(alltabl[i]);
			FreeImage_Unload(all[i]);
		}
		delete[] alltext;
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

void init_text(FIBITMAP**& img, const std::vector<SingleElement>& data, std::string font, int w, int h) {
	int ptsize = 72;
	TTF_Font* ttffont;
	SDL_Color color = { 0, 0, 0 };
	SDL_Surface* surface;
	for (int i = 0; i < data.size(); i++) {
		ttffont = TTF_OpenFont(font.c_str(), ptsize);
		surface = TTF_RenderUNICODE_Solid_Wrapped(ttffont, (const Uint16*)data[i].text.c_str(), color, w);
		TTF_CloseFont(ttffont);
		while (surface->h > h) {
			ptsize -= 2;
			ttffont = TTF_OpenFont(font.c_str(), ptsize);
			surface = TTF_RenderUNICODE_Solid_Wrapped(ttffont, (const Uint16*)data[i].text.c_str(), color, w);
			TTF_CloseFont(ttffont);
		}
		while (surface->h < h / 2) {
			ptsize += 2;
			ttffont = TTF_OpenFont(font.c_str(), ptsize);
			surface = TTF_RenderUNICODE_Solid_Wrapped(ttffont, (const Uint16*)data[i].text.c_str(), color, w);
			TTF_CloseFont(ttffont);
		}
		img[i] = get_fibitmap(surface);
	}
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
		plog << "Table: Problem with number size" << endl;
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

bool mouse_is_in_range(int& mx, int& my, const RegularObject& obj) {
	SDL_Rect rect = obj.get_rect();
	rect.x *= (float)CURWIDTH / SWIDTH;
	rect.y *= (float)CURHEIGHT / SHEIGHT;
	rect.w *= (float)CURWIDTH / SWIDTH;
	rect.h *= (float)CURHEIGHT / SHEIGHT;
	if (mx >= rect.x && mx <= rect.x + rect.w && my >= rect.y && my <= rect.y + rect.h) {
		return true;
	}
	return false;
}

bool mouse_is_in_range_abs(int& mx, int& my, int x1, int x2, int y1, int y2) {
	if (mx >= (int)((double)x1 / SWIDTH * CURWIDTH) &&
		mx <= (int)((double)x2 / SWIDTH * CURWIDTH) &&
		my >= (int)((double)y1 / SHEIGHT * CURHEIGHT) &&
		my <= (int)((double)y2 / SHEIGHT * CURHEIGHT)) {
		return true;
	}
	return false;
}

char* generate_path(int num, char* to) {
	string path = "images\\";
	path.append(to_string(num));
	path.append(".png");
	for (int i = 0; i < path.length(); i++) {
		to[i] = path[i];
	}
	to[path.length()] = '\0';
	return to;
}

char* generate_path_song(std::string song, char* to) {
	if (song.substr(0, 4) == "http") {
		for (int i = 0; i < song.length(); i++) {
			to[i] = song[i];
		}
		to[song.length()] = '\0';
		return to;
	}
	string path = "songs\\";
	path.append(song);
	for (int i = 0; i < path.length(); i++) {
		to[i] = path[i];
	}
	to[path.length()] = '\0';
	return to;
}

void separate(int size, int row, int columns) {
	FIBITMAP **imgset = new FIBITMAP * [size];
	FIBITMAP *from = FreeImage_Load(FIF_PNG, "giantimage.png", PNG_DEFAULT);
	if (columns <= 0 || row <= 0) {
		plog << "Cols, rows cannot be <=0" << endl;
		return;
	}
	int width = FreeImage_GetWidth(from) / columns;
	int height = FreeImage_GetHeight(from) / row;
	if (width == 0 || height == 0) {
		plog << "Imgsize cannot be 0" << endl;
		return;
	}
	try {
		char path[MAX_PATH_LEN];
		for (int i = 0; i < size; i++) {
			imgset[i] = FreeImage_Copy(from, width * (i % columns), (i / columns) * height, width * (i % columns) + width, (i / columns) * height + height);
			generate_path(i + 1, path);
			FreeImage_Save(FIF_PNG, imgset[i], path, PNG_DEFAULT);
		}
	}
	catch (...)
	{
		plog << "Caught exception" << endl;
	}
	FreeImage_Unload(from);
	for (int i = 0; i < size; i++) {
		FreeImage_Unload(imgset[i]);
	}
	delete[] imgset;
}