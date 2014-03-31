
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
