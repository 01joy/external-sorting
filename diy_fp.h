#ifndef DIY_FP_H_
#define DIY_FP_H_
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>

//Grisu2算法实现：https://github.com/miloyip/dtoa-benchmark
//可对照标准Grisu2算法理解

struct DiyFp {
	DiyFp() {}

	DiyFp(uint64_t f, int e) : f(f), e(e) {}

	DiyFp(double d) {
		union {
			double d;
			uint64_t u64;
		} u = { d };

		int biased_e = (u.u64 & kDpExponentMask) >> kDpSignificandSize;
		uint64_t significand = (u.u64 & kDpSignificandMask);
		if (biased_e != 0) {
			f = significand + kDpHiddenBit;
			e = biased_e - kDpExponentBias;
		}
		else {
			f = significand;
			e = kDpMinExponent + 1;
		}
	}

	DiyFp operator-(const DiyFp& rhs) const {
		assert(e == rhs.e);
		assert(f >= rhs.f);
		return DiyFp(f - rhs.f, e);
	}

	DiyFp operator*(const DiyFp& rhs) const {
#if defined(_MSC_VER) && defined(_M_AMD64)
		uint64_t h;
		uint64_t l = _umul128(f, rhs.f, &h);
		if (l & (uint64_t(1) << 63)) // rounding
			h++;
		return DiyFp(h, e + rhs.e + 64);
#elif (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) && defined(__x86_64__)
		unsigned __int128 p = static_cast<unsigned __int128>(f)* static_cast<unsigned __int128>(rhs.f);
		uint64_t h = p >> 64;
		uint64_t l = static_cast<uint64_t>(p);
		if (l & (uint64_t(1) << 63)) // rounding
			h++;
		return DiyFp(h, e + rhs.e + 64);
#else
		const uint64_t M32 = 0xFFFFFFFF;
		const uint64_t a = f >> 32;
		const uint64_t b = f & M32;
		const uint64_t c = rhs.f >> 32;
		const uint64_t d = rhs.f & M32;
		const uint64_t ac = a * c;
		const uint64_t bc = b * c;
		const uint64_t ad = a * d;
		const uint64_t bd = b * d;
		uint64_t tmp = (bd >> 32) + (ad & M32) + (bc & M32);
		tmp += 1U << 31;  /// mult_round
		return DiyFp(ac + (ad >> 32) + (bc >> 32) + (tmp >> 32), e + rhs.e + 64);
#endif
	}

	//浮点数规格化过程
	DiyFp Normalize() const {
#if defined(_MSC_VER) && defined(_M_AMD64)
		unsigned long index;
		_BitScanReverse64(&index, f);
		return DiyFp(f << (63 - index), e - (63 - index));
#elif defined(__GNUC__)
		int s = __builtin_clzll(f);
		return DiyFp(f << s, e - s);
#else
		DiyFp res = *this;
		while (!(res.f & kDpHiddenBit)) {//不断左移，直到最高位第53位为1
			res.f <<= 1;
			res.e--;
		}
		//最后一次移位，成为64位（因为DiyFp精度为64）
		res.f <<= (kDiySignificandSize - kDpSignificandSize - 1);
		res.e = res.e - (kDiySignificandSize - kDpSignificandSize - 1);
		return res;
#endif
	}

	//规格化边界值
	DiyFp NormalizeBoundary() const {
#if defined(_MSC_VER) && defined(_M_AMD64)
		unsigned long index;
		_BitScanReverse64(&index, f);
		return DiyFp(f << (63 - index), e - (63 - index));
#else
		DiyFp res = *this;
		while (!(res.f & (kDpHiddenBit << 1))) {//因为在NormalizedBoundaries中f<<1，所以最高位相应左移一位
			res.f <<= 1;
			res.e--;
		}
		res.f <<= (kDiySignificandSize - kDpSignificandSize - 2);//这里相应-2
		res.e = res.e - (kDiySignificandSize - kDpSignificandSize - 2);
		return res;
#endif
	}

	//求规格化左右边界值
	void NormalizedBoundaries(DiyFp* minus, DiyFp* plus) const {
		DiyFp pl = DiyFp((f << 1) + 1, e - 1).NormalizeBoundary();//+0.5ulp
		DiyFp mi = (f == kDpHiddenBit) ? DiyFp((f << 2) - 1, e - 2) : DiyFp((f << 1) - 1, e - 1);//-0.5ulp
		mi.f <<= mi.e - pl.e;
		mi.e = pl.e;
		*plus = pl;
		*minus = mi;
	}

