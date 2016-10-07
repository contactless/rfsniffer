#include "stdafx.h"
#include "RFAnalyzer.h"


CRFAnalyzer::CRFAnalyzer(CLog *log)
	:m_Log(log)
{
}


CRFAnalyzer::~CRFAnalyzer()
{
}


void CRFAnalyzer::Analyze(base_type* rawData, size_t rawLen)
{
	base_type *data = new base_type[rawLen+1];
	size_t len;
	Clean(rawData, rawLen, data, &len);
	TBase2Float pulseDistribution, pauseDistribution;
	FillHystogramEx(data, len, true, &pulseDistribution);


	// Ищем максимум
	base_type max_pulse=0, max_pause = 0;
	const base_type pulseLimit = 20000, pauseLimit = 10000;

	for (int pos = 0; pos < len; pos++)
	{
		base_type len = CRFProtocol::getLengh(data[pos]);

		if (CRFProtocol::isPulse(data[pos]))
		{
			if (len > max_pulse && len < pulseLimit)
				max_pulse = len;
		}
		else
		{
			if (len > max_pause && len < pauseLimit)
				max_pause = len;
		}
	}

	const int cellCount = 100; 
//	base_type maxLen = max(max_pulse, max_pause);
//	base_type step = maxLen / cellCount;

	int Pulses[cellCount+1]; 
	FillHystogram(data, len, true, &max_pulse, 3, Pulses, cellCount);

	int Pauses[cellCount+1]; 
	FillHystogram(data, len, false, &max_pause, 3, Pauses, cellCount);

	int PulsesDerivative[cellCount];
	int PausesDerivative[cellCount];

	for (int pos = 0; pos < cellCount-1; pos++)
	{
		PulsesDerivative[pos] = Pulses[pos + 1] - Pulses[pos];
		PausesDerivative[pos] = Pauses[pos + 1] - Pauses[pos];
	}

	PulsesDerivative[cellCount - 1] = 0;
	PausesDerivative[cellCount - 1] = 0;

	int maxVal;
	size_t maxPos;
	GetMax(Pulses, cellCount, &maxVal, &maxPos);


	delete[] data;
}


void CRFAnalyzer::Clean(base_type* src, size_t srcLen, base_type* dest, size_t *destLen)
{
	const base_type MIN_PAUSE = 20;
	const base_type MIN_PULSE = 20;

	base_type* destPtr = dest;
	*destLen = 0;
	*dest = 0;
	bool inFix = false;

	for (int i = 0; i < srcLen; i++)
	{
		base_type len = CRFProtocol::getLengh(src[i]);
		bool isPulse = CRFProtocol::isPulse(src[i]);

		if (inFix && isPulse == CRFProtocol::isPulse(*destPtr))
		{
			inFix = false;
			*destPtr += len;
		}
		else if (isPulse && len < MIN_PULSE)
		{
			inFix = true;
			*destPtr += len;
		}
		else if (!isPulse && len < MIN_PAUSE)
		{
			inFix = true;
			*destPtr += len;
		}
		else
		{
			if (*destPtr || destPtr!=dest)
				destPtr++;

			inFix = false;
			*destPtr = src[i];
			(*destLen)++;
		}
	}
}


void CRFAnalyzer::GetMax(int* src, size_t srcLen, int *maxValue, size_t *maxPos)
{
	*maxValue = 0;
	*maxPos = 0;

	for (int pos = 0; pos < srcLen - 1; pos++)
	{
		if (*maxValue < src[pos])
		{
			*maxValue = src[pos];
			*maxPos = pos;
		}
	}
}

void CRFAnalyzer::FillHystogram(base_type* src, size_t srcLen, bool usePulse, base_type *maxLen, int minCount, int* dest, size_t destLen)
{
	bool bRecalc = false;

	do
	{
		base_type step = (*maxLen) / destLen;
		memset(dest, 0, sizeof(int)*destLen);

		for (int pos = 0; pos < srcLen; pos++)
		{
			base_type len = min(*maxLen, CRFProtocol::getLengh(src[pos]));

			if (CRFProtocol::isPulse(src[pos]) == usePulse)
			{
				if (len < *maxLen)
					dest[len / step]++;
			}
		}

		for (int pos = destLen - 1; pos > 0; pos--)
		{
			if (dest[pos] >= minCount)
				break;
			else
			{
				maxLen -= step;
				bRecalc = true;
			}
		}
	} while (bRecalc);
}


void CRFAnalyzer::FillHystogramEx(base_type* src, size_t srcLen, bool usePulse, TBase2Float *result)
{
	TBase2Int sorted;

	for (int pos = 0; pos < srcLen; pos++)
	{
		sorted[src[pos]]++;
	}

}

