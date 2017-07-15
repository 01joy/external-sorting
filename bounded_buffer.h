#ifndef BOUNDED_BUFFER_H_
#define BOUNDED_BUFFER_H_
#include <condition_variable>
#include <mutex>
#include <thread>
#include <algorithm>
#include <vector>
#include <string>
#include "sdk.h"


// https://baptiste-wicht.com/posts/2012/04/c11-concurrency-tutorial-advanced-locking-and-condition-variables.html
class BoundedBuffer {
public:
	BoundedBuffer(int capacity);
	~BoundedBuffer();

	void Deposit(Item* item);
	Item* Fetch();

private:
	std::vector<Item*> buffer_;
	int capacity_;
	int front_;
	int rear_;
	int count_;

	std::mutex lock_;
	std::condition_variable not_full_;
	std::condition_variable not_empty_;

public:
	bool hasNextItem;
};

int InternalSort(Item *item);

void Produce(BoundedBuffer& buffer, int &num_file);

void Consume(BoundedBuffer& buffer, int &num_bad);

#endif
