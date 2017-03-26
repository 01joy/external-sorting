#pragma once
#pragma warning (disable : 4996)//忽略itoa->_itoa
#include<cstdio>
#include<fstream>
#include<ctime>
#include<vector>
#include<string>
#include<mutex>
#include<thread>

using namespace std;

class KMerge
{
private:
	
	int MAX_CHAR_NUM_PER_LINE;//一个条目的最大长度

	int CHAR_SIZE;//sizeof(char);
	int DOUBLE_SIZE;//sizeof(double);
	
	mutex mtx;

public:
	KMerge(int MAX_CHAR_NUM_PER_LINE);

	void adjust(int *ls, double *b, int s, int num_file);

	void create_loser_tree(int *ls, double *b, int num_file);

	bool write_double(ofstream& os, double* d, int n);

	void k_merge_to_str(vector<string> path_inputs, string path_output);
	
	~KMerge();
};