	static const int kDiySignificandSize = 64;//DiyFp尾数位数
	static const int kDpSignificandSize = 52;//IEEE754 double尾数位数

	//英文wiki：浮点数为(1.b_51b_50...b_0)_2*2^(e-1023)
	//把小数点去掉就是(1b_51b_50...b_0)_2*2^(e-1023-52)=(1b_51b_50...b_0)_2*2^(e-1075),所以尾数为f+hidden,新的bias=1075
	static const int kDpExponentBias = 0x3FF + kDpSignificandSize;
	static const int kDpMinExponent = -kDpExponentBias;
	static const uint64_t kDpExponentMask = 0x7FF0000000000000;//指数掩码
	static const uint64_t kDpSignificandMask = 0x000FFFFFFFFFFFFF;//尾数掩码
	static const uint64_t kDpHiddenBit = 0x0010000000000000;//隐藏位，第53位

	uint64_t f;
	int e;
};

//返回10^K的规格化浮点数
inline DiyFp GetCachedPower(int e, int* K) {
	// 10^-348, 10^-340, ..., 10^340
	static const uint64_t kCachedPowers_F[] = {
		0xfa8fd5a0081c0288, 0xbaaee17fa23ebf76,
		0x8b16fb203055ac76, 0xcf42894a5dce35ea,
		0x9a6bb0aa55653b2d, 0xe61acf033d1a45df,
		0xab70fe17c79ac6ca, 0xff77b1fcbebcdc4f,
		0xbe5691ef416bd60c, 0x8dd01fad907ffc3c,
		0xd3515c2831559a83, 0x9d71ac8fada6c9b5,
		0xea9c227723ee8bcb, 0xaecc49914078536d,
		0x823c12795db6ce57, 0xc21094364dfb5637,
		0x9096ea6f3848984f, 0xd77485cb25823ac7,
		0xa086cfcd97bf97f4, 0xef340a98172aace5,
		0xb23867fb2a35b28e, 0x84c8d4dfd2c63f3b,
		0xc5dd44271ad3cdba, 0x936b9fcebb25c996,
		0xdbac6c247d62a584, 0xa3ab66580d5fdaf6,
		0xf3e2f893dec3f126, 0xb5b5ada8aaff80b8,
		0x87625f056c7c4a8b, 0xc9bcff6034c13053,
		0x964e858c91ba2655, 0xdff9772470297ebd,
		0xa6dfbd9fb8e5b88f, 0xf8a95fcf88747d94,
		0xb94470938fa89bcf, 0x8a08f0f8bf0f156b,
		0xcdb02555653131b6, 0x993fe2c6d07b7fac,
		0xe45c10c42a2b3b06, 0xaa242499697392d3,
		0xfd87b5f28300ca0e, 0xbce5086492111aeb,
		0x8cbccc096f5088cc, 0xd1b71758e219652c,
		0x9c40000000000000, 0xe8d4a51000000000,
		0xad78ebc5ac620000, 0x813f3978f8940984,
		0xc097ce7bc90715b3, 0x8f7e32ce7bea5c70,
		0xd5d238a4abe98068, 0x9f4f2726179a2245,
		0xed63a231d4c4fb27, 0xb0de65388cc8ada8,
		0x83c7088e1aab65db, 0xc45d1df942711d9a,
		0x924d692ca61be758, 0xda01ee641a708dea,
		0xa26da3999aef774a, 0xf209787bb47d6b85,
		0xb454e4a179dd1877, 0x865b86925b9bc5c2,
		0xc83553c5c8965d3d, 0x952ab45cfa97a0b3,
		0xde469fbd99a05fe3, 0xa59bc234db398c25,
		0xf6c69a72a3989f5c, 0xb7dcbf5354e9bece,
		0x88fcf317f22241e2, 0xcc20ce9bd35c78a5,
		0x98165af37b2153df, 0xe2a0b5dc971f303a,
		0xa8d9d1535ce3b396, 0xfb9b7cd9a4a7443c,
		0xbb764c4ca7a44410, 0x8bab8eefb6409c1a,
		0xd01fef10a657842c, 0x9b10a4e5e9913129,
		0xe7109bfba19c0c9d, 0xac2820d9623bf429,
		0x80444b5e7aa7cf85, 0xbf21e44003acdd2d,
		0x8e679c2f5e44ff8f, 0xd433179d9c8cb841,
		0x9e19db92b4e31ba9, 0xeb96bf6ebadf77d9,
		0xaf87023b9bf0ee6b
	};
	static const int16_t kCachedPowers_E[] = {
		-1220, -1193, -1166, -1140, -1113, -1087, -1060, -1034, -1007, -980,
		-954, -927, -901, -874, -847, -821, -794, -768, -741, -715,
		-688, -661, -635, -608, -582, -555, -529, -502, -475, -449,
		-422, -396, -369, -343, -316, -289, -263, -236, -210, -183,
		-157, -130, -103, -77, -50, -24, 3, 30, 56, 83,
		109, 136, 162, 189, 216, 242, 269, 295, 322, 348,
		375, 402, 428, 455, 481, 508, 534, 561, 588, 614,
		641, 667, 694, 720, 747, 774, 800, 827, 853, 880,
		907, 933, 960, 986, 1013, 1039, 1066
	};

	//int k = static_cast<int>(ceil((-61 - e) * 0.30102999566398114)) + 374;
	double dk = (-61 - e) * 0.30102999566398114 + 347;	// dk must be positive, so can do ceiling in positive
	int k = static_cast<int>(dk);
	if (k != dk)
		k++;

	unsigned index = static_cast<unsigned>((k >> 3) + 1);
	*K = -(-348 + static_cast<int>(index << 3));	// decimal exponent no need lookup table

	assert(index < sizeof(kCachedPowers_F) / sizeof(kCachedPowers_F[0]));
	return DiyFp(kCachedPowers_F[index], kCachedPowers_E[index]);
}

