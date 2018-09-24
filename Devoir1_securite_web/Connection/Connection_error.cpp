#include "Connection_error.h"
#include <string>

using namespace std;


Connection_error::Connection_error(const string& message)
									: error_Msg(message)
{
}

char const * Connection_error::what() const throw()
{
	return error_Msg.c_str();
}

Connection_error::~Connection_error()
{
}
