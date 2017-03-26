#include "KMerge.h"
#include"milo_dtoa.h"


KMerge::KMerge(int MAX_CHAR_NUM_PER_LINE)
{
	this->MAX_CHAR_NUM_PER_LINE = MAX_CHAR_NUM_PER_LINE;

	CHAR_SIZE = sizeof(char);
	DOUBLE_SIZE = sizeof(double);
}

//败者树调整过程
void KMerge::adjust(int *ls, double *b, int s,int num_file)
{
	for (int t = (num_file + s) / 2; t>0; t /= 2)
	{
		if (b[s]>b[ls[t]])
		{
			int tmp = s;
			s = ls[t];
			ls[t] = tmp;
		}
	}
	ls[0] = s;
}

//创建败者树
void KMerge::create_loser_tree(int *ls, double *b,int num_file)
{
	b[num_file] = -DBL_MAX;//DBL_MIN这是最小正数

	for (int i = 1; i<num_file; i++)
		ls[i] = num_file;
	for (int i = num_file - 1; i >= 0; i--)
		adjust(ls, b, i,num_file);
}


bool KMerge::write_double(ofstream& os, double* d, int n)
{
	char *buf = new char[n*18 + 10];//最大值+10
	char *current_buf = buf;
	//printf("正在转换...");
	for (int i = 0; i < n;i++)
		dtoa_milo(d[i], current_buf, current_buf);
	//printf("转换结束\n");
	unique_lock<mutex> lock(mtx);
	os.write(buf, current_buf - buf);
	lock.unlock();
	delete[] d;
	delete[] buf;
	return true;
}

//使用败者树k路归并，参数分别为输入和输出文件，败者树归并参考：http://gengning938.blog.163.com/blog/static/12822538120115131197839/
void KMerge::k_merge_to_str(vector<string> path_inputs, string path_output)
{
	int num_file = path_inputs.size();
	ifstream *is = new ifstream[num_file];
	double *b = new double[num_file + 1];//每个队列中取出来的数，最后一个数为最小值
	int *loser_tree = new int[num_file];//败者树，[0]存储最小值下标，
	int MAX_DOUBLE_NUM_PER_THREAD = 18000000;//一个线程处理的最多double数
	double *db = new double[MAX_DOUBLE_NUM_PER_THREAD];

	for (int i = 0; i<num_file; i++)
	{
		is[i].open(path_inputs[i], ios::binary);
		is[i].read((char*)&b[i], DOUBLE_SIZE);
	}

	create_loser_tree(loser_tree, b, num_file);

	ofstream file_output(path_output, ios::binary);//必须用二进制写入，这样\n才不会转换为\r\n
	
	int s_pos = loser_tree[0], t = 0;
	double smallest = b[s_pos];
	while (smallest != DBL_MAX)
	{
		db[t++] = smallest;
		
		if (t == MAX_DOUBLE_NUM_PER_THREAD)
		{
			thread td(&KMerge::write_double, this, ref(file_output), db, t);
			td.detach();//放飞线程，让他独自运行
			
			db = new double[t];//申请新的空间
			t = 0;//从头再来
		}

		if (is[s_pos].peek() == EOF)
		{
			//printf("第%d个文件归并结束...\n", s_pos);
			b[s_pos] = DBL_MAX;
		}
		else
			is[s_pos].read((char*)&b[s_pos], DOUBLE_SIZE);

		adjust(loser_tree, b, s_pos,num_file);
		s_pos = loser_tree[0];
		smallest = b[s_pos];
	}

	write_double(file_output, db, t);//主线程处理最后一个数据块
	file_output.close();

	for (int i = 0; i<num_file; i++)
		is[i].close();

	delete[] is;
	delete[] b;
	delete[] loser_tree;
}

KMerge::~KMerge()
{
}
