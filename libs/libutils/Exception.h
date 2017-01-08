#ifndef __ML_EXCEPTION_H
#define __ML_EXCEPTION_H

#pragma warning (disable: 4251)

#include <vector>
#include <string>
#include <iostream>


#include "libutils.h"
#include "Serializable.h"
/*******************************************
*
* Module: CHaException
*
* ����� CHaException
*
********************************************/
class LIBUTILS_API CHaException
    : public CSerializable
{
    /*******************************************
    *
    * Section:! ��������
    *
    * �������� ������ CHaException
    *
    ********************************************/

  public:
    /*******************************************
    *
    * Name: ErrorCodes
    *
    ********************************************/
    enum ErrorCodes {
        ErrNoError = 0,
        ErrInvalidConfig = 100,
        ErrCannotConnectQueueManager,
        ErrCannotOpenQueue,
        ErrInvalidQueueType,
        ErrRoutingError,
        ErrEngineInitError,
        ErrAuditInitError,
        ErrLogInitError,
        ErrSyncFailed,
        ErrCannotInitXerces,
        ErrInvalidMessageFormat,
        ErrConnectorError,
        ErrRemoteException,
        ErrXMLParsingError,
        ErrNotInitialized,
        ErrXMLError,
        ErrJobError,
        ErrAConnectorError,
        ErrAttributeNotFound,
        ErrNotImplemented,
        ErrCreateSocketError,
        ErrConnectStringError,
        ErrConnectError,
        ErrNotConnected,
        ErrBindError,
        ErrAcceptError,
        ErrSendError,
        ErrRecvError,
        ErrParsingError,
        ErrSendMessageError,
        ErrRecvMessageError,
        ErrSerialize,
        ErrBadPacketType,
        ErrTimeout,
        ErrSystemAPIError,
        ErrBadParam,
        ErrLuaException,

        ErrLast
    };

  private:
    /*******************************************
    *
    * Name: m_code
    *
    * ErrorCodes <b>m_code</b> - ��� ����������
    *
    ********************************************/
    ErrorCodes m_code;
    
    static std::vector<std::string> generateTypeDescriptions();
    static std::vector<std::string> m_TypeDescriptions;
    /*******************************************
    *
    * Name: m_Message
    *
    * string <b>m_Message</b> - ����� ����������
    *
    ********************************************/
    std::string m_Message;

  public:
    /*******************************************
    *
    * Section:! ������������
    *
    * ����������� ������ CHaException
    *
    ********************************************/
    /*******************************************
    *
    * Name: CHaException
    *
    * ���������� �� ���������
    *
    ********************************************/
    CHaException(void);
    /*******************************************
    *
    * Name: CHaException
    *
    * ����������, ��������� ���������� �� ������ �����������
    *
    * ���������:
    *       *saxEx* - ������ �� ����������
    *
    ********************************************/
    //  CHaException(const xercesc_2_3::SAXParseException& saxEx);
    /*******************************************
    *
    * Name: CHaException
    *
    * ���������� �����������
    *
    * ���������:
    *       *ex* - ������ �� ����������, ����� �������� ����� �������
    *
    ********************************************/
    CHaException(const CHaException &ex);
    /*******************************************
    *
    * Name: CHaException
    *
    * ����������, ��������� ���������� � ��������� ����� � �������
    *
    * ���������:
    *       *code* - ��� ����������
    *       *Message* - ������, �������� ����� ����������
    *
    ********************************************/
    CHaException(ErrorCodes code, std::string Message);
    CHaException(ErrorCodes code, const char *Format, ...);
    ~CHaException(void);

    /*******************************************
    *
    * Section:! ������
    *
    * ������ ������ CHaException
    *
    ********************************************/
    /*******************************************
    *
    * Name: GetCode
    *
    * ����� ���������� ��� ����������
    *
    ********************************************/
    ErrorCodes GetCode()
    {
        return m_code;
    };

    /*******************************************
    *
    * Name: GetMessage
    *
    * ����� ���������� ����� ����������
    *
    ********************************************/
    std::string GetMessage()
    {
        return m_Message;
    };
    std::string GetMsg()
    {
        return m_Message;
    };
    
    std::string GetExplanation() {
		return std::string("CHaException") + " (ErrorCodes::" + m_TypeDescriptions[m_code] + "): " + m_Message;
	};


    virtual size_t getSize() const;
    virtual void Serialize(CBuffer *buffer, bool bSerialize);
    virtual void Dump(CLog *log);
};

#define NOT_IMPLEMENTED throw CHaException(CHaException::ErrNotImplemented, "%s(%d)", __FILE__, __LINE__)

#endif
