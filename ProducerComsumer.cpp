#include "ProducerComsumer.h"


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

//判断字符串*c是否是合法条目
bool ProducerComsumer::is_number(char * c)
{
	if (!isdigit(c[0]) && c[0] != '-'&&c[0] != '+')//开头可以是加号+
		return false;
	int i = 1, ep = -1, Ep = -1, dotp = -1;//i为c的下标，后3个分别为字符'e','E','.'的位置
	char cc = *(c + i);//当前字符
	while (cc != '\0')
	{
		if (cc == '.')
		{
			if (dotp == -1)
				dotp = i;
			else//有多个点.
				return false;
		}
		else if (cc == 'e')
		{
			if (dotp == -1)//e出现在小数点的前面
				return false;
			if (ep != -1 || Ep != -1)
				return false;
			char tmp = *(c + i + 1);
			ep = i;
			if (tmp == '+' || tmp=='-')
			{
				i++;
			}
		}
		else if (cc == 'E')
		{
			if (dotp == -1)
				return false;
			if (ep != -1 || Ep != -1)
				return false;
			char tmp = *(c + i + 1);
			Ep = i;
			if (tmp == '+' || tmp=='-')
			{
				i++;
			}
		}
		else if (!isdigit(cc))
			return false;

		i++;
		cc = *(c + i);
	}
	return true;
}

//将*p字符串转换为double，修改自 http://www.leapsecond.com/tools/fast_atof.c
double ProducerComsumer::fast_atof(const char *p)
{
	int frac , digit_num = 0;//frac指示是否为小数，digit_num数字个数
	double sign, value, scale;//sign指示符号，value尾数的值，scale指数的值

	// Get sign, if any.

	sign = 1.0;
	if (*p == '-') {
		sign = -1.0;
		p += 1;

	}
	else if (*p == '+') {
		p += 1;
	}

	// Get digits before decimal point or exponent, if any.

	for (value = 0.0; isdigit(*p); p += 1) {
		value = value * 10.0 + (*p - '0');
		digit_num++;
	}

	// Get digits after decimal point, if any.

	if (*p == '.') {
		double pow10 = 10.0;
		p += 1;
		while (isdigit(*p)) {
			digit_num++;
			if (digit_num == 11 && (*p) == '5')
				value += 6 / pow10;//如果第11位是‘5’，强行改成‘6’，这样就不会有四舍五入的问题
			else
				value += (*p - '0') / pow10;
			pow10 *= 10.0;
			p += 1;
		}
	}

	// Handle exponent, if any.

	frac = 0;
	scale = 1.0;
	if ((*p == 'e') || (*p == 'E')) {
		unsigned int expon;

		// Get sign of exponent, if any.

		p += 1;
		if (*p == '-') {
			frac = 1;
			p += 1;

		}
		else if (*p == '+') {
			p += 1;
		}

		// Get digits of exponent, if any.

		for (expon = 0; isdigit(*p); p += 1) {
			expon = expon * 10 + (*p - '0');
		}
		//if (expon > 308) expon = 308;//合法的浮点数，不可能超过308

		// Calculate scaling factor.

		while (expon >= 50) { scale *= 1E50; expon -= 50; }
		while (expon >= 8) { scale *= 1E8;  expon -= 8; }
		while (expon >   0) { scale *= 10.0; expon -= 1; }
	}

	// Return signed and scaled floating point result.

	return sign * (frac ? (value / scale) : (value * scale));
}

//对double array[]数组基数排序，n为数组大小，参考 http://stackoverflow.com/questions/2685035/is-there-a-good-radixsort-implementation-for-floats-in-c-sharp
void ProducerComsumer::radix_sort(double array[], int n)
{
	LL *t = new LL[n];
	LL *a = new LL[n];
	for (int i = 0; i < n; i++)
		a[i] = *(LL*)(&array[i]);//将double的二进制转换为long long

	int groupLength = 16;//可自定义
	int bitLength = 64;
	int len = 1 << groupLength;

	int *count = new int[len];
	int *pref = new int[len];
	int groups = bitLength / groupLength;
	int mask = len - 1;
	int negatives = 0, positives = 0;

	for (int c = 0, shift = 0; c < groups; c++, shift += groupLength)
	{
		// reset count array 
		for (int j = 0; j < len; j++)
			count[j] = 0;

		// counting elements of the c-th group 
		for (int i = 0; i < n; i++)
		{
			count[(a[i] >> shift) & mask]++;

			// additionally count all negative 
			// values in first round
			if (c == 0 && a[i] < 0)
				negatives++;
		}
		if (c == 0) positives = n - negatives;

		// calculating prefixes
		pref[0] = 0;
		for (int i = 1; i < len; i++)
			pref[i] = pref[i - 1] + count[i - 1];

		// from a[] to t[] elements ordered by c-th group 
		for (int i = 0; i < n; i++)
		{
			// Get the right index to sort the number in
			int index = pref[(a[i] >> shift) & mask]++;

			if (c == groups - 1)
			{
				// We're in the last (most significant) group, if the
				// number is negative, order them inversely in front
				// of the array, pushing positive ones back.
				if (a[i] < 0)
					index = positives - (index - negatives) - 1;
				else
					index += negatives;
			}
			t[index] = a[i];
		}

		// a[]=t[] and start again until the last group 
		if (c != groups - 1)
		{
			for (int j = 0; j < n; j++)
				a[j] = t[j];
		}

	}
	delete[] a;
	delete[] count;
	delete[] pref;

	// Convert back the ints to the double array
	for (int i = 0; i < n; i++)
		array[i] = *(double*)(&t[i]);//重新把long long 的二进制转换为double

	delete[] t;
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
		if (is_number(pch))
		{
			dbuffer[k++] = fast_atof(pch);
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
	radix_sort(dbuffer, k);
	
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
