
#include "util.h"
#include <Windows.h>
#include <memory.h>

std::wstring str2wstr(const std::string &str)
{
	const char *bytes = str.c_str();
	int length = (int) MultiByteToWideChar(CP_UTF8, 0, bytes, (int) str.length(), NULL, 0);
	wchar_t *wbytes = new wchar_t[length + 1];
	memset(wbytes, 0, sizeof(wbytes));
	MultiByteToWideChar(CP_UTF8, 0, bytes, (int) str.length(), wbytes, length);
	std::wstring wstr(wbytes);
	delete[] wbytes;
	return wstr;
}

std::string wstr2str(const std::wstring &wstr)
{
	const wchar_t *wbytes = wstr.c_str();
	int length = (int) WideCharToMultiByte(CP_UTF8, 0, wbytes, (int) wstr.length(), NULL, 0, NULL, NULL);
	char *bytes = new char[length + 1];
	memset(bytes, 0, sizeof(bytes));
	WideCharToMultiByte(CP_UTF8, 0, wbytes, (int) wstr.length(), bytes, length, NULL, NULL);
	std::string str(bytes);
	delete[] bytes;
	return str;
}
