#ifndef LOSER_TREE_H_
#define LOSER_TREE_H_

#include <vector>
#include <string>
#include <cfloat>

class LoserTree {
public:
	LoserTree();

	~LoserTree();

	void Merge(const int &num_file);

private:

	void Adjust(std::vector<int> &tree, std::vector<double> &b, int s, int num_file);

	void Create(std::vector<int> &tree, std::vector<double> &b, int num_file);

	bool Write(std::ofstream& os, double* d, int n);

};

#endif
