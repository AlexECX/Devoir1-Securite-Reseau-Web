#pragma once
#include <exception>
#include <string>

class Connection_error :
	public std::exception
{
private:
	std::string error_Msg;
public:
	Connection_error(const std::string& message);
	virtual char const* what() const throw();
	~Connection_error();
};

