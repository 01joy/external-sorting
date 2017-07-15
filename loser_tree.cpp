#include "sdk.h"
#include "loser_tree.h"
#include "diy_fp.h"

using namespace std;

LoserTree::LoserTree() {
}

void LoserTree::Adjust(vector<int> &tree, vector<double> &b, int s, int num_file) {
	for (int t = (num_file + s) / 2; t > 0; t /= 2) {
		if (b[s] > b[tree[t]]) {
			int tmp = s;
			s = tree[t];
			tree[t] = tmp;
		}
	}
	tree[0] = s;
}

void LoserTree::Create(vector<int> &tree, vector<double> &b, int num_file) {
	b[num_file] = -DBL_MAX;

	for (int i = 1; i < num_file; i++)
		tree[i] = num_file;
	for (int i = num_file - 1; i >= 0; i--)
		Adjust(tree, b, i, num_file);
}


//http://gengning938.blog.163.com/blog/static/12822538120115131197839/
void LoserTree::Merge(const int &num_file) {
	const SearchParameter &sp = SearchParameter::GetInstance();

	vector<ifstream> iss(num_file);
	vector<double> b(num_file + 1);
	vector<int> tree(num_file);

	for (int i = 0; i < num_file; i++) {
		iss[i].open(to_string(i).c_str(), ios::binary);
		iss[i].read((char*)&b[i], kDoubleSize);
	}

	Create(tree, b, num_file);

	int len = sp.max_char_per_file_ * 5;
	char *buffer = new char[len];
	char *current_buf = buffer;

	ofstream file_output(sp.path_output_.c_str(), ios::binary);
	
	int s_pos = tree[0], t = 0;
	double smallest = b[s_pos];
	while (smallest != DBL_MAX) {

		MiloDToA(smallest, current_buf, current_buf);
		if(len - (current_buf - buffer) < sp.max_char_per_line_){
			file_output.write(buffer, current_buf - buffer);
			current_buf = buffer;
		}

		if (iss[s_pos].peek() == EOF) {
			b[s_pos] = DBL_MAX;
		} else {
			iss[s_pos].read((char*)&b[s_pos], kDoubleSize);
		}

		Adjust(tree, b, s_pos, num_file);
		s_pos = tree[0];
		smallest = b[s_pos];
	}

	file_output.write(buffer, current_buf - buffer);
	file_output.close();
	delete buffer;

	for (int i = 0; i < num_file; i++) iss[i].close();
}

LoserTree::~LoserTree() {
}
