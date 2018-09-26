#include <winsock.h>
#include <iostream>
#include <string>
#include <thread>
#include <windows.h>
#include "ConnectionException.h"
#include "SimpleServerSocket.h"
#include "Utils.h"

using namespace std;

//#define HARD_CODED
//#define PORT 2030

void runServer(short nPort);

int main(int argc, char **argv)
{
	short nPort;

	//
	// Check for port argument
	//
#ifdef HARD_CODED
	nPort = PORT;
#else
	if (argc != 2) {
		cout << "\nSyntax: PortNumber"; 
		cout << endl;
		return 0;
	}
	nPort = atoi(argv[1]);

#endif // HARD_CODED

	initWSA();

	runServer(nPort);

	// Release WinSock
	//
	WSACleanup();
	return 0;
}

void runServer(short nPort)
{
	string msg = "";
	string msg_copy = "";
	try
	{
		SimpleServerSocket server = SimpleServerSocket(nPort);
		SimpleSocket s1 = server.acceptSocket();
		SimpleSocket s2 = server.acceptSocket();

		s1.recvMessage(msg);
		s2.sendMessage(msg);

		s2.recvMessage(msg);
		s1.sendMessage(msg);
	}
	catch (const ConnectionException e)
	{
		cout << endl << e.what();
	}
	
	
}
