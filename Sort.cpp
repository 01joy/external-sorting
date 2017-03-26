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
#pragma warning (disable : 4996)//忽略itoa->_itoa

int MAX_CHAR_NUM_PER_FILE;//每次读取字符数量
const int MAX_CHAR_NUM_PER_LINE=30;//一个条目最大的字符数，可根据实际输入文件修改
int MAX_DOUBLE_NUM_PER_FILE;//每个文件中double的最大数量<=MAX_CHAR_NUM_PER_FILE/15

string path_input,path_output;//分别是输入，输出文件路径
int sort_thread_num;//排序线程数

//判断path路径的文件是否存在
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

//读取配置参数，不同返回值代表不同出错情况，返回0代表成功，具体看程序
int read_param()
{
	printf("读取配置参数...\n");

	ifstream param_file("Sort.param");
	if(!param_file)
	{
		printf("配置文件不存在！\n");
		return 1;
	}
	string s;
	getline(param_file,s);
	int pos=s.find("=");
	path_input=s.substr(pos+1,s.find(";")-pos-1);
	if(path_input=="")
	{
		printf("path_input值为空！\n");
		return 2;
	}
	if(!is_file_exist(path_input))
	{
		printf("path_input文件不存在！\n");
		return 3;
	}
	getline(param_file,s);
	pos=s.find("=");
	path_output=s.substr(pos+1,s.find(";")-pos-1);
	if(path_output=="")
	{
		printf("path_output值为空！\n");
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
			printf("num_char_per_file值必须大于%d\n",MAX_CHAR_NUM_PER_LINE);
			return 5;
		}
		MAX_DOUBLE_NUM_PER_FILE=MAX_CHAR_NUM_PER_FILE/15;//15分之一
	}
	else
	{
		printf("num_char_per_file值为空！\n");
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
		printf("一个读线程和%d个排序线程正在工作...\n", sort_thread_num);

		ProducerComsumer pc(2, MAX_CHAR_NUM_PER_FILE, MAX_CHAR_NUM_PER_LINE, MAX_DOUBLE_NUM_PER_FILE);//仓库大小为2
		ifstream file_input(path_input,ios::binary);
		thread producer(bind(&ProducerComsumer::ProducerTask, &pc, ref(file_input)));

		vector<thread> threads;
		for (int i = 0; i < sort_thread_num;i++)//匿名thread参考：http://my.oschina.net/zhangjie830621/blog/188699
			threads.push_back(thread(bind(&ProducerComsumer::ConsumerTask, &pc)));

		producer.join();
		for (int i = 0; i < sort_thread_num; i++)
			threads[i].join();

		threads.clear();
		file_input.close();
		time_t tmp_t = clock();
		printf("\n分成小文件并调入内存排序用时%.2f秒\n", (float)(tmp_t - start_t) / CLOCKS_PER_SEC);

		KMerge km(MAX_CHAR_NUM_PER_LINE);

		int num_file = pc.get_file_num();
		printf("\n终极%d路归并...\n", num_file);

		char out_name[3];
		vector<string> final_path_inputs;

		for (int i = 0; i < num_file; i++)
		{
			sprintf(out_name, "%d", i);
			final_path_inputs.push_back(out_name);
		}
		km.k_merge_to_str(final_path_inputs, path_output);

		printf("归并用时%.2f秒\n", (float)(clock() - tmp_t) / CLOCKS_PER_SEC);

		printf("\n排序完成，总用时%.2f秒，共发现%d个非法条目\n", (float)(clock() - start_t) / CLOCKS_PER_SEC, pc.get_bad_num());
	
	
		for (int i = 0; i < num_file; i++)
			remove(final_path_inputs[i].c_str());//删除中间文件
	}
	
	system("pause");
	return 0;
}