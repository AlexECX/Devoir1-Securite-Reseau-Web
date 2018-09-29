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
	string cryptogram = "";
	try
	{
		SimpleServerSocket server = SimpleServerSocket(nPort);
		SimpleSocket s1 = server.acceptSocket();
		SimpleSocket s2 = server.acceptSocket();

		string bob_key = "1234";
		string agnesse_key = "abcd";

		s1.recvMessage(msg);
		if (verifyMAC(msg, agnesse_key)) {
			msg = extractMsg(msg);
			cryptogram = encrypt(decrypt(msg, agnesse_key), bob_key);
			s2.sendMessage(cryptogram + getMac(msg, bob_key));
		}
		else
		{
			cryptogram = encrypt("MAC error from Clement", agnesse_key);
			s1.sendMessage(cryptogram + getMac(msg, agnesse_key));
			cryptogram = encrypt("Agnesse MAC was refused", bob_key);
			s2.sendMessage(cryptogram + getMac(msg, bob_key));
		}

		s2.recvMessage(msg);
		if (verifyMAC(msg, bob_key)) {
			msg = extractMsg(msg);
			cryptogram = encrypt(decrypt(msg, bob_key), agnesse_key);
			s1.sendMessage(cryptogram + getMac(msg, agnesse_key));
		}
		else
		{
			msg = "Bob MAC was refused";
			cryptogram = encrypt(msg, bob_key);
			s1.sendMessage(cryptogram + getMac(msg, agnesse_key));
		}		
		
	}
	catch (const ConnectionException e)
	{
		cout << endl << e.what();
	}
	
	
}
