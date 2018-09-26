#include "ConnectionException.h"
#include <string>
#include <winsock.h>

using namespace std;


ConnectionException::ConnectionException(const string& message)
									: error_Msg(message)
{
}

char const * ConnectionException::what() const throw()
{
	return error_Msg.c_str();
}

ConnectionException::~ConnectionException()
{
}
