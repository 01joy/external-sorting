#include <iostream>
#include <string>
#include <algorithm>
#include <ctime>
#include <vector>
#include <numeric>
#include "bounded_buffer.h"
#include "loser_tree.h"
#include "sdk.h"
using namespace std;

int main(int argc, char *argv[]) {

	if(argc != 2) {
		cout << "Usage:" << endl
			 << argv[0] << " parameter.txt" << endl;
		return 0;
	}

	time_t start_time = clock();

	if(ParseParamFile(argv[1])) {

		SearchParameter &sp = SearchParameter::GetInstance();

		cout << "1 reading thread and " << sp.num_thread_ << " sorting thread(s) are working..." << endl;

		int num_file = 0;
		BoundedBuffer buffer(2);
		thread producer(Produce, ref(buffer), ref(num_file));

		vector<thread> consumers(sp.num_thread_);
		vector<int> num_bad(sp.num_thread_);
		for (int i = 0; i < sp.num_thread_; i++)
			consumers[i] = thread(Consume, ref(buffer), ref(num_bad[i]));

		producer.join();
		for (int i = 0; i < sp.num_thread_; i++)
			consumers[i].join();

		time_t partition_sort_time = clock();
		cout << "partition and sort time: " << (double)(partition_sort_time - start_time) / CLOCKS_PER_SEC << endl;

		cout << "merging " << num_file << " files..." << endl;
		LoserTree tree;
		tree.Merge(num_file);

		time_t merge_time = clock();

		cout << "merge time: " << (double)(merge_time - partition_sort_time) / CLOCKS_PER_SEC << endl
			 << "total time: " << (double)(merge_time - start_time) / CLOCKS_PER_SEC << endl
		     << "illegal number: " << accumulate(num_bad.begin(), num_bad.end(), 0) - 1 << endl;
		// 非法数目减一的原因是，输入数据末尾有一个\n,Produce对每一块数据加了一个\n\0，导致最后多出一个\n
	
		for (int i = 0; i < num_file; i++)
			remove(to_string(i).c_str()); // remove tmp files
	}
	
	system("pause");
	return 0;
}
