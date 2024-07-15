#pragma once
#include <string>
#include <map>
#include <functional>
#include "CommonDefs.h"
#include "Event.h"

using namespace std;

class Subscription
{
public:
	Topic m_topicId;
	SubscriberId m_subscriberId;
	SubscriptionType m_subscriptionType;
	std::function<void(Event)> m_handlerFunction; // This will be called when publisher publish event.

	Subscription(Topic topicId, SubscriberId subscriberId,
		SubscriptionType subscriptionType, std::function<void(Event)> handlerFunction)
	{
		m_topicId = topicId;
		m_subscriberId = subscriberId;
		m_subscriptionType = subscriptionType;
		m_handlerFunction = handlerFunction;
	}

	Subscription(const Subscription& obj)
	{
		m_topicId = obj.m_topicId;
		m_subscriberId = obj.m_subscriberId;
		m_subscriptionType = obj.m_subscriptionType;
		m_handlerFunction = obj.m_handlerFunction;
	}

	~Subscription() = default;

	bool operator < (const Subscription& obj) const
	{
		return m_topicId < obj.m_topicId;
	}
};