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
	string crypt_key_B = "bobinoonibob";
	string mac_key_B = "mac_key_B";
	string message = "";
	string mac = "";

	//S'identifie aupres de Clement
	message = "is Bob";
	client.sendMessage(message + generateMac(message, mac_key_B));
	client.recvMessage(message);
	if (!authenticate(message, "is Clement", mac_key_B))
	{
		cout << "\nsender is not Clement";
		return;
	}

	//Recois clé de session et MAC
	client.recvMessage(message);
	if (!verifyMAC(message, mac_key_B)) {
		cout << "\nsender is not Clement";
		return;
	}
	crypt_key_B = decrypt(extractMsg(message), crypt_key_B);

	client.recvMessage(message);
	if (!verifyMAC(message, mac_key_B)) {
		cout << "\nsender is not Clement";
		return;
	}
	mac_key_B = decrypt(extractMsg(message), crypt_key_B);

	//R�ception
	cout << "\nwaiting for Agnesse";
	client.recvMessage(message);
	cout << "\n\nMessage recu (crypted)de Clement provenance Agnesse:\n";
	cout << "\"" << message << "\"" << endl << endl;

	//D�sencryption
	cout << "\nMessage clair recu de Clement (provenance Agnesse)\n\n" << endl;
	cout << "\"" << decrypt(extractMsg(message), crypt_key_B) << "\"" << endl;

	//V�rif MAC
	bool verify = verifyMAC(message, mac_key_B);
	cout << "\nthe verifyMAC result is: "
		<< (verify == true ? "True" : "False");

	if (verify) {
		//Message de Bob pour Agnesse
		cout << "\n\nQuel est le message de Bob?" << endl;
		getline(cin, message);
		cout << endl << endl;
		cout << "Message a etre transmit (clair) a Clement pour Agnesse: \n";
		cout << "\"" << message << "\"" << endl << endl;

		//Encryption
		message = encrypt(message, crypt_key_B);

		//Envoie du message
		cout << endl << endl;
		cout << "\nMessage a etre transmit (crypt) a Clement pour Agnesse: \n\n";
		cout << "\"" << message << "\"" << endl << endl;
		client.sendMessage(message + generateMac(message, mac_key_B));
	}
	else {
		cout << endl << "Clement MAC incorrect";
		//Envoie erreurs
		message = encrypt("Clement MAC incorrect", crypt_key_B);
		client.sendMessage(message + generateMac(message, mac_key_B));
	}

	

}