#pragma once
#include "libutils.h"
#include "MovingAverage.h"

class LIBUTILS_API CState
{
public:
	CState(void);
	virtual ~CState(void);
	virtual string getString() const = 0;
	virtual void setString(string value) = 0;
};

class LIBUTILS_API CStringState:
	public CState
{
	string m_Value;
public:
	CStringState(void);
	virtual ~CStringState(void);
	virtual string getString() const;
	virtual void setString(string value);
};

class LIBUTILS_API CFloatState:
	public CState
{
	float m_Value;
public:
	CFloatState(void);
	virtual ~CFloatState(void);
	virtual string getString() const;
	virtual void setString(string value);
};

typedef CMovingAverage<float, 60>  LIBUTILS_API CFloatMovingAverage;

class LIBUTILS_API CFloatAverageState:
	public CState
{
	CFloatMovingAverage m_Value;
public:
	CFloatAverageState(void);
	virtual ~CFloatAverageState(void);
	virtual string getString() const;
	virtual void setString(string value);
};

class LIBUTILS_API CIntegerState:
	public CState
{
	int m_Value;
public:
	CIntegerState(void);
	virtual ~CIntegerState(void);
	virtual string getString() const;
	virtual void setString(string value);
};
