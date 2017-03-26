#pragma once
#pragma warning(disable:4996)
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <fstream>
#include <algorithm>
#include <vector>
#include <string>
#include <cctype>
using namespace std;

#define LL long long

//C++11������������ģ�Ͳο� http://www.cnblogs.com/haippy/p/3252092.html
class ProducerComsumer
{
private:
	int kItemRepositorySize; // ����������
	int MAX_CHAR_NUM_PER_FILE;//ÿ�ζ�ȡ�ַ�����
	int MAX_CHAR_NUM_PER_LINE;//һ����Ŀ�����ַ��� 
	int MAX_DOUBLE_NUM_PER_FILE;//ÿ���ļ���double���������
	int CHAR_SIZE;//sizeof(char);
	int DOUBLE_SIZE;//sizeof(double);
	bool all_done;//�������Ƿ����
	int num_file;//����С�ļ�����
	vector<int> bad;//bad[i]�����˵�i���ļ��зǷ���Ŀ����������Ϊ�Ƕ��̣߳�����Ҫ�ֱ��¼�����ͬʱ�޸�bad�Ļ�������߳����������Ӱ��Ч��

	
	vector<char*> item_buffer;//������ָ��
	vector<int> buffer_len;//ÿ����������ʵ�ʳ���
	int read_position;//��������λ��
	int write_position;//д������λ��
	mutex mtx;//������������
	condition_variable repo_not_full;//��������������������
	condition_variable repo_not_empty;//��������������������

public:
	ProducerComsumer(int kItemRepositorySize, int MAX_CHAR_NUM_PER_FILE, int MAX_CHAR_NUM_PER_LINE, int MAX_DOUBLE_NUM_PER_FILE);

	bool is_number(char * c);

	double fast_atof(const char *p);

	void radix_sort(double array[], int n);

	void partition_sort(char *buffer, int len, int order);

	void ProduceItem(char* item, int len);

	bool ConsumeItem();

	void ProducerTask(ifstream& is);

	void ConsumerTask();

	int get_bad_num();

	int get_file_num();

	~ProducerComsumer();
};