//inline void GrisuRound(char* buffer, int len, uint64_t delta, uint64_t rest, uint64_t ten_kappa, uint64_t wp_w) {
//	while (rest < wp_w && delta - rest >= ten_kappa &&
//		(rest + ten_kappa < wp_w ||  /// closer
//		wp_w - rest > rest + ten_kappa - wp_w)) {
//		buffer[len - 1]--;
//		rest += ten_kappa;
//	}
//}

inline unsigned CountDecimalDigit32(uint32_t n) {
	// Simple pure C++ implementation was faster than __builtin_clz version in this situation.
	if (n < 10) return 1;
	if (n < 100) return 2;
	if (n < 1000) return 3;
	if (n < 10000) return 4;
	if (n < 100000) return 5;
	if (n < 1000000) return 6;
	if (n < 10000000) return 7;
	if (n < 100000000) return 8;
	if (n < 1000000000) return 9;
	return 10;
}

inline const char* GetDigitsLut() {
	static const char cDigitsLut[200] = {
		'0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0', '7', '0', '8', '0', '9',
		'1', '0', '1', '1', '1', '2', '1', '3', '1', '4', '1', '5', '1', '6', '1', '7', '1', '8', '1', '9',
		'2', '0', '2', '1', '2', '2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7', '2', '8', '2', '9',
		'3', '0', '3', '1', '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3', '7', '3', '8', '3', '9',
		'4', '0', '4', '1', '4', '2', '4', '3', '4', '4', '4', '5', '4', '6', '4', '7', '4', '8', '4', '9',
		'5', '0', '5', '1', '5', '2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
		'6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6', '7', '6', '8', '6', '9',
		'7', '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5', '7', '6', '7', '7', '7', '8', '7', '9',
		'8', '0', '8', '1', '8', '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9',
		'9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9', '7', '9', '8', '9', '9'
	};
	return cDigitsLut;
}

inline void WriteExponent(int K, char* buffer) {
	if (K < 0) {
		*buffer++ = '-';
		K = -K;
	}
	else
		*buffer++ = '+';

	if (K >= 100) {
		*buffer++ = '0' + static_cast<char>(K / 100);
		K %= 100;
		const char* d = GetDigitsLut() + K * 2;
		*buffer++ = d[0];
		*buffer++ = d[1];
	}
	else if (K >= 10) {
		const char* d = GetDigitsLut() + K * 2;
		*buffer++ = '0';
		*buffer++ = d[0];
		*buffer++ = d[1];
	}
	else
	{
		*buffer++ = '0';
		*buffer++ = '0';
		*buffer++ = '0' + static_cast<char>(K);
	}

	*buffer = '\n';
}

