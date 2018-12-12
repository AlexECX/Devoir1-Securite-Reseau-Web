#include "SocketException.h"
#include <string>


using namespace std;


SocketException::SocketException(const string & msg) :
	std::runtime_error(msg) {
	stream_Msg = "Traceback: \n";
	stream_Msg += msg + ": SocketException";
}

SocketException::SocketException(const string& msg, const char *file, int line, string f) :
	std::runtime_error(msg) {
	stream_Msg = "Traceback:";
	stream_Msg += "\n  File \"" + string(file) + "\", line " + to_string(line) + " in " + f;
	stream_Msg += "\nSocketException: " + msg;

}

char const * SocketException::what() const throw()
{
	return stream_Msg.c_str();
}

SocketException::~SocketException()
{
}

std::string formatedWSAError()
{
	int err = WSAGetLastError();
	char msgbuf[256];   // for a message up to 255 bytes.
	msgbuf[0] = '\0';    // Microsoft doesn't guarantee this on man page.

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
		NULL,                // lpsource
		err,                 // message id
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
		msgbuf,              // output buffer
		sizeof(msgbuf),     // size of msgbuf, bytes
		NULL);               // va_list of arguments

	if (msgbuf[0] == '\0') {
		return std::to_string(err);
	}
	else {
		return std::string(msgbuf);
	}
}
