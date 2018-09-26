#include <winsock.h>
#include <iostream>
#include <string>
#include <windows.h>
#include <thread>
#include "ConnectionException.h"
#include "Utils.h"
#include "SimpleSocket.h"

using namespace std;

//#define HARD_CODED
//#define HOST "P2-4020-21" //Can be Name or IP address
//#define PORT 2030

void runClient(short nPort, char* host);

int main(int argc, char **argv)
{
	short nPort;
	char* host;

	//
	// Check for the host and port arguments
	//
#ifdef HARD_CODED
	nPort = PORT;
	host = HOST;
#else
	if (argc != 3) {
		cout << "\nSyntax: ServerName PortNumber"; //test
		cout << endl;
		return 0;
	}
	nPort = atoi(argv[2]);
	host = argv[1];

#endif // HARD_CODED

	initWSA();

	runClient(nPort, host);

	// Release WinSock
	//
	WSACleanup();
	
	return 0;

}

void runClient(short nPort, char * host)
{
	try
	{
		string msg = "";
		bool connected = false;
		SimpleSocket client = SimpleSocket();

		while (!connected) {
			try
			{
				cout << "\nconnecting to " + string(host) + " on port " << nPort;
				client.connectSocket(host, nPort);
				connected = true;
			}
			catch (const ConnectionException e)
			{
				cout << endl << e.what();
				this_thread::sleep_for(5s);
			}

		}
		cout << "\nwaiting for Agnesse on port " << to_string(nPort);
		client.recvMessage(msg);
		cout << "\nreceived \"" << msg << "\"";
		cout << "\nsending \"Hello\" to Agnesse";
		client.sendMessage("Hello from Bob");
	}
	catch (const ConnectionException e)
	{
		cout << endl << e.what();
	}
	

}
