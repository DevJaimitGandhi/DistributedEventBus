#pragma once
#include<string>
#include "RetryAlgorithm.h"

using namespace std;

template <typename PARAMETER>
class PeriodicRetryAlgo : public RetryAlgorithm<PARAMETER>
{
	long long m_waitTimeInMilliSenconds;

public:
	PeriodicRetryAlgo(int maxAttempt, long long waitTimeInMilliSenconds) : RetryAlgorithm<PARAMETER>(maxAttempt,
		std::bind(&PeriodicRetryAlgo::calcThreadSleepTime, this, std::placeholders::_1))
	{
		m_waitTimeInMilliSenconds = waitTimeInMilliSenconds;
	}

	~PeriodicRetryAlgo() = default;

	long long calcThreadSleepTime(...)
	{
		return m_waitTimeInMilliSenconds;
	}
};
