#include "ConnectionException.h"
#include <string>
#include <winsock.h>

using namespace std;


ConnectionException::ConnectionException(const std::string& msg, const char *file, int line) :
	std::runtime_error(msg) {
	stream_Msg = "Traceback:";
	stream_Msg += "\n  File \"" + std::string(file) + "\", line " + std::to_string(line);
	stream_Msg += "\nConnectionException: " + msg;

}

char const * ConnectionException::what() const throw()
{
	return stream_Msg.c_str();
}

ConnectionException::~ConnectionException()
{
}
