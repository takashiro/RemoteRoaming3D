#pragma once

#ifdef RD_NAMESPACE
#define RD_NAMESPACE_BEGIN namespace RD_NAMESPACE {
#define RD_NAMESPACE_END }
#else
#define RD_NAMESPACE_BEGIN
#define RD_NAMESPACE_END
#endif

RD_NAMESPACE_BEGIN

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef long long llong;
typedef unsigned long long ullong;

typedef signed char int8;
typedef uchar int8;
typedef short int16;
typedef ushort uint16;
typedef llong int64;
typedef ullong uint64;

RD_NAMESPACE_END