inline void DigitGen(const DiyFp& W, const DiyFp& Mp, uint64_t delta, char* buffer, int* len, int* K) {
	static const uint32_t kPow10[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };
	const DiyFp one(uint64_t(1) << -Mp.e, Mp.e);
	const DiyFp wp_w = Mp - W;
	uint32_t p1 = Mp.f >> -one.e;
	uint64_t p2 = Mp.f & (one.f - 1);
	int kappa = CountDecimalDigit32(p1),d;
	*len = 0;

	bool done = false;

	while (kappa > 0) {
		//uint32_t d;
		switch (kappa) {
		case 10: d = p1 / 1000000000; p1 %= 1000000000; break;
		case  9: d = p1 / 100000000; p1 %= 100000000; break;
		case  8: d = p1 / 10000000; p1 %= 10000000; break;
		case  7: d = p1 / 1000000; p1 %= 1000000; break;
		case  6: d = p1 / 100000; p1 %= 100000; break;
		case  5: d = p1 / 10000; p1 %= 10000; break;
		case  4: d = p1 / 1000; p1 %= 1000; break;
		case  3: d = p1 / 100; p1 %= 100; break;
		case  2: d = p1 / 10; p1 %= 10; break;
		case  1: d = p1;              p1 = 0; break;
		default:
#if defined(_MSC_VER)
			__assume(0);
#elif __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5)
			__builtin_unreachable();
#else
			d = 0;
#endif
		}
		if (d || *len)
			buffer[(*len)++] = '0' + d;
		if (*len == 1)(*len)++; // skip dot
		kappa--;
		uint64_t tmp = (((uint64_t)p1) << -one.e) + p2;
		if (tmp <= delta) {
			*K += kappa;
			done = true;
			//GrisuRound(buffer, *len, delta, tmp, static_cast<uint64_t>(kPow10[kappa]) << -one.e, wp_w.f);
			break;
		}
	}

	// kappa = 0
	if (!done)
	{
		do {
			p2 *= 10;
			delta *= 10;
			d = p2 >> -one.e;
			if (d || *len)
				buffer[(*len)++] = '0' + d;
			if (*len == 1)(*len)++; //skip dot
			kappa--;
			if (*len > 11)break;
			p2 &= one.f - 1;
		} while (p2 > delta);
		*K += kappa;
	}

	//以下代码特定为了实现%16.9e的效果，不需要额外prettify//陈镇霖20150310
	buffer[1] = '.';
	int k = 10;
	if ((*len) > 11 && buffer[11] > '4')
	{
		buffer[10]++;

		while (buffer[k] > '9'&&k != 0)
		{
			buffer[k] = '0';
			if (k - 1 == 1)k--;//小数点
			buffer[--k]++;
		}

	}
	if ((*len) <= 11)
	{
		memset(&buffer[*len], '0', 11 - (*len));
	}
	(*len)--;//减掉小数点
	if ((*len) > 11)//*len==12
		(*len)--;//因为最后一位舍去了，所以长度减掉
	buffer[11] = 'E';
	if (k == 0 && buffer[0]>'9')//9.9999999999999999->10.00000000000000->1.000000000000e1
	{
		buffer[0] = '1';
		(*len)++;
	}
	WriteExponent((*len) + (*K) - 1, &buffer[12]);//因为科学计数法的小数点前还有一个数字，所以指数是length + k-1
}

inline void Grisu2(double value, char* buffer, int* length, int* K) {
	const DiyFp v(value);
	DiyFp w_m, w_p;
	v.NormalizedBoundaries(&w_m, &w_p);

	const DiyFp c_mk = GetCachedPower(w_p.e, K);
	const DiyFp W = v.Normalize() * c_mk;
	DiyFp Wp = w_p * c_mk;
	DiyFp Wm = w_m * c_mk;
	Wm.f++;
	Wp.f--;
	DigitGen(W, Wp, Wp.f - Wm.f, buffer, length, K);
}

//相当于自定义sprintf，将value转换为符合%16.9E的格式写入buffer，并使指针往后移；buffer2=&buffer
inline void MiloDToA(double value, char* buffer,char*& buffer2) {
	// Not handling NaN and inf
	//assert(!isnan(value));
	//assert(!isinf(value));

	int offset = 17;
	if (value == 0) {
		buffer[0] = '0';
		buffer[1] = '.';
		memset(&buffer[2], '0', 9);
		buffer[11] = 'E';
		buffer[12] = '+';
		buffer[13] = '0';
		buffer[14] = '0';
		buffer[15] = '0';
		buffer[16] = '\n';
	}
	else {
		if (value < 0) {
			*buffer++ = '-';
			value = -value;
			offset = 18;
		}
		int length, K;
		Grisu2(value, buffer, &length, &K);
		//Prettify(buffer, length, K);
	}
	buffer2 += offset;//写入指针偏移
}

#endif
