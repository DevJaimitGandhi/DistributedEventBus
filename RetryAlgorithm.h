#pragma once
#include <string>
#include <thread>
#include <functional>
#include "RetryAbleException.h"
#include "RetryLimitExceededException.h"

using namespace std;

template <typename PARAMETER>
class RetryAlgorithm
{
	int m_maxAttempt;
	function<long(int)> m_funcToGetThreadSleepTime;

public:
	RetryAlgorithm(int maxAttempt, function<long long(int)> funcToGetThreadSleepTime)
	{
		m_maxAttempt = maxAttempt;
		m_funcToGetThreadSleepTime = funcToGetThreadSleepTime;
	}

	void attempt(function<void(PARAMETER)> task, PARAMETER parameter, int attemptNum)
	{
		try {
			task(parameter);
		}
		catch (RetryAbleException& ex)
		{
			if (attemptNum == m_maxAttempt)
			{
				throw RetryLimitExceededException("Max retry limit exceeded");
			}
			
			try {
				long long timeToSleep = m_funcToGetThreadSleepTime(attemptNum);
				std::this_thread::sleep_for(std::chrono::milliseconds(timeToSleep));
			}
			catch (exception& ex)
			{
				throw std::exception("Thread related exception");
			}

			attempt(task, parameter, attemptNum + 1);
		}
		catch (exception& ex)
		{
			throw std::exception(ex.what());
		}
	}
};