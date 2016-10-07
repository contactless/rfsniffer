#pragma once

#include "libutils.h"

class CSerializable;

class LIBUTILS_API CBuffer
{
	char *m_Buffer, *m_BufferPtr;
	size_t m_MaxSize, m_Size;
public:
	CBuffer(void);
	CBuffer(char *Buffer, size_t Size);
	virtual ~CBuffer(void);
	
	void Alloc(size_t Size);
	char *getBuffer();
	size_t getSize(){return m_Size;}
	void Reset(){m_BufferPtr = m_Buffer;};

	static size_t getSizeSize(size_t s);
	static size_t getSizeString(const string &s);

	void Serialize(char &data, bool bSerialize);
	void Serialize(short &data, bool bSerialize);
	void Serialize(long &data, bool bSerialize);
	void Serialize(string &data, bool bSerialize);
	void Serialize(const string &data){string s=data; Serialize(s, true);};
	void SerializeBinary(void *data, size_t size, bool bSerialize);
	void SerializeSize(size_t &size, bool bSerialize);
	void SerializeSize(size_t size){SerializeSize(size, true);};
	void Serialize(CSerializable &data, bool bSerialize, bool bAlloc);
};
