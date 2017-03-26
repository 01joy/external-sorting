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
using namespace std;
#pragma warning (disable : 4996)//����itoa->_itoa

int MAX_CHAR_NUM_PER_FILE;//ÿ�ζ�ȡ�ַ�����
const int MAX_CHAR_NUM_PER_LINE=30;//һ����Ŀ�����ַ������ɸ���ʵ�������ļ��޸�
int MAX_DOUBLE_NUM_PER_FILE;//ÿ���ļ���double���������<=MAX_CHAR_NUM_PER_FILE/15

string path_input,path_output;//�ֱ������룬����ļ�·��
int sort_thread_num;//�����߳���

//�ж�path·�����ļ��Ƿ����
bool is_file_exist(string path)
{
	ifstream file(path);
	if(!file)
	{
		file.close();
		return false;
	}
	file.close();
	return true;
}

//��ȡ���ò�������ͬ����ֵ����ͬ�������������0����ɹ������忴����
int read_param()
{
	printf("��ȡ���ò���...\n");

	ifstream param_file("Sort.param");
	if(!param_file)
	{
		printf("�����ļ������ڣ�\n");
		return 1;
	}
	string s;
	getline(param_file,s);
	int pos=s.find("=");
	path_input=s.substr(pos+1,s.find(";")-pos-1);
	if(path_input=="")
	{
		printf("path_inputֵΪ�գ�\n");
		return 2;
	}
	if(!is_file_exist(path_input))
	{
		printf("path_input�ļ������ڣ�\n");
		return 3;
	}
	getline(param_file,s);
	pos=s.find("=");
	path_output=s.substr(pos+1,s.find(";")-pos-1);
	if(path_output=="")
	{
		printf("path_outputֵΪ�գ�\n");
		return 4;
	}
	getline(param_file,s);
	pos=s.find("=");
	s=s.substr(pos+1,s.find(";")-pos-1);
	if(s!="")
		sort_thread_num=atoi(s.c_str());

	getline(param_file, s);
	pos = s.find("=");
	s = s.substr(pos + 1, s.find(";") - pos - 1);
	if(s!="")
	{
		MAX_CHAR_NUM_PER_FILE=atoi(s.c_str());
		if(MAX_CHAR_NUM_PER_FILE<MAX_CHAR_NUM_PER_LINE)
		{
			printf("num_char_per_fileֵ�������%d\n",MAX_CHAR_NUM_PER_LINE);
			return 5;
		}
		MAX_DOUBLE_NUM_PER_FILE=MAX_CHAR_NUM_PER_FILE/15;//15��֮һ
	}
	else
	{
		printf("num_char_per_fileֵΪ�գ�\n");
		return 6;
	}
	param_file.close();
	return 0;
}

int main()
{
	time_t start_t = clock();

	if(read_param()==0)
	{
		printf("һ�����̺߳�%d�������߳����ڹ���...\n", sort_thread_num);

		ProducerComsumer pc(2, MAX_CHAR_NUM_PER_FILE, MAX_CHAR_NUM_PER_LINE, MAX_DOUBLE_NUM_PER_FILE);//�ֿ��СΪ2
		ifstream file_input(path_input,ios::binary);
		thread producer(bind(&ProducerComsumer::ProducerTask, &pc, ref(file_input)));

		vector<thread> threads;
		for (int i = 0; i < sort_thread_num;i++)//����thread�ο���http://my.oschina.net/zhangjie830621/blog/188699
			threads.push_back(thread(bind(&ProducerComsumer::ConsumerTask, &pc)));

		producer.join();
		for (int i = 0; i < sort_thread_num; i++)
			threads[i].join();

		threads.clear();
		file_input.close();
		time_t tmp_t = clock();
		printf("\n�ֳ�С�ļ��������ڴ�������ʱ%.2f��\n", (float)(tmp_t - start_t) / CLOCKS_PER_SEC);

		KMerge km(MAX_CHAR_NUM_PER_LINE);

		int num_file = pc.get_file_num();
		printf("\n�ռ�%d·�鲢...\n", num_file);

		char out_name[3];
		vector<string> final_path_inputs;

		for (int i = 0; i < num_file; i++)
		{
			sprintf(out_name, "%d", i);
			final_path_inputs.push_back(out_name);
		}
		km.k_merge_to_str(final_path_inputs, path_output);

		printf("�鲢��ʱ%.2f��\n", (float)(clock() - tmp_t) / CLOCKS_PER_SEC);

		printf("\n������ɣ�����ʱ%.2f�룬������%d���Ƿ���Ŀ\n", (float)(clock() - start_t) / CLOCKS_PER_SEC, pc.get_bad_num());
	
	
		for (int i = 0; i < num_file; i++)
			remove(final_path_inputs[i].c_str());//ɾ���м��ļ�
	}
	
	system("pause");
	return 0;
}