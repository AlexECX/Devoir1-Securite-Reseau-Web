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
void scriptedConvo(SimpleSocket client, string& msg);

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

			scriptedConvo(client, msg);

		}

		}
	catch (const ConnectionException e)
	{
		cout << endl << e.what();
	}
	

}

void scriptedConvo(SimpleSocket client, string& msg) {
	string agnesse_key = "abcd";

	msg = "Hello from Agnesse";
	cout << "\nsending \"" << msg << "\" to Bob";
	string mac = simpleHMCA(msg, agnesse_key);
	cout << "\n The MAC is " << mac;

	client.sendMessage(msg + mac);
	cout << "\nwaiting for Bob";
	client.recvMessage(msg);

	cout << "\nreceived \"" << extractMsg(msg) << "\"";
	cout << "\nthe verifyMAC result is: " << ((verifyMAC(msg, agnesse_key) == true) ? "True" : "False");

}
