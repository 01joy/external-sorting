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
	bad.assign(100, 0);//���������100���ļ���ÿ���ļ���ʼ�Ƿ���ĿΪ0

	write_position = 0;
	read_position = 0;
	num_file = 0;

	CHAR_SIZE = sizeof(char);
	DOUBLE_SIZE = sizeof(double);
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
	bad[order] = bad_tmp;//��i��jʱ������ͬʱд��bad[i]��bad[j]����Ϊ��ַ��ͬ
	//sort(dbuffer, dbuffer + k);
	RadixSort(dbuffer, k);
	
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
