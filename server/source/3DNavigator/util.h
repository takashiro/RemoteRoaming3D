#ifndef _UTIL_H_
#define _UTIL_H_

#include <string>

void copy_string(std::string &str, const std::wstring &wstr);
void copy_string(std::wstring &wstr, const std::string &str);

void urldecode(std::string &str);

std::string base64_encode(const unsigned char *data, unsigned long long length);
std::string base64_encode(const std::string &str);

std::string sha1(const char *data, unsigned long long length);
std::string sha1(const std::string &str);

#endif
