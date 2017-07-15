#include <cstdio>
#include "bounded_buffer.h"

using namespace std;

BoundedBuffer::BoundedBuffer(int capacity) : capacity_(capacity), front_(0), rear_(0), count_(0), hasNextItem(true) {
	buffer_.resize(capacity_);
}

BoundedBuffer::~BoundedBuffer() {
}

void BoundedBuffer::Deposit(Item* item) {
	std::unique_lock<std::mutex> l(lock_);

	not_full_.wait(l, [this]() {return count_ != capacity_; });

	buffer_[rear_] = item;
	rear_ = (rear_ + 1) % capacity_;
	++count_;

	not_empty_.notify_one();
}

Item* BoundedBuffer::Fetch() {
	if(count_ == 0 && hasNextItem == false) return NULL;

	std::unique_lock<std::mutex> l(lock_);

	not_empty_.wait(l, [this]() {return count_ != 0; });

	Item* result = buffer_[front_];
	front_ = (front_ + 1) % capacity_;
	--count_;

	not_full_.notify_one();

	return result;
}

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

	//sort(nums.begin(), nums.end());
	RadixSort(nums);

	ofstream os(to_string(id).c_str(), ios::binary);
	os.write((char*)&nums[0], nums.size() * kDoubleSize);
	os.close();

	return num_bad;
}

void Produce(BoundedBuffer &buffer, int &num_file) {
	const SearchParameter &sp = SearchParameter::GetInstance();
	ifstream is(sp.path_input_, ios::binary);
	num_file = 0;
	int len, j;
	char c;
	bool reach_end_of_line;
	while (is.peek() != EOF) {
		char * content = new char[sp.max_char_per_file_ + sp.max_char_per_line_];
		is.read(content, sp.max_char_per_file_);
		len = is.gcount(); // real length
		j = sp.max_char_per_file_;
		reach_end_of_line = true;

		while (is.get(c) && c != '\n') {
			content[j++] = c;
			reach_end_of_line = false;
		}
		Item *item = NULL;
		if(!reach_end_of_line) len = j;

		content[len] = '\n';
		content[len + 1] = '\0';
		item = new Item(num_file++, len + 1, content);

		buffer.Deposit(item);
	}
	is.close();
	buffer.hasNextItem = false;
}

void Consume(BoundedBuffer &buffer, int &num_bad) {
	num_bad = 0;
	while (true) {
		Item *item = buffer.Fetch();
		if(item == NULL) break;
		num_bad += InternalSort(item);
	}
}

