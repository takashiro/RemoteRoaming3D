
#include "util.h"
#include <Windows.h>
#include <memory.h>
#include <iostream>
#include <stdlib.h>

void copy_string(std::wstring &wstr, const std::string &str)
{
	const char *bytes = str.c_str();
	int length = (int) MultiByteToWideChar(CP_UTF8, 0, bytes, (int) str.length(), NULL, 0);
	wchar_t *wbytes = new wchar_t[length + 1];
	MultiByteToWideChar(CP_UTF8, 0, bytes, (int) str.length(), wbytes, length);
	wbytes[length] = 0;
	wstr = wbytes;
	delete[] wbytes;
}

void copy_string(std::string &str, std::wstring &wstr)
{
	const wchar_t *wbytes = wstr.c_str();
	int length = (int) WideCharToMultiByte(CP_UTF8, 0, wbytes, (int) wstr.length(), NULL, 0, NULL, NULL);
	char *bytes = new char[length + 1];
	WideCharToMultiByte(CP_UTF8, 0, wbytes, (int) wstr.length(), bytes, length, NULL, NULL);
	bytes[length] = 0;
	str = bytes;
	delete[] bytes;
}

static char hextonumber(char ch)
{
	return '0' <= ch && ch <= '9' ? ch - '0' : 'A' <= ch && ch <= 'F' ? ch - 'A' + 10 : 0;
}

void urldecode(std::string &str)
{
	size_t limit = str.length() - 2;
	for (size_t i = 0; i < limit; i++) {
		if (str[i] == '%') {
			char ch1 = hextonumber(str[i + 1]);
			char ch2 = hextonumber(str[i + 2]);
			ch1 <<= 4;
			ch1 |= ch2;
			str.replace(i, 3, &ch1, 1);
			limit -= 2;
		}
	}
}

std::string base64_encode(const unsigned char *data, unsigned long long length)
{
	const char *index_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	std::string result;
	unsigned long long i, max;
	for (i = 2, max = length; i < max; i += 3) {
		result += index_table[data[i - 2] >> 2];
		result += index_table[((data[i - 2] & 0x3) << 4) | ((data[i - 1] & 0xF0) >> 4)];
		result += index_table[((data[i - 1] & 0xF) << 2) | ((data[i] & 0xC0) >> 6)];
		result += index_table[data[i] & 0x3F];
	}

	i -= 2;
	if (i + 1 < max) {
		result += index_table[data[i] >> 2];
		result += index_table[((data[i] & 0x3) << 4) | ((data[i + 1] & 0xF0) >> 4)];
		result += index_table[(data[i + 1] & 0xF) << 2];
		result += '=';
	}
	else if (i < max) {
		result += index_table[data[i] >> 2];
		result += index_table[(data[i] & 0x3) << 4];
		result += '=';
		result += '=';
	}
	return result;
}

std::string base64_encode(const std::string &str)
{
	return base64_encode(reinterpret_cast<const unsigned char *>(str.data()), str.size());
}

void sha1_chunk(unsigned int *h, unsigned int *words)
{
	unsigned int a = h[0];
	unsigned int b = h[1];
	unsigned int c = h[2];
	unsigned int d = h[3];
	unsigned int e = h[4];

	unsigned int f, k;
	int i;

	for (i = 16; i < 80; i++) {
		words[i] = words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16];
		words[i] = (words[i] << 1) | words[i] >> 31;
	}

	for (i = 0; i < 80; i++) {
		if (i < 20) {
			f = (b & c) | (~b & d);
			k = 0x5A827999;
		}
		else if (i < 40) {
			f = b ^ c ^ d;
			k = 0x6ED9EBA1;
		}
		else if (i  < 60) {
			f = (b & c) | (b & d) | (c & d);
			k = 0x8F1BBCDC;
		}
		else {
			f = b ^ c ^ d;
			k = 0xCA62C1D6;
		}

		unsigned int tmp = (a << 5 | (a >> 27)) + f + e + k + words[i];
		e = d;
		d = c;
		c = (b << 30) | (b >> 2);
		b = a;
		a = tmp;
	}

	h[0] += a;
	h[1] += b;
	h[2] += c;
	h[3] += d;
	h[4] += e;
}

std::string sha1(const char *data, unsigned long long length)
{
	unsigned int h[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
	unsigned int words[80];
	int i = 0;

	//For each 512-bit (64-byte) chunk
	const char *chunk = data;
	unsigned long long end;
	for (end = 64; end <= length; end += 64) {
		const char *w = chunk;
		for (i = 0; i < 16; i++) {
			words[i] = (w[0] << 24) | (w[1] << 16) | (w[2] << 8) | w[3];
			w += 4;
		}
		sha1_chunk(h, words);
		chunk += 64;
	}

	//Fill the last chunk
	if (end > length) {
		end -= 64;
		if (end <= length) {
			end += 4;
			for (; end <= length; end += 4) {
				words[i] = (chunk[0] << 24) | (chunk[1] << 16) | (chunk[2] << 8) | chunk[3];
				i++;
				chunk += 4;
			}
			end -= 4;
		}
	}

	if (end > length) {
		end -= 4;
		int j = 0;
		for (; end < length; end++) {
			words[i] = (words[i] << 8) | chunk[j];
			j++;
		}
		words[i] = (words[i] << 8) | 0x80;
		j++;
		for (; j < 4; j++) {
			words[i] <<= 8;
		}
	}
	else {
		words[i] = 0x80000000;
	}

	if (i == 15) {
		sha1_chunk(h, words);
		i = -1;
	}

	for (i++; i < 14; i++) {
		words[i] = 0;
	}

	unsigned long long bits = length * 8;
	const unsigned char *from = reinterpret_cast<const unsigned char *>(&bits);
	words[14] = (from[7] << 24) | (from[6] << 16) | (from[5] << 8) | from[4];
	words[15] = (from[3] << 24) | (from[2] << 16) | (from[1] << 8) | from[0];

	sha1_chunk(h, words);

	std::string result;
	result.reserve(20);
	for (unsigned int num : h) {
		const char *w = reinterpret_cast<const char *>(&num);
		result.push_back(w[3]);
		result.push_back(w[2]);
		result.push_back(w[1]);
		result.push_back(w[0]);
	}
	return result;
}

std::string sha1(const std::string &str)
{
	return sha1(str.data(), str.size());
}
