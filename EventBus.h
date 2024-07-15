#pragma once
#include <iostream>
#include <queue>
#include <vector>
#include <list>
#include <map>
#include <set>
#include "CommonDefs.h"
#include "Event.h"
#include "RetryAbleException.h"
#include "RetryLimitExceededException.h"
#include "RetryAlgorithm.h"
#include "Subscription.h"
#include "KeyedExecutor.h"
#include "FailureEvent.h"
#include "Timer.h"

using namespace std;

class EventBus
{
private:
	std::shared_ptr<EventBus> m_deadLetterQueue;
	std::shared_ptr<RetryAlgorithm<Event>> m_retryAlgoPtr;

	map<Topic, vector<Event>> m_mapOfTopicAndEvents;
	map<Topic, std::set<Subscription>> m_mapOfTopicAndSubscriber;

	/*
	*	Subscriber index is to track to last fetched event of that topic.
	*		i.e. From which index for that topic, subscriber needs to poll.
	* 
	*	Event index is to move subscriber pointer to that particular event index.
	*	So that, next time, subscriber will poll from next event.
	* 
	*	Timestamp index is to point to next element (i.e. greater time stamp) of given 
	*	timestamp, so that subscriber can poll from that index.
	*		e.g. Subscriber wants to revisit all the events which occuers from last 2 days.
	*/

	map<Topic, map<SubscriberId, Index>> m_mapOfTopicAndSubscriberIndex;
	map<Topic, map<EventId, Index>> m_mapOfTopicAndEventIndex;
	map<Topic, map<TimeStamp, Index>> m_mapOfTopicAndTimestampIndex;

	KeyedExecutor executor;

public:
	EventBus(int numThreads, std::shared_ptr<EventBus> deadLetterQueue,
		std::shared_ptr<RetryAlgorithm<Event>> retryAlgoPtr) : executor(numThreads)
	{
		m_deadLetterQueue = deadLetterQueue;
		m_retryAlgoPtr = retryAlgoPtr;
	}
	/*EventBus(int numThreads) : executor(numThreads)
	{

	}*/

	~EventBus() = default;

	void registerTopic(Topic topic)
	{
		if (m_mapOfTopicAndEvents.find(topic) != m_mapOfTopicAndEvents.end())
			return;

		m_mapOfTopicAndEvents[topic];
		m_mapOfTopicAndSubscriber[topic];
		m_mapOfTopicAndSubscriberIndex[topic];
		m_mapOfTopicAndEventIndex[topic];
		m_mapOfTopicAndTimestampIndex[topic];
	}

	void subscribe(Subscription& subscription)
	{
		auto func = [&]() {
			const Topic& topic = subscription.m_topicId;
			if (m_mapOfTopicAndSubscriber.find(topic) == m_mapOfTopicAndSubscriber.end())
			{
				return;
			}
				
			m_mapOfTopicAndSubscriber.at(topic).emplace(subscription);
			Index index(m_mapOfTopicAndEvents.at(topic).size());;
			m_mapOfTopicAndSubscriberIndex.at(topic).emplace(subscription.m_subscriberId, index);
		};
		executor.submit(subscription.m_topicId, func);
	}

	void publish(Topic topic, Event event)
	{
		executor.submit(topic, 
			std::bind(&EventBus::addEventToBus, this, std::placeholders::_1, std::placeholders::_2 ), 
				topic, event);
	}

	Event* pollEvent(Topic topic, SubscriberId subId)
	{
		std::promise<Event*>* _promise = new std::promise<Event*>;
		std::future<Event*> _future = (_promise->get_future());
		auto func = [&, topic, subId]() mutable{
			if (m_mapOfTopicAndEvents.find(topic) == m_mapOfTopicAndEvents.end())
			{
				_promise->set_value(nullptr);
				return;
			}
			Index index = m_mapOfTopicAndSubscriberIndex.at(topic).at(subId);
			if (m_mapOfTopicAndEvents.at(topic).size() < (index.getValue() + 1))
			{
				//index out of bound.
				_promise->set_value(nullptr);
				return;
			}
			Event event = m_mapOfTopicAndEvents.at(topic).at(index.getValue());
			m_mapOfTopicAndSubscriberIndex.at(topic).at(subId).increment();
			_promise->set_value(new Event(event));
		};

		executor.submit(topic + subId, std::move(func));
		return _future.get();
	}

