#include "SortMachine.h"

SortList SortList::slice(int start, int end) const {
	SortList res(end - start);
	for (int i = start; i < end; i++) {
		res.setelem(i - start, getelem(i));
	}
	return res;
}

SortList &SortList::operator= (int *p) {
	for (int i = 0; i < SortMaxLen; i++) {
		setelem(i, p[i]);
	}
	return *this;
}

SortList &SortList::operator= (const SortList &list) {
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

Sort::Sort(int LIST_Len, std::ofstream &plog) : sortlist(LIST_Len * 2, SortList(LIST_Len)) {
	status = 0;
	pointer = 1;
	sortlistlen = 1;
	TempList = new int[LIST_Len];
	RecordList = new int[LIST_Len];
	parentlist = new int[LIST_Len * 2];
	basic_size = LIST_Len;
	Fill(TempList, LIST_Len);
	for (int i = 0; i < LIST_Len; i++) {
		RecordList[i] = 0;
	}
	sortlist[0] = TempList;
	this->plog = &plog;
	for (int i = 0; i < sortlistlen; i++) {
		if (sortlist[i].length() >= 2) {
			int Marker = ceil((double)(sortlist[i].length()) / 2);
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

void Sort::save() {
	std::ofstream savefile;
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
	std::ifstream savefile;
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
	*plog << "Loaded:\n";
	for (int i1 = 0; i1 < 6; i1++) {
		*plog << *(ptrs[i1]) << ' ';
	}
	printsortlist();
	for (int l = 0; l < basic_size; l++) {
		*plog << RecordList[l] << ' ';
	}
	*plog << '\n';
	savefile.close();
	delete[] data;
}

bool Sort::sort_undo() {
	if (leftID == 0 && rightID == 0) {
		return 0;
	}
	recordID--;
	if (RecordList[recordID] == sortlist[leftlist].getelem(leftID - 1)) {
		leftID--;
	}
	else if (RecordList[recordID] == sortlist[rightlist].getelem(rightID - 1)) {
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
	*plog << "[";
	for (int i = 0; i < sortlistlen; i++) {
		sortlist[i].print(*plog);
		if (i != sortlistlen - 1) {
			*plog << ", ";
		}
		else {
			*plog << "]\n";
		}
	}
	*plog << "Length: " << sortlistlen << '\n';
}

void Sort::printfulllist(int len) const {
	*plog << "[";
	for (int i = 0; i < len; i++) {
		if (i != len - 1) {
			*plog << TempList[i] << ", ";
		}
		else {
			*plog << TempList[i] << "]\n";
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
		*plog << "List is Sorted now\n";
	}
}