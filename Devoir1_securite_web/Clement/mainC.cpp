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

	try
	{
		SimpleServerSocket server = SimpleServerSocket(nPort);
		SimpleSocket clientA = server.acceptSocket();
		SimpleSocket clientB = server.acceptSocket();

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
	string agnesse_key = "gadgettegdag";

	//R�ception de Agnesse
	clientA.recvMessage(message);
	cout << endl << endl;
	cout << "Crypted message received from Agnesse for Bob: \n" << endl;
	cout << "\"" << message << "\"" << endl << endl;

	if (verifyMAC(message, agnesse_key)) {
		//D�sencryption d'Agnesse
		message = decrypt(extractMsg(message), agnesse_key);
		cout << "Clear message received from Agnesse for Bob: \n" << endl;
		cout << "\"" << message << "\"" << endl << endl;

		//Encryption pour Bob
		message = encrypt(message, bob_key);
		cout << "Message a etre transmit (crypt) a Bob de Agnesse: \n\n";
		cout << "\"" << message << "\"" << endl << endl;

		//Envoie � Bob
		clientB.sendMessage(message + generateMac(message, bob_key));
	}
	else {
		cout << endl << "Agnesse MAC incorrect";
		//Envoie erreurs
		message = encrypt("MAC error from Clement", agnesse_key);
		clientA.sendMessage(message + generateMac(message, agnesse_key));
		message = encrypt("Agnesse MAC incorrect", bob_key);
		clientB.sendMessage(message + generateMac(message, bob_key));
	}


	//R�ception de Bob
	clientB.recvMessage(message);
	cout << "Crypted message received from Bob for Agnesse: \n" << endl;
	cout << "\"" << message << "\"" << endl << endl;

	if (verifyMAC(message, bob_key)) {
		//D�sencryption de Bob
		message = decrypt(extractMsg(message), bob_key);
		cout << "Clear message received from Bob for Agnesse: \n" << endl;
		cout << "\"" << message << "\"" << endl << endl;

		//Encryption pour Agnesse
		message = encrypt(message, agnesse_key);
		cout << "Message a etre transmit (crypt) a Agnesse de Bob: \n\n";
		cout << "\"" << message << "\"" << endl << endl;

		//Envoie � Agnesse
		clientA.sendMessage(message + generateMac(message, agnesse_key));
	}
	else {
		cout << endl << "Agnesse MAC incorrect";
		//Envoie erreurs
		message = encrypt("MAC error from Clement", bob_key);
		clientB.sendMessage(message + generateMac(message, bob_key));
		message = encrypt("Bob MAC incorrect", agnesse_key);
		clientA.sendMessage(message + generateMac(message, agnesse_key));
	}

}