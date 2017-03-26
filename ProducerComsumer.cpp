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
	bad.assign(100, 0);//���������100���ļ���ÿ���ļ���ʼ�Ƿ���ĿΪ0

	write_position = 0;
	read_position = 0;
	num_file = 0;

	CHAR_SIZE = sizeof(char);
	DOUBLE_SIZE = sizeof(double);
}

//�ж��ַ���*c�Ƿ��ǺϷ���Ŀ
bool ProducerComsumer::is_number(char * c)
{
	if (!isdigit(c[0]) && c[0] != '-'&&c[0] != '+')//��ͷ�����ǼӺ�+
		return false;
	int i = 1, ep = -1, Ep = -1, dotp = -1;//iΪc���±꣬��3���ֱ�Ϊ�ַ�'e','E','.'��λ��
	char cc = *(c + i);//��ǰ�ַ�
	while (cc != '\0')
	{
		if (cc == '.')
		{
			if (dotp == -1)
				dotp = i;
			else//�ж����.
				return false;
		}
		else if (cc == 'e')
		{
			if (dotp == -1)//e������С�����ǰ��
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

//��*p�ַ���ת��Ϊdouble���޸��� http://www.leapsecond.com/tools/fast_atof.c
double ProducerComsumer::fast_atof(const char *p)
{
	int frac , digit_num = 0;//fracָʾ�Ƿ�ΪС����digit_num���ָ���
	double sign, value, scale;//signָʾ���ţ�valueβ����ֵ��scaleָ����ֵ

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
				value += 6 / pow10;//�����11λ�ǡ�5����ǿ�иĳɡ�6���������Ͳ������������������
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
		//if (expon > 308) expon = 308;//�Ϸ��ĸ������������ܳ���308

		// Calculate scaling factor.

		while (expon >= 50) { scale *= 1E50; expon -= 50; }
		while (expon >= 8) { scale *= 1E8;  expon -= 8; }
		while (expon >   0) { scale *= 10.0; expon -= 1; }
	}

	// Return signed and scaled floating point result.

	return sign * (frac ? (value / scale) : (value * scale));
}

//��double array[]�����������nΪ�����С���ο� http://stackoverflow.com/questions/2685035/is-there-a-good-radixsort-implementation-for-floats-in-c-sharp
void ProducerComsumer::radix_sort(double array[], int n)
{
	LL *t = new LL[n];
	LL *a = new LL[n];
	for (int i = 0; i < n; i++)
		a[i] = *(LL*)(&array[i]);//��double�Ķ�����ת��Ϊlong long

	int groupLength = 16;//���Զ���
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
		array[i] = *(double*)(&t[i]);//���°�long long �Ķ�����ת��Ϊdouble

	delete[] t;
}

//��*buffer�ַ�����'\n'�ָת��Ϊdouble��Ȼ���������ɵ�order���ź����С�ļ���len��*buffer�ĳ���
void ProducerComsumer::partition_sort(char *buffer,int len, int order)
{
	double *dbuffer = new double[MAX_DOUBLE_NUM_PER_FILE];//���ڴ洢ת�����double
	char out_name[3], *pch = new char[MAX_CHAR_NUM_PER_LINE];//*pchΪĳһ��double�ַ���
	int i=0,j,k = 0, bad_tmp = 0;
	
	while (i!=len)//��*(buffer+i)!='\0'��
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
	bad[order] = bad_tmp;//��i��jʱ������ͬʱд��bad[i]��bad[j]����Ϊ��ַ��ͬ
	//sort(dbuffer, dbuffer + k);
	radix_sort(dbuffer, k);
	
	sprintf(out_name, "%d", order);
	ofstream file_output(out_name, ios::binary);//������д�룬����
	file_output.write((char*)dbuffer, k*DOUBLE_SIZE);//д��ʵ�ʵĳ���
	file_output.close();
	//cout << "�����ļ�" << order << '\n';

	delete[] dbuffer;
}

//����һ��Item����item��len���뻺����
void ProducerComsumer::ProduceItem(char* item,int len)
{
	unique_lock<mutex> lock(mtx);
	while (((write_position + 1) % kItemRepositorySize) == read_position) { // �����������ȴ�
		//cout << "���߳����ڵȴ�һ���յĻ�����...\n\n";
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

//��������
void ProducerComsumer::ProducerTask(ifstream& is)
{
	int j, len;
	char c;
	bool reach_end_of_line;//�Ƿ񵽴�ĳһ�е�ĩβ\n
	while (is.peek() != EOF)//���ϴ�is��ȡ���ݣ�ֱ���ļ�β
	{
		char * buffer = new char[MAX_CHAR_NUM_PER_FILE + MAX_CHAR_NUM_PER_LINE];
		//memset(buffer, 0, CHAR_SIZE*(MAX_CHAR_NUM_PER_FILE + MAX_CHAR_NUM_PER_LINE));//Ҫ���Ϊ0������ᵼ��д���ʱ�����strlen��ֵ����//20150311����Ҫ�ˣ���Ϊ�����\0�ڱ����Ҵ�����bufferʵ�ʳ���
		is.read(buffer, MAX_CHAR_NUM_PER_FILE);
		len = is.gcount();//ʵ�ʶ�ȡ����
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
		//cout << "���߳�" << this_thread::get_id() << "���ڶ�ȡ����...\n\n";
	}
	all_done = true;
	is.close();
	//cout << "���߳�" << this_thread::get_id() << "�����˳�...\n";
}

//����һ��Item
bool ProducerComsumer::ConsumeItem()
{
	unique_lock<mutex> lock(mtx);

	while (write_position == read_position) {// �������գ��ȴ�
		if (all_done)
		{
			lock.unlock();
			return false;//���������ݶ�������ʱ���˳�
		}

		//cout << "�����߳����ڵȴ�����...\n\n";
		repo_not_empty.wait(lock);
	}

	int order = num_file++;

	char* data = item_buffer[read_position];
	int len = buffer_len[read_position];

	char* buffer = new char[len + 2];
	memcpy(buffer, data, len);//�Ȱ����ݿ�������
	
	delete[] data;

	read_position++;

	if (read_position >= kItemRepositorySize)
		read_position = 0;

	repo_not_full.notify_all();
	lock.unlock();//����

	buffer[len] = '\n';//����ڱ�������partition
	buffer[len + 1] = '\0';
	partition_sort(buffer, len+1,order);//��������

	return true;
}

//��������
void ProducerComsumer::ConsumerTask()
{
	while (ConsumeItem()) {
		//cout << "�����߳�" << this_thread::get_id() << "������������...\n\n";
	}
	//cout << "�����߳�" << this_thread::get_id() << "�����˳�...\n";
}

//��ȡ�Ƿ���Ŀ����
int ProducerComsumer::get_bad_num()
{
	int ans = 0;
	for (int i = 0; i < num_file; i++)
		ans += bad[i];
	return ans-1;//��һ��ԭ���ǣ���������ĩβ��һ��\n,ConsumeItem�����ּ���һ��\n\0�����¶��һ��*pch=\0
}

//��ȡ�ļ���Ŀ
int ProducerComsumer::get_file_num()
{
	return num_file;
}

ProducerComsumer::~ProducerComsumer()
{
}
