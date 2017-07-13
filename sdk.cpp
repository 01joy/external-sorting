/*
 * sdk.cpp
 *
 *  Created on: 2017Äê7ÔÂ13ÈÕ
 *      Author: zhenlin
 */

#include "sdk.h"

using namespace std;

// small function
static inline bool DoesFileExist(const std::string path)
{
	ifstream file(path.c_str());
	if(!file) {
		file.close();
		return false;
	}
	file.close();
	return true;
}

bool ParseParamFile(const string& path)
{
	if(!DoesFileExist(path)) return false;

	SearchParameter &sp = SearchParameter::GetInstance();

	ifstream param_file(path);
	string line;
	while(getline(param_file, line)){
		if(line.find('=') == string::npos) continue;
		size_t pos = line.find('=');
		string key = line.substr(0, pos);
		string value = line.substr(pos + 1, line.find(";") - pos - 1);
		if(key == "path_input") sp.path_input_ = value;
		else if(key == "path_output") sp.path_output_ = value;
		else if(key == "num_thread") sp.num_thread = stoi(value);
		else if(key == "max_char_per_file") {
			sp.max_char_per_file = stoi(value);
			sp.max_double_per_file = sp.max_char_per_file / 15;
		}
	}
	param_file.close();
	return true;
}
