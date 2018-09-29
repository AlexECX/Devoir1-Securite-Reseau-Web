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
void scriptedConvo(SimpleSocket client, string& transit);

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
	/*string transit = "Hello form ";
	transit = encrypt(transit, "1234");
	cout << endl << transit;
	transit = decrypt(transit, "1234");
	cout << endl << transit;
*/

	try
	{
		string transit = "";
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
		
		scriptedConvo(client, transit);
	}
	catch (const ConnectionException e)
	{
		cout << endl << e.what();
	}
	

}

void scriptedConvo(SimpleSocket client, string& transit) {
	string bob_key = "1234";
	string message;

	cout << "\nwaiting for Agnesse";
	client.recvMessage(transit);
	cout << "\nreceived \"" << transit << "\"";
	transit = "Hello from Bob";
	cout << "\nsending \"" << transit << "\" to Agnesse";
	client.sendMessage(transit);


	/**
	cout << "\nwaiting for Agnesse";
	client.recvMessage(transit);
	message = extractMsg(transit);
	cout << "\nreceived \"" << message << "\"";
	cout << "\nreceived \"" << decrypt(message, bob_key) << "\"";

	bool mac_result = verifyMAC(transit, bob_key);
	cout << "\nthe verifyMAC result is: " << (mac_result == true ? "True" : "False");

	if (mac_result == true) {
		transit = "Hello from Bob";
		cout << "\nsending \"" << transit << "\" to Agnesse";
		transit = encrypt(transit, bob_key);
		cout << "\nsending \"" << transit << "\" to Agnesse";
		
		string mac = getMac(transit, bob_key);
		cout << "\n The MAC is " << mac;
		client.sendMessage(transit + mac);
	}
	else
	{
		message = "MAC error from Bob";
		cout << "\nsending \"" << message << "\"";
		transit = encrypt(message, bob_key);
		client.sendMessage(transit + getMac(message, bob_key));
	}
	/**/
}
