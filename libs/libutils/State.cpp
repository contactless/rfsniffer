#include "StdAfx.h"
#include "State.h"

//  CState
CState::CState(void)
{
}

CState::~CState(void)
{
}

//  CStringState
CStringState::CStringState(void)
:CState()
{
}

CStringState::~CStringState(void)
{
}

string CStringState::getString() const
{
	return m_Value;
}
void CStringState::setString(string value)
{
	m_Value = value;
}

//  CFloatState
CFloatState::CFloatState(void)
:CState()
{
}

CFloatState::~CFloatState(void)
{
}

string CFloatState::getString() const
{
	char Buffer[30];
	snprintf(Buffer, sizeof(Buffer), "%.1f", m_Value);
	return Buffer;
}
void CFloatState::setString(string value)
{
	m_Value = (float)atof(value.c_str());
}

//  CFloatAverageState
const int AVR_COUNT=5;

CFloatAverageState::CFloatAverageState(void)
:CState()
{
}

CFloatAverageState::~CFloatAverageState(void)
{
}

string CFloatAverageState::getString() const
{
	float averageValue = m_Value.getAverage(AVR_COUNT);
	char Buffer[30];
	snprintf(Buffer, sizeof(Buffer), "%.1f", averageValue);
	return Buffer;
}
void CFloatAverageState::setString(string value)
{
	m_Value.addValue((float)atof(value.c_str()));
}

//  CIntegerState
CIntegerState::CIntegerState(void)
:CState()
{
}

CIntegerState::~CIntegerState(void)
{
}

string CIntegerState::getString() const
{
	char Buffer[30];
	snprintf(Buffer, sizeof(Buffer), "%d", m_Value);
	return Buffer;
}
void CIntegerState::setString(string value)
{
	m_Value = atoi(value.c_str());
}

