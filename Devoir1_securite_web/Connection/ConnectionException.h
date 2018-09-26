#pragma once
#include <exception>
#include <string>

class ConnectionException :
	public std::exception
{
private:
	std::string error_Msg;
public:
	ConnectionException(const std::string& message);
	virtual char const* what() const throw();
	~ConnectionException();
};

