#pragma once
#include <stdexcept>
#include <winsock.h>
#include "Utils.h"

#define WSA_ERROR "Winsock Error "+formateWSAError(WSAGetLastError())
#define TRACEBACK __FILE__, __LINE__, __FUNCTION__

std::string formatedWSAError();

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

