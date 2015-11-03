
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

static char hextonumber(char ch){
	return '0' <= ch && ch <= '9' ? ch - '0' : 'A' <= ch && ch <= 'F' ? ch - 'A' + 10 : 0;
}

void urldecode(std::string &str)
{
	int limit = str.length() - 2;
	for (int i = 0; i < limit; i++) {
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
