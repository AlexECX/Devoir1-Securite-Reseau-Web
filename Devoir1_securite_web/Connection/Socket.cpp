#include "Socket.h"
#include "SocketException.h"
#include <string>

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

int Socket::connectSocket(const string& server_addr, unsigned cPort)
{
	addrInfo.sin_family = this->af;
	addrInfo.sin_port = ::htons(cPort);

	addrInfo.sin_addr.s_addr = ::inet_addr(server_addr.c_str());
	if (addrInfo.sin_addr.s_addr == INADDR_NONE) {
		LPHOSTENT lpHostEntry = ::gethostbyname(server_addr.c_str());
		if (lpHostEntry == NULL) {
			throw SocketException(server_addr +" invalid parameter", TRACEBACK);
		}
		else {
			addrInfo.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
		}
	}

	int status = ::connect(mySocket, (LPSOCKADDR)&addrInfo, sizeof(struct sockaddr));
	if (status < 0) {
		this->socketError(WSA_ERROR, __FUNCTION__);
		return status;
	}
	
	return status;
}
