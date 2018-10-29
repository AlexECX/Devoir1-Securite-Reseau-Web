#pragma once
#include <stdexcept>
#include <string>

#define WSA_ERROR "Winsock error "+std::to_string(WSAGetLastError())
#define TRACEBACK __FILE__, __LINE__, __FUNCTION__

class ConnectionException : public std::runtime_error
{
private:
	std::string stream_Msg;

public:
	ConnectionException(const std::string& msg);
	ConnectionException(const std::string& msg, const char *file, 
						 int line, std::string f);
			
	virtual char const* what() const throw();

	~ConnectionException() throw();
};

