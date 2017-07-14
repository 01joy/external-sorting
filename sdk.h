/*
 * sdk.h
 *
 *  Created on: 2017Äê7ÔÂ13ÈÕ
 *      Author: zhenlin
 */

#ifndef SDK_H_
#define SDK_H_

#include<iostream>
#include<string>
#include<fstream>

typedef long long LL;

const int kCharSize = sizeof(char);
const int kDoubleSize = sizeof(double);

class SearchParameter {
public:
	static SearchParameter& GetInstance() {
		static SearchParameter instance;
		return instance;
	}
	std::string path_input_;
	std::string path_output_;
	int num_thread_;
	int max_char_per_file_;
	int max_char_per_line_;
	int max_double_per_file_;
private:
	SearchParameter() : path_input_(""), path_output_(""), num_thread_(1), max_char_per_file_(100000000), max_char_per_line_(30), max_double_per_file_(max_char_per_file_ / 15) {};
	SearchParameter(const SearchParameter&){};
	SearchParameter& operator=(const SearchParameter&) {};
	~SearchParameter() {};
};


struct Item {
	int id_;
	int len_;
	char *content_;
	Item(): id_(0), len_(0), content_(NULL) {};
	Item(int id, int len, char *content): id_(id), len_(len), content_(content) {};
	~Item() { delete content_; };
};


// small function
static inline bool DoesFileExist(const std::string path);

bool ParseParamFile(const std::string& path);

bool IsLegalNumber(const char *line);

double FastAToF(const char *p);

void RadixSort(double array[], int n);


#endif /* SDK_H_ */
