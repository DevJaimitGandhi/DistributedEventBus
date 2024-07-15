#pragma once
#include <exception>
#include <string>

using namespace std;

class RetryAbleException : public exception
{
private:
	string m_msg;

public:
	RetryAbleException(const char* msg)
	{
		m_msg = msg;
	}

	~RetryAbleException() = default;

	// Override the what() method to return our message 
	const char* what() const throw()
	{
		return m_msg.c_str();
	}
};
