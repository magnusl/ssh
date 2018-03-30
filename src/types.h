#ifndef _TYPES_H_
#define _TYPES_H_

#ifdef __GNUC__
typedef uint32_t    uint32;
typedef int32_t     int32
typedef uint64_t    uint64
typedef uint16_t    uint16;
typedef int16_t     int16;
#elif _MSC_VER
typedef unsigned __int32 uint32;
typedef signed __int32 int32;
typedef unsigned __int16 uint16;
typedef signed __int16 int16;
typedef unsigned __int64 uint64;
#else
#error The compiler used is not supported
#endif

typedef unsigned char byte;
#endif