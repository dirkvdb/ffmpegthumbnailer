#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <string>

class Exception
{
public:
	Exception (const std::string& message)
	: m_Message(message)
	{
	}
	
	std::string getMessage()
	{
		return m_Message;
	}
	
private:
	std::string m_Message;	
};

#endif
