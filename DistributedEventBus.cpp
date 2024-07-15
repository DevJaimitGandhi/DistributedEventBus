#include <iostream>

#include<string>
#include <crtdbg.h>
#include "Event.h"
#include "RetryAlgorithm.h"
#include "EventBus.h"
#include "PeriodicRetryAlgo.h"
#include "ExponentialBackOffAlgo.h"
//using namespace std;
//
//template <typename PARAMETER>
//class PeriodicRetryAlgo1 : public RetryAlgorithm<PARAMETER>
//{
//public:
//	PeriodicRetryAlgo1(int maxAttempt, int waitTimeInMilliSenconds) : RetryAlgorithm<PARAMETER>(maxAttempt,
//		std::bind(&PeriodicRetryAlgo1::calcThreadSleepTime, this, waitTimeInMilliSenconds))
//	{
//
//	}
//
//	long long calcThreadSleepTime(...)
//	{
//		return (long long)(1000);
//	}
//};

void test(Topic topic, Event event)
{

}

class TestEventBus
{
	const Topic topic = "TOPIC_1";
public:
	
	void testDefaultBehavior()
	{
		int numThreads = 5;
		int max_attempt_to_retry = 5;
		std::shared_ptr<RetryAlgorithm<Event>> retryAlgoPtr(
					new ExponentialBackOffAlgo<Event>(max_attempt_to_retry));
		unique_ptr<EventBus> eventBusPtr(new EventBus(numThreads, nullptr, retryAlgoPtr));

		EventId eventID = "event-id-1";
		eventBusPtr->registerTopic(topic);
		eventBusPtr->publish(topic, getEvent(eventID, Timer::timeSinceEpochMillisec()));
		SubscriberId subId = "subscriber-1";
		Subscription* subscription = new Subscription(topic, subId, SubscriptionType::PULL, 0);
		eventBusPtr->subscribe(*subscription);
		Event* ev = eventBusPtr->pollEvent(topic, subId);
		_ASSERT(ev == nullptr);

		EventId eventID2 = "event-id-2";
		eventBusPtr->publish(topic, getEvent(eventID2, Timer::timeSinceEpochMillisec()));
		auto ev2 = eventBusPtr->pollEvent2(topic, subId);
		auto temp = ev2.get();
		_ASSERT(temp != nullptr);
	}

private:
	Event getEvent(EventId eventId, const TimeStamp timeStamp)
	{
		stEvent event(eventId, eventId, timeStamp, map<string, string>());
		return event;
	}
};

#include "KeyedExecutor.h"
int main()
{
	//KeyedExecutor exec(5);
	//stEvent event;
	//exec.submit("1", test, "hello", event);

	//EventBus bus(5);
	//bus.publish("Hello", event);
	//bus.pollEvent("", "");
	////ThreadPool t(5);
	////t.enqueuTask(1, test, "Hello", event);
	////PeriodicRetryAlgo1<Event> ep(1,10);
	//const string str = "1233";
	//int threadIndex = std::hash<string>()(str.c_str()) %10;
	TestEventBus testEventBus;
	testEventBus.testDefaultBehavior();

}
