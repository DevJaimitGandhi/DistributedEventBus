#pragma once
#include <exception>
#include <string>

using namespace std;

class RetryLimitExceededException : public exception
{
private:
	string m_msg;

public:
	RetryLimitExceededException(const char* msg)
	{
		m_msg = msg;
	}

	~RetryLimitExceededException() = default;

	// Override the what() method to return our message 
	const char* what() const throw()
	{
		return m_msg.c_str();
	}
};