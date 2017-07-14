#include<iostream>
#include<string>
#include<algorithm>
#include<fstream>
#include<ctime>
#include<cstdio>
#include<cstdlib>
#include<vector>
#include<cctype>
#include<cfloat>

#include "bounded_buffer.h"
#include "KMerge.h"
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

		time_t tmp_t = clock();
		printf("\n分成小文件并调入内存排序用时%.2f秒\n", (float)(tmp_t - start_time) / CLOCKS_PER_SEC);

		KMerge km(sp.max_char_per_line_);

		printf("\n终极%d路归并...\n", num_file);

		char out_name[3];
		vector<string> final_path_inputs;

		for (int i = 0; i < num_file; i++) {
			sprintf(out_name, "%d", i);
			final_path_inputs.push_back(out_name);
		}
		km.k_merge_to_str(final_path_inputs, sp.path_output_);

		printf("归并用时%.2f秒\n", (float)(clock() - tmp_t) / CLOCKS_PER_SEC);

		printf("\n排序完成，总用时%.2f秒，共发现%d个非法条目\n", (float)(clock() - start_time) / CLOCKS_PER_SEC, accumulate(num_bad.begin(),num_bad.end(),0));
	
	
/*
		for (int i = 0; i < num_file; i++)
			remove(final_path_inputs[i].c_str());//删除中间文件
*/
	}
	
	system("pause");
	return 0;
}
