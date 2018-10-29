#include "SimpleSocket.h"
#include "ConnectionException.h"
#include <string>
#include <iostream>

using namespace std;


SimpleSocket::SimpleSocket()
{
}

SimpleSocket::SimpleSocket(SOCKET socket): Connection(socket)
{
}

SimpleSocket::SimpleSocket(const char * szServer, short cPort)
{
	connectSocket(szServer, cPort);
}


SimpleSocket::~SimpleSocket()
{
}

void SimpleSocket::connectSocket(const char * szServer, short cPort)
{
	LPHOSTENT lpHostEntry;

	addrInfo.sin_family = AF_INET;
	addrInfo.sin_port = htons(cPort);

	addrInfo.sin_addr.s_addr = inet_addr(szServer);
	if (addrInfo.sin_addr.s_addr == INADDR_NONE) {
		cout << endl << szServer << " is not IPv4, hostname?";
		lpHostEntry = gethostbyname(szServer);
		if (lpHostEntry == NULL) {
			throw ConnectionException(string(szServer)+" invalid parameter", TRACEBACK);
		}
		else {
			addrInfo.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
		}
	}

	int nRet = connect(mySocket,                      // Socket
		(LPSOCKADDR)&addrInfo,       // Our address
		sizeof(struct sockaddr));    // Size of address structure
	if (nRet == SOCKET_ERROR || nRet == INVALID_SOCKET) {
		throw ConnectionException(WSA_ERROR, TRACEBACK);
	}
}
