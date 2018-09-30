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
	string message = "";
	string mac = "";

	/**/
	cout << "\nwaiting for Agnesse";
	client.recvMessage(transit);
	cout << "\nreceived \"\n";
	cout << transit << "\"";
	bool mac_result = verifyMAC(transit, bob_key);
	cout << "\nthe verifyMAC result is: "
		<< (mac_result == true ? "True" : "False");

	message = decrypt(extractMsg(transit), bob_key);
	cout << "\nreceived \"" << message << "\"";

	if (mac_result == true) {
		message = "Hello from Bob";
		cout << "\nsending \"" << message << "\" to Agnesse";
		message = encrypt(message, bob_key);
		cout << "\nsending \"" << message << "\" to Agnesse";
		
		mac = generateMac(message, bob_key);
		cout << "\nthe MAC is " << mac;
		client.sendMessage(message + mac);
	}
	else
	{
		message = "MAC error from Bob";
		cout << "\nsending \"" << message << "\"";
		message = encrypt(message, bob_key);
		client.sendMessage(message + generateMac(message, bob_key));
	}
	/**/
}
