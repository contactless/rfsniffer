#pragma once

#pragma warning (disable : 4251)
template <class T, unsigned int DefaultLimit> class  CMovingAverage
{
	unsigned int m_Limit;
	vector<T> m_Values;
public:

	CMovingAverage(void)
	{
		m_Limit = DefaultLimit;
	}

	virtual ~CMovingAverage(void)
	{
	}

	T getAverage(int count) const
	{
		if (m_Values.size()==0)
			return 0;

		T avr=0;
		int i=m_Values.size()-count;

		if (i<0)
		{
			i=0;
			count=m_Values.size();
		}

		for (;i<(int)m_Values.size();i++)
			avr+=m_Values[i];

		return avr/count;
	}

	void addValue(T value)
	{
		m_Values.push_back(value);
		if (m_Values.size()>m_Limit)
			m_Values.erase(m_Values.begin());
	}

	void setValue(T value)
	{
		m_Values.clear();
		m_Values.push_back(value);
	}
};
