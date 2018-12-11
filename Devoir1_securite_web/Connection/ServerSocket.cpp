#include "ServerSocket.h"
#include "SocketException.h"
#include <string>


using namespace std;

ServerSocket::ServerSocket()
{
}

ServerSocket::ServerSocket(unsigned cPort)
{
	bindSocket("", cPort);
}

ServerSocket::ServerSocket(const string& server_addr, unsigned cPort)
{
	bindSocket(server_addr, cPort);
}


ServerSocket::~ServerSocket()
{
}

bool ServerSocket::bindSocket(const string& server_addr, unsigned cPort, unsigned queue_size)
{
	addrInfo.sin_family = this->af;
	if (server_addr != "") {
		addrInfo.sin_addr.s_addr = ::inet_addr(server_addr.c_str());
	}
	else {
		addrInfo.sin_addr.s_addr = INADDR_ANY;   // Indicates that connections can come from any local interface (IP address)
											 // (by the way,  INADDR_ANY is actually "0x00000000")
											 // ************ if you wish to specify a specific local interface (IP address) :
											 //              =>     addrInfo.sin_addr.s_addr = ::inet_addr( "127.0.0.1" );
	}
	
	addrInfo.sin_port = ::htons(cPort);        // Use port from command line

											 //
											 // bind the name to the socket
											 //
	if (::bind(mySocket, (LPSOCKADDR)&addrInfo, sizeof(struct sockaddr)) == SOCKET_ERROR) {
		this->socketError(WSA_ERROR, __FUNCTION__);
		return false;
	}

	if (::listen(mySocket, queue_size) == SOCKET_ERROR) { // Number of connection request queue
		this->socketError(WSA_ERROR, __FUNCTION__);
		return false;
	}
	
	return true;
}

Socket ServerSocket::acceptSocket()
{
	//
// Wait for an incoming request
//
	auto listenSocket = Socket(::accept(mySocket, 0, 0));

	return listenSocket;
}

