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
	string transit = "";
	string message = "";
	string cryptogram = "";
	try
	{
		SimpleServerSocket server = SimpleServerSocket(nPort);
		SimpleSocket s1 = server.acceptSocket();
		SimpleSocket s2 = server.acceptSocket();

		string bob_key = "1234";
		string agnesse_key = "abcd";

		s1.recvMessage(transit);
		s2.sendMessage(transit);

		s2.recvMessage(transit);
		s1.sendMessage(transit);

		/**
		s1.recvMessage(transit);
		if (verifyMAC(transit, agnesse_key)) {
			message = extractMsg(transit);
			cryptogram = encrypt(decrypt(message, agnesse_key), bob_key);
			s2.sendMessage(cryptogram + getMac(transit, bob_key));
		}
		else
		{
			cryptogram = encrypt("MAC error from Clement", agnesse_key);
			s1.sendMessage(cryptogram + getMac(transit, agnesse_key));
			cryptogram = encrypt("Agnesse MAC was refused", bob_key);
			s2.sendMessage(cryptogram + getMac(transit, bob_key));
		}

		s2.recvMessage(transit);
		if (verifyMAC(transit, bob_key)) {
			transit = extractMsg(transit);
			cryptogram = encrypt(decrypt(transit, bob_key), agnesse_key);
			s1.sendMessage(cryptogram + getMac(transit, agnesse_key));
		}
		else
		{
			transit = "Bob MAC was refused";
			cryptogram = encrypt(transit, bob_key);
			s1.sendMessage(cryptogram + getMac(transit, agnesse_key));
		}		
		/**/
		
	}
	catch (const ConnectionException e)
	{
		cout << endl << e.what();
	}
	
	
}
