#include <winsock.h>
#include <iostream>
#include <string>
#include <windows.h>
#include <thread>
#include "ConnectionException.h"
#include "Utils.h"
#include "SimpleSocket.h"
#include <map>
#include <bitset>

using namespace std;

//#define HARD_CODED
//#define HOST "P2-4020-21" //Can be Name or IP address
//#define PORT 2030

void runClient(short nPort, const char* host);
void scriptedConvo(SimpleSocket client, string& msg);

int main(int argc, char **argv)
{
	short nPort;
	const char* host;

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

void runClient(short nPort, const char * host)
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
		
		scriptedConvo(client, msg);
	}
	catch (const ConnectionException e)
	{
		cout << endl << e.what();
	}
	

}

void scriptedConvo(SimpleSocket client, string& msg) {
	string bob_key = "1234";


	cout << "\nwaiting for Agnesse";
	client.recvMessage(msg);
	msg = extractMsg(msg);
	cout << "\nreceived \"" << msg << "\"";
	cout << "\nreceived \"" << decrypt(msg, bob_key) << "\"";

	bool mac_result = verifyMAC(msg, bob_key);
	cout << "\nthe verifyMAC result is: " << (mac_result == true ? "True" : "False");

	if (mac_result == true) {
		msg = "Hello from Bob";
		cout << "\nsending \"" << msg << "\" to Agnesse";
		msg = encrypt(msg, bob_key);
		cout << "\nsending \"" << msg << "\" to Agnesse";
		
		string mac = getMac(msg, bob_key);
		cout << "\n The MAC is " << mac;
		client.sendMessage(msg + mac);
	}
	else
	{
		msg = "MAC error from Bob";
		cout << "\nsending \"" << msg << "\"";
		msg = encrypt(msg, bob_key);
		client.sendMessage(msg + getMac(msg, bob_key));
	}
}
