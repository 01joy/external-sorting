#ifndef PRODUCERCOMSUMER_H_
#define PRODUCERCOMSUMER_H_
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
#include "sdk.h"


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
	std::vector<int> bad;//bad[i]�����˵�i���ļ��зǷ���Ŀ����������Ϊ�Ƕ��̣߳�����Ҫ�ֱ��¼�����ͬʱ�޸�bad�Ļ�������߳����������Ӱ��Ч��

	
	std::vector<char*> item_buffer;//������ָ��
	std::vector<int> buffer_len;//ÿ����������ʵ�ʳ���
	int read_position;//��������λ��
	int write_position;//д������λ��
	std::mutex mtx;//������������
	std::condition_variable repo_not_full;//��������������������
	std::condition_variable repo_not_empty;//��������������������

public:
	ProducerComsumer(int kItemRepositorySize, int MAX_CHAR_NUM_PER_FILE, int MAX_CHAR_NUM_PER_LINE, int MAX_DOUBLE_NUM_PER_FILE);

	void partition_sort(char *buffer, int len, int order);

	void ProduceItem(char* item, int len);

	bool ConsumeItem();

	void ProducerTask(std::ifstream& is);

	void ConsumerTask();

	int get_bad_num();

	int get_file_num();

	~ProducerComsumer();
};

#endif
