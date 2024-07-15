#pragma once
#include <chrono>
#include "Event.h"

using namespace std;

class FailureEvent : public Event
{
	string m_failureMsg;
	TimeStamp m_timeStamp;
public:
	FailureEvent(Event event, string errorLog ,TimeStamp timestamp):
		Event(event.eventId, event.eventName, timestamp, event.eventAttributes)
	{
		m_failureMsg = errorLog;
		m_timeStamp = timestamp;
	}
};