#ifndef _UTIL_H_
#define _UTIL_H_

#include <string>

void copy_string(std::string &str, const std::wstring &wstr);
void copy_string(std::wstring &wstr, const std::string &str);

void urldecode(std::string &str);

#endif
