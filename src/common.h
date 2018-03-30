#ifndef _COMMON_H_
#define _COMMON_H_

#include "types.h"

/* Some common macros for memory managment.
 */
#define SAFE_DELETE(x) delete x; x = 0;
#define SAFE_DELETE_ARRAY(x) delete [] x; x = 0;

uint64 endian_swap64(uint64 x);
uint32 endian_swap32(uint32 x);
uint16 endian_swap16(uint16 x);

#if defined(_M_IX86) || defined(__i386__) || defined(_M_X64)
#define __LITTLE_ENDIAN__
#endif

#ifdef __LITTLE_ENDIAN__
#define __ntohl64(x) endian_swap64(x)
#define __htonl64(x) endian_swap64(x)
#define __ntohl32(x) endian_swap32(x)
#define __htonl32(x) endian_swap32(x)
#define __ntohl16(x) endian_swap16(x)
#define __htonl16(x) endian_swap16(x)

#else 
#define __ntohl64(x) x
#define __htonl64(x) x
#define __ntohl64(x) x
#define __htonl64(x) x
#define __ntohl32(x) x
#define __htonl32(x) x
#define __ntohl16(x) x
#define __htonl16(x) x
#endif

#ifdef _DEBUG
#define DebugPrint(x) wcerr << __FUNCTION__ << L": " << x << endl;
#else
#define DebugPrint(x)
#endif


#endif
