#pragma once
#include <vector>
#include <fstream>

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
	SortList &operator= (int *p);
	SortList &operator= (const SortList &list);
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
	void print(std::ofstream &plog) const {
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

class Sort {
public:
	Sort(int LIST_Len, std::ofstream &plog);
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
	std::vector<SortList> sortlist;
	int sortlistlen;
	int *parentlist;
	int leftlist, rightlist;
	int leftID, rightID, recordID;
	int *RecordList;
	bool status;
	int basic_size;
	std::ofstream *plog;
};
