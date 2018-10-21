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

	saServer.sin_family = AF_INET;
	saServer.sin_port = htons(cPort);

	saServer.sin_addr.s_addr = inet_addr(szServer);
	if (saServer.sin_addr.s_addr == INADDR_NONE) {
		cout << endl << szServer << " is not IPv4, hostname?";
		lpHostEntry = gethostbyname(szServer);
		if (lpHostEntry == NULL) {
			throw ConnectionException(string(szServer)+" invalid parameter", TRACEBACK);
		}
		else {
			saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
		}
	}

	int nRet = connect(mySocket,                      // Socket
		(LPSOCKADDR)&saServer,       // Our address
		sizeof(struct sockaddr));    // Size of address structure
	if (nRet == SOCKET_ERROR || nRet == INVALID_SOCKET) {
		throw ConnectionException(WSA_ERROR, TRACEBACK);
	}
}
