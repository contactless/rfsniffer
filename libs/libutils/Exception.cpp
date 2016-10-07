#include "stdafx.h"
#include "Exception.h"
#include "Buffer.h"
#include "logging.h"

CHaException::CHaException(void)
{
	m_code = ErrNoError;
	m_Message = "No Error";
}

CHaException::CHaException(ErrorCodes code, string Message)
{
	m_code = code;
	m_Message = Message;
}

CHaException::CHaException(const CHaException& ex)
{
	m_code = ex.m_code;
	m_Message = ex.m_Message;
}


CHaException::CHaException(ErrorCodes code, const char* Format, ...)
{
	va_list marker;
	va_start (marker,Format);
	char Buffer[4096];
#ifdef _WIN32_WCE
	vsprintf(Buffer, Format, marker);
#elif defined (WIN32)
	vsprintf_s(Buffer, sizeof(Buffer), Format, marker);
#else
	vsnprintf(Buffer, sizeof(Buffer), Format, marker);
#endif
	m_code = code;
	m_Message = Buffer;
}

CHaException::~CHaException(void)
{
}


size_t CHaException::getSize() const
{
	return CBuffer::getSizeString(m_Message)+sizeof(short);
}

void CHaException::Serialize(CBuffer *buffer, bool bSerialize)
{
	short Code = m_code;
	buffer->Serialize(Code, bSerialize);
	buffer->Serialize(m_Message, bSerialize);
	m_code = (ErrorCodes)Code;
}


void CHaException::Dump(CLog *log)
{
	log->Printf(1, "CHaException::Dump(%d, '%s')", m_code, m_Message.c_str());
}
