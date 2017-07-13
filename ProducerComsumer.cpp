#include "ProducerComsumer.h"

using namespace std;

ProducerComsumer::ProducerComsumer(int kItemRepositorySize, int MAX_CHAR_NUM_PER_FILE, int MAX_CHAR_NUM_PER_LINE, int MAX_DOUBLE_NUM_PER_FILE)
{
	this->kItemRepositorySize = kItemRepositorySize;
	this->MAX_CHAR_NUM_PER_FILE = MAX_CHAR_NUM_PER_FILE;
	this->MAX_CHAR_NUM_PER_LINE = MAX_CHAR_NUM_PER_LINE;
	this->MAX_DOUBLE_NUM_PER_FILE = MAX_DOUBLE_NUM_PER_FILE;
	all_done = false;
	item_buffer.resize(kItemRepositorySize);
	buffer_len.resize(kItemRepositorySize);
	bad.assign(100, 0);//假设最多有100个文件，每个文件初始非法条目为0

	write_position = 0;
	read_position = 0;
	num_file = 0;

	CHAR_SIZE = sizeof(char);
	DOUBLE_SIZE = sizeof(double);
}

//对*buffer字符串按'\n'分割，转换为double，然后排序，生成第order个排好序的小文件；len是*buffer的长度
void ProducerComsumer::partition_sort(char *buffer,int len, int order)
{
	double *dbuffer = new double[MAX_DOUBLE_NUM_PER_FILE];//用于存储转换后的double
	char out_name[3], *pch = new char[MAX_CHAR_NUM_PER_LINE];//*pch为某一个double字符串
	int i=0,j,k = 0, bad_tmp = 0;
	
	while (i!=len)//比*(buffer+i)!='\0'快
	{
		j = 0;
		while (*(buffer + i) != '\n')
		{
			pch[j++] = *(buffer + i);
			i++;
		}
		pch[j] = '\0';
		i++;
		if (IsLegalNumber(pch))
		{
			dbuffer[k++] = FastAToF(pch);
		}
		else
		{
			printf("%s\n", pch);
			bad_tmp++;
		}
	}
	delete[] buffer;
	bad[order] = bad_tmp;//当i≠j时，可以同时写入bad[i]和bad[j]，因为地址不同
	//sort(dbuffer, dbuffer + k);
	RadixSort(dbuffer, k);
	
	sprintf(out_name, "%d", order);
	ofstream file_output(out_name, ios::binary);//二进制写入，更快
	file_output.write((char*)dbuffer, k*DOUBLE_SIZE);//写入实际的长度
	file_output.close();
	//cout << "生成文件" << order << '\n';

	delete[] dbuffer;
}

//生产一个Item，将item和len放入缓冲区
void ProducerComsumer::ProduceItem(char* item,int len)
{
	unique_lock<mutex> lock(mtx);
	while (((write_position + 1) % kItemRepositorySize) == read_position) { // 缓冲区满，等待
		//cout << "读线程正在等待一个空的缓冲区...\n\n";
		(repo_not_full).wait(lock);
	}

	item_buffer[write_position] = item;
	buffer_len[write_position] = len;
	write_position++;

	if (write_position == kItemRepositorySize)
		write_position = 0;

	repo_not_empty.notify_all();
	lock.unlock();
}

//生产任务
void ProducerComsumer::ProducerTask(ifstream& is)
{
	int j, len;
	char c;
	bool reach_end_of_line;//是否到达某一行的末尾\n
	while (is.peek() != EOF)//不断从is读取数据，直到文件尾
	{
		char * buffer = new char[MAX_CHAR_NUM_PER_FILE + MAX_CHAR_NUM_PER_LINE];
		//memset(buffer, 0, CHAR_SIZE*(MAX_CHAR_NUM_PER_FILE + MAX_CHAR_NUM_PER_LINE));//要清空为0，否则会导致写入的时候计算strlen数值错误//20150311不需要了，因为添加了\0哨兵，且传出了buffer实际长度
		is.read(buffer, MAX_CHAR_NUM_PER_FILE);
		len = is.gcount();//实际读取长度
		j = MAX_CHAR_NUM_PER_FILE;
		reach_end_of_line = true;
		while (is.get(c) && c != 0x0a)
		{
			buffer[j++] = c;
			reach_end_of_line = false;
		}
		if (reach_end_of_line)
			ProduceItem(buffer, len);
		else
			ProduceItem(buffer, j);
		//cout << "读线程" << this_thread::get_id() << "正在读取数据...\n\n";
	}
	all_done = true;
	is.close();
	//cout << "读线程" << this_thread::get_id() << "正在退出...\n";
}

//消费一个Item
bool ProducerComsumer::ConsumeItem()
{
	unique_lock<mutex> lock(mtx);

	while (write_position == read_position) {// 缓冲区空，等待
		if (all_done)
		{
			lock.unlock();
			return false;//当所有数据都处理完时，退出
		}

		//cout << "排序线程正在等待数据...\n\n";
		repo_not_empty.wait(lock);
	}

	int order = num_file++;

	char* data = item_buffer[read_position];
	int len = buffer_len[read_position];

	char* buffer = new char[len + 2];
	memcpy(buffer, data, len);//先把数据拷贝下来
	
	delete[] data;

	read_position++;

	if (read_position >= kItemRepositorySize)
		read_position = 0;

	repo_not_full.notify_all();
	lock.unlock();//解锁

	buffer[len] = '\n';//添加哨兵，方便partition
	buffer[len + 1] = '\0';
	partition_sort(buffer, len+1,order);//处理数据

	return true;
}

//消费任务
void ProducerComsumer::ConsumerTask()
{
	while (ConsumeItem()) {
		//cout << "排序线程" << this_thread::get_id() << "正在消费数据...\n\n";
	}
	//cout << "排序线程" << this_thread::get_id() << "正在退出...\n";
}

//获取非法条目总数
int ProducerComsumer::get_bad_num()
{
	int ans = 0;
	for (int i = 0; i < num_file; i++)
		ans += bad[i];
	return ans-1;//减一的原因是，输入数据末尾有一个\n,ConsumeItem后面又加了一个\n\0，导致多出一个*pch=\0
}

//获取文件数目
int ProducerComsumer::get_file_num()
{
	return num_file;
}

ProducerComsumer::~ProducerComsumer()
{
}
