#ifndef KMERGE_H_
#define KMERGE_H_

#include<cstdio>
#include<fstream>
#include<ctime>
#include<vector>
#include<string>
#include<mutex>
#include<thread>
#include<cfloat>

class KMerge
{
private:
	
	int MAX_CHAR_NUM_PER_LINE;//一个条目的最大长度

	int CHAR_SIZE;//sizeof(char);
	int DOUBLE_SIZE;//sizeof(double);
	
	std::mutex mtx;

public:
	KMerge(int MAX_CHAR_NUM_PER_LINE);

	void adjust(int *ls, double *b, int s, int num_file);

	void create_loser_tree(int *ls, double *b, int num_file);

	bool write_double(std::ofstream& os, double* d, int n);

	void k_merge_to_str(std::vector<std::string> path_inputs, std::string path_output);
	
	~KMerge();
};

#endif
