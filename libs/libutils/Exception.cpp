#include "Exception.h"
#include "logging.h"

typedef std::string string;


std::vector<std::string> CHaException::generateTypeDescriptions()
{
    std::vector<std::string> descriptions(CHaException::ErrorCodes::ErrLast + 1);
    // bad but fast
#define ADD_ERROR_DEFAULT(err) descriptions[err] = #err
    ADD_ERROR_DEFAULT(ErrNoError);
    ADD_ERROR_DEFAULT(ErrInvalidConfig);
    ADD_ERROR_DEFAULT(ErrCannotConnectQueueManager);
    ADD_ERROR_DEFAULT(ErrCannotOpenQueue);
    ADD_ERROR_DEFAULT(ErrInvalidQueueType);
    ADD_ERROR_DEFAULT(ErrRoutingError);
    ADD_ERROR_DEFAULT(ErrEngineInitError);
    ADD_ERROR_DEFAULT(ErrAuditInitError);
    ADD_ERROR_DEFAULT(ErrLogInitError);
    ADD_ERROR_DEFAULT(ErrSyncFailed);
    ADD_ERROR_DEFAULT(ErrCannotInitXerces);
    ADD_ERROR_DEFAULT(ErrInvalidMessageFormat);
    ADD_ERROR_DEFAULT(ErrConnectorError);
    ADD_ERROR_DEFAULT(ErrRemoteException);
    ADD_ERROR_DEFAULT(ErrXMLParsingError);
    ADD_ERROR_DEFAULT(ErrNotInitialized);
    ADD_ERROR_DEFAULT(ErrXMLError);
    ADD_ERROR_DEFAULT(ErrJobError);
    ADD_ERROR_DEFAULT(ErrAConnectorError);
    ADD_ERROR_DEFAULT(ErrAttributeNotFound);
    ADD_ERROR_DEFAULT(ErrNotImplemented);
    ADD_ERROR_DEFAULT(ErrCreateSocketError);
    ADD_ERROR_DEFAULT(ErrConnectStringError);
    ADD_ERROR_DEFAULT(ErrConnectError);
    ADD_ERROR_DEFAULT(ErrNotConnected);
    ADD_ERROR_DEFAULT(ErrBindError);
    ADD_ERROR_DEFAULT(ErrAcceptError);
    ADD_ERROR_DEFAULT(ErrSendError);
    ADD_ERROR_DEFAULT(ErrRecvError);
    ADD_ERROR_DEFAULT(ErrParsingError);
    ADD_ERROR_DEFAULT(ErrSendMessageError);
    ADD_ERROR_DEFAULT(ErrRecvMessageError);
    ADD_ERROR_DEFAULT(ErrSerialize);
    ADD_ERROR_DEFAULT(ErrBadPacketType);
    ADD_ERROR_DEFAULT(ErrTimeout);
    ADD_ERROR_DEFAULT(ErrSystemAPIError);
    ADD_ERROR_DEFAULT(ErrBadParam);
    ADD_ERROR_DEFAULT(ErrLuaException);

    ADD_ERROR_DEFAULT(ErrLast);

#undef ADD_ERROR_DEFAULT
    return descriptions;
}

std::vector<std::string> CHaException::m_TypeDescriptions =
    CHaException::generateTypeDescriptions();


CHaException::CHaException(void)
{
    m_code = ErrNoError;
    m_Message = "No Error";
}

CHaException::CHaException(ErrorCodes code, string Message)
{
    m_code = code;
    m_Message = Message;
    m_Explanation = std::string("CHaException") + " (ErrorCodes::" +
            m_TypeDescriptions[m_code] + "): " + m_Message;
}

CHaException::CHaException(const CHaException &ex)
{
    m_code = ex.m_code;
    m_Message = ex.m_Message;
    m_Explanation = ex.m_Explanation;
}


CHaException::CHaException(ErrorCodes code, const char *Format, ...)
{
    va_list marker;
    va_start (marker, Format);
    char Buffer[4096];
#ifdef _WIN32_WCE
    vsprintf(Buffer, Format, marker);
#elif defined (WIN32)
    vsprintf_s(Buffer, sizeof(Buffer), Format, marker);
#else
    vsnprintf(Buffer, sizeof(Buffer), Format, marker);
#endif
    va_end (marker);
    m_code = code;
    m_Message = Buffer;
    m_Explanation = GetExplanation();
}

CHaException::~CHaException(void)
{
}

