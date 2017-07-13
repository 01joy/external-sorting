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

class SearchParameter {
public:
	static SearchParameter& GetInstance() {
		static SearchParameter instance;
		return instance;
	}
	std::string path_input_;
	std::string path_output_;
	int num_thread;
	int max_char_per_file;
	int max_char_per_line;
	int max_double_per_file;
private:
	SearchParameter() : path_input_(""), path_output_(""), num_thread(1), max_char_per_file(100000000), max_char_per_line(30), max_double_per_file(max_char_per_file / 15) {};
	SearchParameter(const SearchParameter&){};
	SearchParameter& operator=(const SearchParameter&) {};
	~SearchParameter() {};
};


// small function
static inline bool DoesFileExist(const std::string path);

bool ParseParamFile(const std::string& path);


#endif /* SDK_H_ */
