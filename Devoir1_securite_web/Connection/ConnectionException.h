#pragma once
#include <exception>
#include <stdexcept>
#include <string>
#include <sstream>

class ConnectionException : public std::runtime_error
{
private:
	std::string stream_Msg;

public:
	ConnectionException(const std::string& msg, const char *file, int line);
			
	virtual char const* what() const throw();

	~ConnectionException() throw();
};

