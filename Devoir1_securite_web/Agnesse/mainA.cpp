#include <iostream>
#include <string>
#include <windows.h>
#include <thread>
#include "../Connection/ConnectionException.h"
#include "../Connection/Utils.h"
#include "../Connection/SimpleSocket.h"
#include "../Encryption/encryption.h"

using namespace std;

#define HARD_CODED
#define HOST "127.0.0.1" //Can be Name or IP address
#define PORT 2030

void runClient(short nPort, const char* host);
void scriptedConvoTest(SimpleSocket client);

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
	system("pause");
	return 0;

}

void runClient(short nPort, const char * host)
{

	try
	{
		bool connected = false;
		SimpleSocket client = SimpleSocket();

		while (!connected) {
			try
			{
				cout << "\nconnecting to " + string(host) + " on port " << nPort;
				client.connectSocket(host, nPort);
				connected = true;
			}
			catch (const ConnectionException& e)
			{
				cout << endl << e.what();
				this_thread::sleep_for(5s);
			}
		}

		scriptedConvoTest(client);

		}
	catch (const ConnectionException& e)
	{
		cout << endl << e.what();
	}
	

}

void scriptedConvoTest(SimpleSocket client) {
	string transit = "";
	string agnesse_key = "gadgettegdag";
	string message = "";
	string mac = "";

	//Input
	cout << "\nQuel est le message d'Agnesse?" << endl;
	getline(cin, message);
	cout << endl;
	cout << "Message a etre transmit (clair) a Clement pour Bob: \n";
	cout << "\"" << message << "\"" << endl << endl;

	//Encryption
	message = encrypt(message, agnesse_key);
	cout << endl;
	cout << "\nMessage a etre transmit (crypt) a Clement pour Bob: \n\n";
	cout << "\"" << message << "\"" << endl << endl;

	//MAC
	mac = generateMac(message, agnesse_key);
	cout << endl;
	cout << "\nCode MAC\n\n";
	cout << mac << endl << endl;

	//Envoie du message
	client.sendMessage(message + mac);
	cout << "\nwaiting for Bob" << endl;

	//R�ception
	client.recvMessage(message);
	cout << "Message crypte recu de Clement (provenance Bob)\n" << endl;
	cout << "\"" << message << "\"" << endl << endl;
	cout << "\nthe verifyMAC result is: "
		<< (verifyMAC(message, agnesse_key) == true ? "True" : "False")
		<< endl << endl;

	//D�sencryption
	message = decrypt(extractMsg(message), agnesse_key);
	cout << "Message clair recu de Clement (provenance Bob)\n" << endl;
	cout << "\"" << message << "\"" << endl << endl;

}