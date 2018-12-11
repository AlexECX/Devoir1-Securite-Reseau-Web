#include <iostream>
#include <string>
#include <windows.h>
#include <thread>
#include <regex>
#include "../Connection/SocketException.h"
#include "../Connection/Utils.h"
#include "../Connection/ServerSocket.h"
#include "../Encryption/encryption.h"

using namespace std;

#define HARD_CODED
#define PORT 2030

void runServer(unsigned nPort);
void scriptedConvoTest(Socket clientA, Socket clientB);
string getFileName(const string& s);

int main(int argc, char **argv)
{
	unsigned nPort;

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

void runServer(unsigned nPort)
{
	string message = "";
	string mac_key_B = "mac_key_B";
	string mac_key_A = "mac_key_A";
	
	try
	{
		ServerSocket server = ServerSocket(nPort);
		if (!server.valid_socket()) 
			throw SocketException(server.getSocketErr());
		Socket clientA;
		Socket clientB;

		for (size_t i = 0; i < 2; i++)
		{
			Socket client = server.acceptSocket();
			if (!client.valid_socket())
				throw SocketException(client.getSocketErr(), TRACEBACK);
			cout << "\n\n\nReception en cours..." << endl;
			client.recvMsg(message);
			cout << "Message recu.\n" << endl;
			if (authenticate(message, "is Agnesse", mac_key_A)) {
				cout << "\n\n\nAgnesse est maintenant connectee." << endl;
				clientA = client;
				message = "is Clement";
				cout << "Envoie d'un message a Agnesse...\n\n" << endl;
				clientA.sendMsg_noExcept(message + generateMac(message, mac_key_A));
				cout << "\n\n\nMessage envoye.\n" << endl;
			}
			else if (authenticate(message, "is Bernard", mac_key_B)) {
				cout << "\n\n\nBernard est maintenant connectee." << endl;
				clientB = client;
				message = "is Clement";
				cout << "Envoie d'un message a Bernard...\n\n" << endl;
				clientB.sendMsg_noExcept(message + generateMac(message, mac_key_B));
				cout << "\n\n\nMessage envoye." << endl;

			}
			else {
				cout << "\nUnknown connection";
				return;
			}
		}
		
		scriptedConvoTest(clientA, clientB);
		
	}
	catch (const SocketException& e)
	{
		cout << endl << e.what();
	}	
}

void scriptedConvoTest(Socket clientA, Socket clientB) {
	string message = "";

	string bernard_key = "bobinoonibob";
	string mac_key_B = "mac_key_B";
	string agnesse_key = "gadgettegdag";
	string mac_key_A = "mac_key_A";

	string session_key = generateKey(8);
	string mac_key = generateKey(32);

	//Clément envoie une question à Agnesse
	cout << "Envoie d'une question a Agnesse...\n\n" << endl;
	message = "Clement demande:\nA qui desires-tu parler Agnesse?";
	message = encrypt(message, agnesse_key);
	clientA.sendMsg_noExcept(message + generateMac(message, mac_key_A));
	cout << "\n\n\nMessage envoye." << endl;

	//Clément reçoit la réponse d'Agnesse
	cout << "Attente de la reponse...\n\n" << endl;
	clientA.recvMsg(message);
	cout << "Message recu." << endl;
	cout << "Verification et dechiffrement..." << endl;
	if (!verifyMAC(message, mac_key_A)) {
		cout << "\nsender is not Agnesse";
		return;
	}
	message = decrypt(extractMsg(message), agnesse_key);
	cout << "\nAgnesse desire parler avec: " << endl;
	cout << message << endl;

	//Distribution d'une clé de session, MAC key de session et informations reseau a Agnesse et Bernard si validation ok
	if (tolower_str(message).compare("bernard") == 0)
	{
		//Envoie des messages d'information
		cout << "\nValidation complete. Envoie des messages a Bernard et Agnesse..." << endl;
		message = "Clement vous confirme que " + message + " est de confiance.\nLa conversation pourra debuter.\nVoici les informations pertinentes pour la communication.";
		cout << "Envoie du message a Agnesse...\n\n" << endl;
		message = encrypt(message, agnesse_key);
		clientA.sendMsg_noExcept(message + generateMac(message, mac_key_A));
		cout << "\n\n\nMessage envoye." << endl;
		message = "Agnesse desire communiquer avec vous. Agnesse est de confiance.\nVoici les information de connection...";
		cout << "Envoie du message a Bernard...\n\n" << endl;
		message = encrypt(message, bernard_key);
		clientB.sendMsg_noExcept(message + generateMac(message, mac_key_B));
		cout << "\n\n\nMessage envoye." << endl;

		//Envoie des clés de session Agnesse
		cout << "Envoie de la cle de session a Agnesse...\n\n" << endl;
		message = encrypt(session_key, agnesse_key);
		clientA.sendMsg_noExcept(message + generateMac(message, mac_key_A));
		cout << "\n\n\nCle de session envoye a Agnesse." << endl;
		cout << "Envoie de la cle de session MAC a Agnesse...\n\n" << endl;
		message = encrypt(mac_key, agnesse_key);
		clientA.sendMsg_noExcept(message + generateMac(message, mac_key_A));
		cout << "\n\n\nCle de session MAC envoye a Agnesse." << endl;
		cout << "Envoie des informations reseau a Agnesse...\n\n" << endl;
		connectionInfo info = clientB.getIPinfo();
		message = encrypt(info.IP+":"+"2031", agnesse_key);
		clientA.sendMsg_noExcept(message + generateMac(message, mac_key_A));
		cout << "\n\n\nInformation reseau envoye a Agnesse." << endl;

		//Envoie des clés de session Bernard
		cout << "Envoie de la cle de session a Bernard...\n\n" << endl;
		message = encrypt(session_key, bernard_key);
		clientB.sendMsg_noExcept(message + generateMac(message, mac_key_B));
		cout << "\n\n\nCle de session envoye a Bernard." << endl;
		cout << "Envoie de la cle de session MAC a Bernard...\n\n" << endl;
		message = encrypt(mac_key, bernard_key);
		clientB.sendMsg_noExcept(message + generateMac(message, mac_key_B));
		cout << "\n\n\nCle de session MAC envoye a Bernard." << endl;
		cout << "Envoie des informations reseau a Bernard...\n\n" << endl;
		info = clientA.getIPinfo();
		message = encrypt(info.IP + ":" + "2031", bernard_key);
		clientB.sendMsg_noExcept(message + generateMac(message, mac_key_B));
		cout << "\n\n\nInformation reseau envoye a Bernard." << endl;
	}
	else
	{
		cout << "\nDestinataire inconnu, veillez spécifier un destinataire connu..." << endl;
	}

	//Fermeture du tiers de confiance...
	cout << "Traitement de la demande complete...Fermeture du service.\n" << endl;	
}


string getFileName(const string& s) {
	char sep = '/';

#ifdef _WIN32
	sep = '\\';
#endif

	size_t i = s.rfind(sep, s.length());
	if (i != string::npos) {
		return(s.substr(i + 1, s.length() - i));
	}
	return(s);
}