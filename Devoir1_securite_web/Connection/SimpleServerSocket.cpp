#include "SimpleServerSocket.h"
#include "ConnectionException.h"
#include <string>
#include <iostream>


using namespace std;

SimpleServerSocket::SimpleServerSocket()
{
}

SimpleServerSocket::SimpleServerSocket(short cPort)
{
	bindSocket(nullptr, cPort);
	
	
}

SimpleServerSocket::SimpleServerSocket(const char * szServer, short cPort)
{
	bindSocket(szServer, cPort);
}


SimpleServerSocket::~SimpleServerSocket()
{
}

void SimpleServerSocket::bindSocket(const char * szServer, short cPort)
{
	saServer.sin_family = AF_INET;
	if (szServer != nullptr) {
		saServer.sin_addr.s_addr = inet_addr(szServer);
	}
	else
	{
		saServer.sin_addr.s_addr = INADDR_ANY;   // Indicates that connections can come from any local interface (IP address)
											 // (by the way,  INADDR_ANY is actually "0x00000000")
											 // ************ if you wish to specify a specific local interface (IP address) :
											 //              =>     saServer.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	}
	
	saServer.sin_port = htons(cPort);        // Use port from command line

											 //
											 // bind the name to the socket
											 //
	int nRet;
	nRet = bind(mySocket,                      // Socket
		(LPSOCKADDR)&saServer,       // Our address
		sizeof(struct sockaddr));    // Size of address structure
	if (nRet == SOCKET_ERROR) {
		throw ConnectionException(WSA_ERROR, TRACEBACK);
	}

	nRet = listen(mySocket,                          // Bound socket
		SOMAXCONN);                            // Number of connection request queue
	if (nRet == SOCKET_ERROR) {
		throw ConnectionException(WSA_ERROR, TRACEBACK);
	}
}

SimpleSocket SimpleServerSocket::acceptSocket()
{
	//
// Wait for an incoming request
//

	std::cout << "\nBlocking at accept()";
	SOCKET listenSocket = accept(mySocket,            // Listening socket
		NULL,                            // Optional client address
		NULL);
	if (listenSocket == INVALID_SOCKET) {
		throw ConnectionException(WSA_ERROR, TRACEBACK);
	}
	else
		std::cout << "\nConnected";

	return SimpleSocket(listenSocket);
}

