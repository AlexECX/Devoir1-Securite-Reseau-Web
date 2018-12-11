#pragma once
#include <stdexcept>
#include <winsock.h>


class SocketException : public std::runtime_error
{
private:
	std::string stream_Msg;

public:
	SocketException(const std::string& msg);
	SocketException(const std::string& msg, const char *file, 
						 int line, std::string f);
			
	virtual char const* what() const throw();

	~SocketException() throw();
};

