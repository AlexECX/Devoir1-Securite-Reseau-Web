#include "Socket.h"
#include "SocketException.h"
#include <string>
#include <iostream>

using namespace std;


Socket::Socket()
{
}

Socket::Socket(SOCKET socket): BaseSocket(socket)
{
}

Socket::Socket(const string& server_addr, unsigned cPort)
{
	connectSocket(server_addr, cPort);
}


Socket::~Socket()
{
}

bool Socket::connectSocket(const string& server_addr, unsigned cPort)
{
	addrInfo.sin_family = this->af;
	addrInfo.sin_port = htons(cPort);

	addrInfo.sin_addr.s_addr = inet_addr(server_addr.c_str());
	if (addrInfo.sin_addr.s_addr == INADDR_NONE) {
		LPHOSTENT lpHostEntry = gethostbyname(server_addr.c_str());
		if (lpHostEntry == NULL) {
			throw SocketException(server_addr +" invalid parameter", TRACEBACK);
		}
		else {
			addrInfo.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
		}
	}

	if (connect(mySocket, (LPSOCKADDR)&addrInfo, sizeof(struct sockaddr)) == SOCKET_ERROR) {
		this->socketError(WSA_ERROR, __FUNCTION__);
		return false;
	}
	
	return true;
}
