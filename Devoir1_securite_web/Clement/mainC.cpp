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

	//Clément envoie une question à Agnesse
	message = "Clement demande:\nA qui desires-tu parler Agnesse?";
	message = encrypt(message, agnesse_key);
	clientA.sendMessage(message + generateMac(message, mac_key_A));

	//Clément reçoit la réponse d'Agnesse
	clientA.recvMessage(message);
	if (!verifyMAC(message, mac_key_A)) {
		cout << "\nsender is not Clement";
		return;
	}
	message = decrypt(extractMsg(message), agnesse_key);
	cout << "\n\nAgnesse desire parler avec: " << endl;
	cout << message << endl;

	//Distribution d'une clé de session, MAC key de session et informations reseau a Agnesse et Bob si validation ok
	if (message == "Bob" || message == "bob" || message == "BOB" || message == "BOb" || message == "BoB" || message == "bOb")
	{
		cout << "\nValidation complete. Envoie des messages a Bob et Agnesse...\n" << endl;
		message = "Clement vous confirme que " + message + " est de confiance.\nLa conversation pourra debuter.\nVoici les informations pertinentes pour la communication.";
		message = encrypt(message, agnesse_key);
		clientA.sendMessage(message + generateMac(message, mac_key_A));
		message = "Agnesse desire communiquer avec vous. Agnesse est de confiance.\nVoici les information de connection...";
		message = encrypt(message, bob_key);
		clientB.sendMessage(message + generateMac(message, mac_key_B));

		message = encrypt(session_key, agnesse_key);
		clientA.sendMessage(message + generateMac(message, mac_key_A));
		message = encrypt(mac_key, agnesse_key);
		clientA.sendMessage(message + generateMac(message, mac_key_A));
		message = encrypt("127.0.0.1:2031", agnesse_key);
		clientA.sendMessage(message + generateMac(message, mac_key_A));

		message = encrypt(session_key, bob_key);
		clientB.sendMessage(message + generateMac(message, mac_key_B));
		message = encrypt(mac_key, bob_key);
		clientB.sendMessage(message + generateMac(message, mac_key_B));
		message = encrypt("127.0.0.1:2031", bob_key);
		clientB.sendMessage(message + generateMac(message, mac_key_B));
	}
	else
	{
		cout << "\nDestinataire inconnu, veillez spécifier un destinataire connu..." << endl;
	}

	cout << "Traitement de la demande complete...Fermeture du service.\n" << endl;	
}