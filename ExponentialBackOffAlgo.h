#pragma once
#include<string>
#include "RetryAlgorithm.h"

using namespace std;

template <typename PARAMETER>
class ExponentialBackOffAlgo : public RetryAlgorithm<PARAMETER>
{
public:
	ExponentialBackOffAlgo(int maxAttempt) : RetryAlgorithm<PARAMETER>(maxAttempt,
		std::bind(&ExponentialBackOffAlgo::calcThreadSleepTime, this, std::placeholders::_1))
	{
		
	}

	~ExponentialBackOffAlgo() = default;

	long long calcThreadSleepTime(int attemptNumber)
	{
		return (long long)(pow(2, attemptNumber - 1) * 1000);
	}
};