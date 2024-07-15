#pragma once
#include <string>

using namespace std;

typedef string Topic;
typedef string EventId;
typedef string EventName;
typedef string SubscriberId;
typedef unsigned long long TimeStamp;

class Index
{
	int m_val;
public:
	Index(int val)
	{
		m_val = val;
	}

	~Index() = default;

	int getValue()
	{
		return m_val;
	}

	void increment()
	{
		++m_val;
	}
};

enum class SubscriptionType
{
	PULL,
	PUSH
};