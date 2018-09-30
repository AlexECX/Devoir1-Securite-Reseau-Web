#include <winsock.h>
#include <iostream>
#include <string>
#include <windows.h>
#include <thread>
#include "ConnectionException.h"
#include "Utils.h"
#include "SimpleSocket.h"
#include "encryption.h"

using namespace std;

//#define HARD_CODED
#define HOST "127.0.0.1" //Can be Name or IP address
#define PORT 8812

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
		cout << "\nSyntax: ServerName PortNumber";
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
	string agnesse_key = "abcd";
	string message = "";
	string mac = "";

	/**/
	message = "Hello from Agnesse Hello from Agnesse Hello from Agnesse";
	cout << "\nsending \"" << message << "\" to Bob";
	message = encrypt(message, agnesse_key);
	cout << "\nsending \"" << message << "\" to Bob";
	mac = generateMac(message, agnesse_key);
	cout << "\nthe MAC is " << mac;

	client.sendMessage(message + mac);
	cout << "\nwaiting for Bob";

	client.recvMessage(transit);
	cout << "\nreceived \"" << transit << "\"";
	cout << "\nthe verifyMAC result is: "
		<< (verifyMAC(transit, agnesse_key) == true ? "True" : "False");
	message = decrypt(extractMsg(transit), agnesse_key);
	cout << "\nreceived \"" << message << "\"";

	/**/
}
