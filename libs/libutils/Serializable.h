#pragma once

#include "libutils.h"
class CBuffer;
class CLog;

class LIBUTILS_API CSerializable
{
public:
	virtual size_t getSize() const = 0;
	virtual void Serialize(CBuffer *buffer, bool bSerialize) = 0;
	virtual void Dump(CLog *log);
};
