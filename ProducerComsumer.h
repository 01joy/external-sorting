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


//C++11生产者消费者模型参考 http://www.cnblogs.com/haippy/p/3252092.html
class ProducerComsumer
{
private:
	int kItemRepositorySize; // 缓冲区数量
	int MAX_CHAR_NUM_PER_FILE;//每次读取字符数量
	int MAX_CHAR_NUM_PER_LINE;//一个条目最大的字符数 
	int MAX_DOUBLE_NUM_PER_FILE;//每个文件中double的最大数量
	int CHAR_SIZE;//sizeof(char);
	int DOUBLE_SIZE;//sizeof(double);
	bool all_done;//生产者是否完成
	int num_file;//生成小文件数量
	std::vector<int> bad;//bad[i]保存了第i个文件中非法条目的数量，因为是多线程，所以要分别记录，如果同时修改bad的话，多个线程抢这个锁，影响效率

	
	std::vector<char*> item_buffer;//缓冲区指针
	std::vector<int> buffer_len;//每个缓冲区的实际长度
	int read_position;//读缓冲区位置
	int write_position;//写缓冲区位置
	std::mutex mtx;//缓冲区互斥锁
	std::condition_variable repo_not_full;//条件变量：缓冲区不满
	std::condition_variable repo_not_empty;//条件变量：缓冲区不空

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
