#ifndef __mymd_md_h__
#define __mymd_md_h__

const int FILEREAD_BUFSIZE = 16*1024*1024;
#ifdef _MSC_VER
	#pragma once
	#include <vector>
	using std::vector;
#endif

enum {
    MASK_MD5     = 1,
    MASK_MD4     = 2,
    MASK_SHA1    = 4,
    MASK_ED2K    = 8,
    MASK_SEGED2K = 16,
};

typedef unsigned long ui32;
typedef unsigned short ui16;

#ifdef WIN32
	//typedef __int64 i64;
	#define i386
	#define LITTLE_ENDIAN   1234    /* LSB first: i386, vax */
	#define BIG_ENDIAN      4321    /* MSB first: 68000, ibm, net */
	#define PDP_ENDIAN      3412    /* LSB first in word, MSW first in long */
	#define BYTE_ORDER  LITTLE_ENDIAN
	//#ifndef WIN32
	//#define WIN32 _WIN32
	//#endif
#endif

const int aaa=BYTE_ORDER;

#define SHA_CBLOCK      64
#define SHA_LBLOCK      16
#define SHA_BLOCK       16
#define SHA_LAST_BLOCK  56
#define SHA_LENGTH_BLOCK 8
#define SHA_DIGEST_LENGTH 20

struct SHA_CTX {
        ui32 h0, h1, h2, h3, h4;
        ui32 Nl, Nh;
        ui32 data[SHA_LBLOCK];
        int num;
};
#define SHA1_CTX        SHA_CTX

void    SHA_Init(SHA_CTX *c);
void    SHA_Update(SHA_CTX *c, const unsigned char *data, size_t len);
void    SHA_Final(unsigned char *md, SHA_CTX *c);
char   *SHA_End(SHA_CTX *, char *);
char   *SHA_File(const char *, char *);
char   *SHA_Data(const unsigned char *, unsigned int, char *);
void    LIBUTILS_API SHA1_Init(SHA_CTX *c);
void    LIBUTILS_API SHA1_Update(SHA_CTX *c, const unsigned char *data, size_t len);
void    LIBUTILS_API SHA1_Final(unsigned char *md, SHA_CTX *c);
char   *SHA1_End(SHA_CTX *, char *);
char   *SHA1_File(const char *, char *);
char   *SHA1_Data(const unsigned char *, unsigned int, char *);

#define Encode memcpy
#define Decode memcpy
#endif // __mymd_md_h__
