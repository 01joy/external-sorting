#include "bounded_buffer.h"

using namespace std;

void BoundedBuffer::Deposit(Item* item) {
	std::unique_lock<std::mutex> l(lock_);

	not_full_.wait(l, [this]() {return count_ != capacity_; });

	buffer_[rear_] = item;
	rear_ = (rear_ + 1) % capacity_;
	++count_;

	not_empty_.notify_one();
}

Item* BoundedBuffer::Fetch() {
	std::unique_lock<std::mutex> l(lock_);

	not_empty_.wait(l, [this]() {return count_ != 0; });

	Item* result = buffer_[front_];
	front_ = (front_ + 1) % capacity_;
	--count_;

	not_full_.notify_one();

	return result;
}


//��*buffer�ַ�����'\n'�ָת��Ϊdouble��Ȼ���������ɵ�order���ź����С�ļ���len��*buffer�ĳ���
int InternalSort(Item *item) {
	const SearchParameter &sp = SearchParameter::GetInstance();

	char *&content = item->content_;
	const int &len = item->len_;
	const int id = item->id_;

	vector<double> nums;
	int i = 0, num_bad = 0;

	while (i < len) {
		int j = i;
		while (j < len && *(content + j) != '\n') ++j;

		*(content + j) = '\0';

		if (IsLegalNumber(content + i)) {
			nums.push_back(FastAToF(content + i));
		} else {
			printf("%s\n", content + i);
			++num_bad;
		}
		i = ++j;
	}

	delete item;
	item = NULL;

	sort(nums.begin(), nums.end());
	//RadixSort(nums);
	cout<<"nums.size:"<<nums.size()<<endl;
	ofstream os(to_string(id).c_str(), ios::binary);
	os.write((char*)&nums, nums.size() * kDoubleSize);
	os.close();

	return num_bad;
}

void Produce(BoundedBuffer &buffer, int &num_file) {
	const SearchParameter &sp = SearchParameter::GetInstance();
	ifstream is(sp.path_input_, ios::binary);
	num_file = 0;
	int len;
	char c;
	while (is.peek() != EOF) {
		char * content = new char[sp.max_char_per_file_ + sp.max_char_per_line_];
		is.read(content, sp.max_char_per_file_);
		len = is.gcount(); // real length
		while (is.get(c) && c != '\n') {
			content[len++] = c;
		}
		while (*(content + len - 1) == '\n') {
			content[--len] = '\0';
		}
		Item *item = new Item(num_file++, len, content);
		buffer.Deposit(item);
	}
	buffer.hasNextItem = false;
	is.close();
}


void Consume(BoundedBuffer &buffer, int &num_bad) {
	num_bad = 0;
	while (true) {
		if(!buffer.hasNextItem) break;
		Item *item = buffer.Fetch();
		num_bad += InternalSort(item);
	}
}

