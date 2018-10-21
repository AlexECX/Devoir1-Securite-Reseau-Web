#include <iostream>
#include <string>
#include <windows.h>
#include <thread>
#include "../Connection/ConnectionException.h"
#include "../Connection/Utils.h"
#include "../Connection/SimpleServerSocket.h"
#include "../Encryption/encryption.h"

using namespace std;

#define HARD_CODED
#define PORT 2030

void runServer(short nPort);
void scriptedConvoTest(SimpleSocket clientA, SimpleSocket clientB);

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
	system("pause");
	return 0;
}

void runServer(short nPort)
{
	string message = "";
	string mac_key_B = "mac_key_B";
	string mac_key_A = "mac_key_A";
	
	try
	{
		SimpleServerSocket server = SimpleServerSocket(nPort);
		SimpleSocket clientA;
		SimpleSocket clientB;

		for (size_t i = 0; i < 2; i++)
		{
			SimpleSocket client = server.acceptSocket();
			client.recvMessage(message);
			if (authenticate(message, "is Agnesse", mac_key_A)) {
				clientA = client;
				message = "is Clement";
				clientA.sendMessage(message + generateMac(message, mac_key_A));
			}
			else if (authenticate(message, "is Bob", mac_key_B)) {
				clientB = client;
				message = "is Clement";
				clientB.sendMessage(message + generateMac(message, mac_key_B));

			}
			else {
				cout << "\nUnknown connection";
				return;
			}
		}

		scriptedConvoTest(clientA, clientB);
		
	}
	catch (const ConnectionException& e)
	{
		cout << endl << e.what();
	}
	
	
}

void scriptedConvoTest(SimpleSocket clientA, SimpleSocket clientB) {
	string message = "";

	string bob_key = "bobinoonibob";
	string mac_key_B = "mac_key_B";
	string agnesse_key = "gadgettegdag";
	string mac_key_A = "mac_key_A";

	string session_key = generateKey(8);
	string mac_key = generateKey(32);

	//Distribution d'une session et MAC key a Agnesse et Bob
	message = encrypt(session_key, agnesse_key);
	clientA.sendMessage(message + generateMac(message, mac_key_A));
	message = encrypt(mac_key, session_key);
	clientA.sendMessage(message + generateMac(message, mac_key_A));

	message = encrypt(session_key, bob_key);
	clientB.sendMessage(message + generateMac(message, mac_key_B));
	message = encrypt(mac_key, session_key);
	clientB.sendMessage(message + generateMac(message, mac_key_B));

	bob_key = agnesse_key = session_key;
	mac_key_B = mac_key_A = mac_key;

	//R�ception de Agnesse
	clientA.recvMessage(message);
	cout << endl << endl;
	cout << "Crypted message received from Agnesse for Bob: \n" << endl;
	cout << "\"" << message << "\"" << endl << endl;

	if (verifyMAC(message, mac_key_A)) {
		cout << "\nthe verifyMAC result is: True";
		//Envoie � Bob
		clientB.sendMessage(message);
	}
	else {
		cout << endl << "Agnesse MAC incorrect";
		//Envoie erreurs
		message = encrypt("MAC error from Clement", agnesse_key);
		clientA.sendMessage(message + generateMac(message, mac_key_A));
		message = encrypt("Agnesse MAC incorrect", bob_key);
		clientB.sendMessage(message + generateMac(message, mac_key_B));
	}


	//R�ception de Bob
	clientB.recvMessage(message);
	cout << "Crypted message received from Bob for Agnesse: \n" << endl;
	cout << "\"" << message << "\"" << endl << endl;

	if (verifyMAC(message, mac_key_B)) {
		cout << "\nthe verifyMAC result is: True";
		//Envoie � Agnesse
		clientA.sendMessage(message);
	}
	else {
		cout << endl << "Agnesse MAC incorrect";
		//Envoie erreurs
		message = encrypt("MAC error from Clement", bob_key);
		clientB.sendMessage(message + generateMac(message, mac_key_B));
		message = encrypt("Bob MAC incorrect", agnesse_key);
		clientA.sendMessage(message + generateMac(message, mac_key_A));
	}

}