#include "ConnectionException.h"
#include <string>
#include <winsock.h>

using namespace std;


ConnectionException::ConnectionException(const string & msg) :
	std::runtime_error(msg) {
	stream_Msg = msg;
}

ConnectionException::ConnectionException(const string& msg, const char *file, int line, string f) :
	std::runtime_error(msg) {
	stream_Msg = "Traceback:";
	stream_Msg += "\n  File \"" + string(file) + "\", line " + to_string(line) + " in " + f;
	stream_Msg += "\nConnectionException: " + msg;

}

char const * ConnectionException::what() const throw()
{
	return stream_Msg.c_str();
}

ConnectionException::~ConnectionException()
{
}
