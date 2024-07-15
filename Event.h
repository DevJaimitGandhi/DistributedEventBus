#pragma once
#include <string>
#include <map>
#include "CommonDefs.h"


using namespace std;

typedef struct stEvent
{
	EventId eventId;
	EventName eventName;
	TimeStamp eventTimeStamp;
	map<string, string> eventAttributes;
	
	/*stEvent()
	{

	}*/

	stEvent(EventId _eventId, EventName _eventName, TimeStamp _eventTimeStamp,
		map<string, string> _eventAttributes)
	{
		eventId = _eventId;
		eventName = _eventName;
		eventTimeStamp = _eventTimeStamp;
		eventAttributes = _eventAttributes;

	}
	~stEvent() = default;

}Event;