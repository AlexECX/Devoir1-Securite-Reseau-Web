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
	string message = "HelloFrom\nTheOtherSideHelloFrom\n����TheOther\n  .Side$";
	cout << endl << message;
	message = encrypt(message);
	cout << endl <<  message;

	message = encrypt(message);
	cout << endl << message;


	
	


	/*try
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
	*/

}

void scriptedConvo(SimpleSocket client, string& msg) {
	string bob_key = "1234";


	cout << "\nwaiting for Agnesse";
	client.recvMessage(msg);
	cout << "\nreceived \"" << extractMsg(msg) << "\"";

	bool mac_result = verifyMAC(msg, bob_key);
	cout << "\nthe verifyMAC result is: " << (mac_result == true ? "True" : "False");

	if (mac_result == true) {
		cout << "\nsending \"Hello\" to Agnesse";
		msg = "Hello from Bob";
		string mac = simpleHMCA(msg, bob_key);
		cout << "\n The MAC is " << mac;
		client.sendMessage(msg + mac);
	}
	else
	{
		cout << "\nsending MAC error";
		msg = "MAC error from Bob";
		client.sendMessage(msg + simpleHMCA(msg, bob_key));
	}
}
