#pragma once

#include "global.h"

#include <string>

RD_NAMESPACE_BEGIN

void copy_string(std::string &str, const std::wstring &wstr);
void copy_string(std::wstring &wstr, const std::string &str);

void urldecode(std::string &str);

std::string base64_encode(const uchar *data, uint64 length);
std::string base64_encode(const std::string &str);

std::string sha1(const char *data, uint64 length);
std::string sha1(const std::string &str);

RD_NAMESPACE_END
