#include "ServerSocket.h"
#include "SocketException.h"
#include <string>


using namespace std;

ServerSocket::ServerSocket()
{
}

ServerSocket::ServerSocket(unsigned cPort)
{
	bind("", cPort);
}

ServerSocket::ServerSocket(const string& server_addr, unsigned cPort)
{
	bind(server_addr, cPort);
}


ServerSocket::~ServerSocket()
{
}

int ServerSocket::bind(const string& server_addr, unsigned cPort, unsigned queue_size)
{
	addrInfo.sin_family = this->af;

	if (server_addr != "") {
		addrInfo.sin_addr.s_addr = ::inet_addr(server_addr.c_str());
		if (addrInfo.sin_addr.s_addr == INADDR_NONE) {
			LPHOSTENT lpHostEntry = ::gethostbyname(server_addr.c_str());
			if (lpHostEntry == NULL) {
				throw SocketException(server_addr + " invalid parameter", TRACEBACK);
			}
			else {
				addrInfo.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
			}
		}
	}
	else {
		addrInfo.sin_addr.s_addr = INADDR_ANY;   }
	
	addrInfo.sin_port = ::htons(cPort);        // Use port from command line

	int status = ::bind(mySocket, (LPSOCKADDR)&addrInfo, sizeof(struct sockaddr));
	if (status < 0) {
		this->socketError(WSA_ERROR, __FUNCTION__);
		return status;
	}

	status = ::listen(mySocket, queue_size);
	if (status < 0) { // Number of connection request queue
		this->socketError(WSA_ERROR, __FUNCTION__);
		return status;
	}
	
	return status;
}

Socket ServerSocket::accept()
{
	//
// Wait for an incoming request
//
	auto listenSocket = Socket(::accept(mySocket, 0, 0));

	return listenSocket;
}

