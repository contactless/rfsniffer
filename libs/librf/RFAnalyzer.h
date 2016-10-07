#pragma once
#include "stdafx.h"
#include "rflib.h"
#include "RFProtocol.h"

typedef map<base_type, int> TBase2Int;
typedef map<base_type, double> TBase2Float;

class CRFAnalyzer
{
	CLog *m_Log;
public:
	CRFAnalyzer(CLog *log);
	~CRFAnalyzer();

	void Analyze(base_type*, size_t len);
	void Clean(base_type* src, size_t srcLen, base_type* dest, size_t *destLen);
	void GetMax(int* src, size_t srcLen, int *maxValue, size_t *maxPos);
	void FillHystogram(base_type* src, size_t srcLen, bool usePulse, base_type *maxLen, int minCount, int* dest, size_t destLen);
	void FillHystogramEx(base_type* src, size_t srcLen, bool usePulse, TBase2Float *result);

};