	std::future<Event*> pollEvent2(Topic topic, SubscriberId subId)
	{
		auto func = [&, topic, subId]() mutable -> Event* {
			if (m_mapOfTopicAndEvents.find(topic) == m_mapOfTopicAndEvents.end())
			{
				return nullptr;
			}
			Index index = m_mapOfTopicAndSubscriberIndex.at(topic).at(subId);
			if (m_mapOfTopicAndEvents.at(topic).size() < (index.getValue() + 1))
			{
				//index out of bound.
			
				return nullptr;
			}
			Event event = m_mapOfTopicAndEvents.at(topic).at(index.getValue());
			m_mapOfTopicAndSubscriberIndex.at(topic).at(subId).increment();
			return new Event(event);
		};
		auto task = make_shared<packaged_task<Event*()>>(func);
		std::future<Event*> fut =  task->get_future();
		executor.submit(topic + subId, [task]() {
			(*task)(); });
		return fut;
	}

	void setIndexAfterTimeStamp(Topic topic, SubscriberId subId, const TimeStamp timeStamp)
	{
		auto func = [&]() {
			if (m_mapOfTopicAndTimestampIndex.find(topic) == m_mapOfTopicAndTimestampIndex.end())
				return;

			auto itr = m_mapOfTopicAndTimestampIndex.at(topic).upper_bound(timeStamp);
			if (itr != m_mapOfTopicAndTimestampIndex.at(topic).end())
			{
				Index newIndex = itr->second;
				m_mapOfTopicAndSubscriberIndex.at(topic).at(subId) = Index(newIndex);
			}else{
				Index newIndex(m_mapOfTopicAndEvents.at(topic).size());
				m_mapOfTopicAndSubscriberIndex.at(topic).at(subId) = Index(newIndex);
			}
		};

		executor.submit(topic + subId, func);
	}

	void setIndexAfterEventId(Topic topic, SubscriberId subId, EventId eventId)
	{
		auto func = [&]() {
			if (m_mapOfTopicAndEventIndex.find(topic) == m_mapOfTopicAndEventIndex.end())
				return;

			Index index = m_mapOfTopicAndEventIndex.at(topic).at(eventId);
			index.increment();
			m_mapOfTopicAndSubscriberIndex.at(topic).at(subId) = index;
		};
		executor.submit(topic + subId, func);
	}

private:
	void addEventToBus(Topic topic, Event event)
	{
		// This means, topic is not registed. So not allowed to publish.
		if (m_mapOfTopicAndEvents.find(topic) == m_mapOfTopicAndEvents.end())
			return;

		Index newIndex(m_mapOfTopicAndEvents.at(topic).size());

		/*
		*	Update the required indexes of the map.
		*/
		m_mapOfTopicAndEventIndex.at(topic).emplace(event.eventId, newIndex);
		m_mapOfTopicAndTimestampIndex.at(topic).emplace(event.eventTimeStamp, newIndex);
		m_mapOfTopicAndEvents.at(topic).emplace_back(event);

		/*
		*	Forward(i.e. Notify) this event to all the push based subscriber for this topic.
		*/
		for (auto& subscription : m_mapOfTopicAndSubscriber.at(topic))
		{
			if (subscription.m_subscriptionType == SubscriptionType::PUSH)
			{
				pushEventToSubscriber(topic, event, subscription);
			}
		}
		std::cout << "addEventToBus Done\n";

	}

	void pushEventToSubscriber(Topic topic, Event event, Subscription subscriber)
	{
		/*
		*	For subscriber, to get max parallism, we are doing hash of (topic, subscriberId)
		*/
		executor.submit(topic + subscriber.m_topicId, [&]() {
			try {
				m_retryAlgoPtr->attempt(subscriber.m_handlerFunction, event, 1);
			}
			catch (RetryLimitExceededException& ex)
			{
				if (m_deadLetterQueue.get() != nullptr)
				{
					m_deadLetterQueue->publish(topic, 
							FailureEvent(event, "RetryLimitExceeded", Timer::timeSinceEpochMillisec()));
				}
			}
			catch (exception& ex)
			{
				//Some other threading related or something else exception.
			}
		});
	}
};

