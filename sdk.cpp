/*
 * sdk.cpp
 *
 *  Created on: 2017年7月13日
 *      Author: zhenlin
 */

#include "sdk.h"

using namespace std;

// small function
static inline bool DoesFileExist(const std::string path) {
	ifstream file(path.c_str());
	if(!file) {
		file.close();
		return false;
	}
	file.close();
	return true;
}

bool ParseParamFile(const string& path) {
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
		else if(key == "num_thread") sp.num_thread_ = stoi(value);
		else if(key == "max_char_per_file") {
			sp.max_char_per_file_ = stoi(value);
			sp.max_double_per_file_ = sp.max_char_per_file_ / 15;
		}
	}
	param_file.close();
	return true;
}

bool IsLegalNumber(const char *line) {
	if (!isdigit(line[0]) && line[0] != '-' && line[0] != '+') return false;
	int offset = 1, e_pos = -1, E_pos = -1, dot_pos = -1;
	char cur = *(line + offset);
	while (cur != '\0') {
		if (cur == '.') {
			if (dot_pos == -1)
				dot_pos = offset;
			else
				return false; // many dots
		} else if (cur == 'e') {
			if (dot_pos == -1) return false; // e before dot
			if (e_pos != -1 || E_pos != -1) return false;
			char tmp = *(line + offset + 1);
			e_pos = offset;
			if (tmp == '+' || tmp=='-') offset++;
		} else if (cur == 'E') {
			if (dot_pos == -1) return false;
			if (e_pos != -1 || E_pos != -1) return false;
			char tmp = *(line + offset + 1);
			E_pos = offset;
			if (tmp == '+' || tmp=='-') offset++;
		} else if (!isdigit(cur)) {
			return false;
		}
		offset++;
		cur = *(line + offset);
	}
	return true;
}

// http://www.leapsecond.com/tools/fast_atof.c
double FastAToF(const char *p) {
	int frac, digit_num = 0;//frac指示是否为小数，digit_num数字个数
	double sign, value, scale;//sign指示符号，value尾数的值，scale指数的值

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
				value += 6 / pow10;//如果第11位是‘5’，强行改成‘6’，这样就不会有四舍五入的问题
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
		//if (expon > 308) expon = 308;//合法的浮点数，不可能超过308

		// Calculate scaling factor.

		while (expon >= 50) { scale *= 1E50; expon -= 50; }
		while (expon >= 8) { scale *= 1E8;  expon -= 8; }
		while (expon >   0) { scale *= 10.0; expon -= 1; }
	}

	// Return signed and scaled floating point result.

	return sign * (frac ? (value / scale) : (value * scale));
}

// http://stackoverflow.com/questions/2685035/is-there-a-good-radixsort-implementation-for-floats-in-c-sharp
void RadixSort(std::vector<double> &nums) {
	int n = nums.size();
	vector<LL> t(n), a(n);

	for (int i = 0; i < n; i++)
		a[i] = *(LL*)(&nums[i]);

	int groupLength = 16;
	int bitLength = 64;
	int len = 1 << groupLength;

	vector<int> count(len), pref(len);

	int groups = bitLength / groupLength;
	int mask = len - 1;
	int negatives = 0, positives = 0;

	for (int c = 0, shift = 0; c < groups; c++, shift += groupLength) {
		// reset count array
		for (int j = 0; j < len; j++)
			count[j] = 0;

		// counting elements of the c-th group
		for (int i = 0; i < n; i++) {
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
		for (int i = 0; i < n; i++) {
			// Get the right index to sort the number in
			int index = pref[(a[i] >> shift) & mask]++;

			if (c == groups - 1) {
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
		if (c != groups - 1) {
			for (int j = 0; j < n; j++)
				a[j] = t[j];
		}

	}

	// Convert back the ints to the double array
	for (int i = 0; i < n; i++)
		nums[i] = *(double*)(&t[i]);
}
