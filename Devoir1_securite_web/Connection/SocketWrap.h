#pragma once
#include <winsock.h>


class SocketWrap
{
private:
	SOCKET theSocket;
public:
	SocketWrap(SOCKET socket) {
		theSocket = socket;
	}
	~SocketWrap() {
		closesocket(theSocket);
	}

	SOCKET& getTheSocket() {
		return theSocket;
	}
};