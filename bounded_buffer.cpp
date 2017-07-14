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


//对*buffer字符串按'\n'分割，转换为double，然后排序，生成第order个排好序的小文件；len是*buffer的长度
int InternalSort(const SearchParameter &sp, Item *item) {
	char *&content = item->content_;
	const int &len = item->len_;

	vector<double> nums;
	int i = 0, num_bad = 0;

	while (i != len) {
		int j = i;
		while (*(content + j) != '\n') ++j;

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
	
	ofstream os(to_string(item->id_).c_str(), ios::binary);
	os.write((char*)&nums, nums.size() * kDoubleSize);
	os.close();

	return num_bad;
}


void Produce(const SearchParameter &sp, BoundedBuffer& buffer, int &num_file) {
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
		Item *item = new Item(num_file++, len, content);
		buffer.Deposit(item);
	}
	buffer.hasNextItem = false;
	is.close();
}


void Consume(const SearchParameter &sp, BoundedBuffer& buffer, int &num_bad) {
	num_bad = 0;
	while (true) {
		if(!buffer.hasNextItem) break;
		Item *item = buffer.Fetch();
		num_bad += InternalSort(sp, item);
	}
}



