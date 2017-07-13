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
#include"ProducerComsumer.h"
#include"KMerge.h"
#include"sdk.h"
using namespace std;

int main(int argc, char *argv[]) {

	if(argc != 2) {
		cout << "Usage:" << endl
			 << argv[0] << " parameter.txt" << endl;
		return 0;
	}

	time_t start_time = clock();

	if(!ParseParamFile(argv[1])) {

		SearchParameter &sp = SearchParameter::GetInstance();

		cout << "1 reading thread and " << sp.num_thread_ << "sorting threads are working..." << endl;

		ProducerComsumer pc(2, sp.max_char_per_file_, sp.max_char_per_line_, sp.max_double_per_file_);//�ֿ��СΪ2
		ifstream file_input(sp.path_input_,ios::binary);
		thread producer(bind(&ProducerComsumer::ProducerTask, &pc, ref(file_input)));

		vector<thread> threads;
		for (int i = 0; i < sp.num_thread_; i++)//����thread�ο���http://my.oschina.net/zhangjie830621/blog/188699
			threads.push_back(thread(bind(&ProducerComsumer::ConsumerTask, &pc)));

		producer.join();
		for (int i = 0; i < sp.num_thread_; i++)
			threads[i].join();

		threads.clear();
		file_input.close();
		time_t tmp_t = clock();
		printf("\n�ֳ�С�ļ��������ڴ�������ʱ%.2f��\n", (float)(tmp_t - start_time) / CLOCKS_PER_SEC);

		KMerge km(sp.max_char_per_line_);

		int num_file = pc.get_file_num();
		printf("\n�ռ�%d·�鲢...\n", num_file);

		char out_name[3];
		vector<string> final_path_inputs;

		for (int i = 0; i < num_file; i++) {
			sprintf(out_name, "%d", i);
			final_path_inputs.push_back(out_name);
		}
		km.k_merge_to_str(final_path_inputs, sp.path_output_);

		printf("�鲢��ʱ%.2f��\n", (float)(clock() - tmp_t) / CLOCKS_PER_SEC);

		printf("\n������ɣ�����ʱ%.2f�룬������%d���Ƿ���Ŀ\n", (float)(clock() - start_time) / CLOCKS_PER_SEC, pc.get_bad_num());
	
	
		for (int i = 0; i < num_file; i++)
			remove(final_path_inputs[i].c_str());//ɾ���м��ļ�
	}
	
	system("pause");
	return 0;
}
