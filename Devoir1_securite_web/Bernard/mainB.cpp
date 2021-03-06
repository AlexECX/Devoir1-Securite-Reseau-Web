#include <iostream>
#include <string>
#include <windows.h>
#include <thread>
#include "../Connection/SocketException.h"
#include "../Connection/Utils.h"
#include "../Connection/Socket.h"
#include "../Connection/ServerSocket.h"
#include "../Encryption/encryption.h"

using namespace std;

#define HARD_CODED
#define HOST "127.0.0.1" //Can be Name or IP address
#define PORT 2030

void runClient(unsigned nPort, const string& host);
void scriptedConvoTest(Socket client);
void runConversationAgnesseBernard(Socket clientAgnesse, std::string, std::string);

int main(int argc, char **argv)
{
	unsigned nPort;
	string host;

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

	WSA_Utils::initWSA();

	runClient(nPort, host);

	// Release WinSock
	//
	WSACleanup();
		
	system("pause");
	return 0;

}

void runClient(unsigned nPort, const string& host)
{
	try
	{
		Socket client = Socket();
		if (!client.valid_socket())
			throw SocketException(client.getSocketErr(), TRACEBACK);

		while (true) {
			cout << "\nconnecting to " + string(host) + " on port " << nPort;
			if (client.connectSocket(host, nPort)) {
				break;
			}
			else {
				this_thread::sleep_for(2s);
			}
		}
		cout << "\nConnection successful." << endl;
		scriptedConvoTest(client);
	}
	catch (const SocketException& e)
	{
		cout << endl << e.what();
	}
}

void scriptedConvoTest(Socket client) {
	string bernard_key = "bobinoonibob";
	string session_key = "";
	string mac_key = "mac_key_B";
	string session_mac_key = "";
	string message = "";
	string mac = "";
	string agnesse_IP = "";
	string agnesse_port = "";

	//S'identifie aupres de Clement
	cout << "Authentification de Bernard aupres de Clement..." << endl;
	cout << "Envoie d'un message..." << endl;
	message = "is Bernard";
	client.sendMsg(message + generateMac(message, mac_key));
	cout << "\n\n\nMessage envoye a Clement." << endl;
	cout << "Identification de Clement...." << endl;
	client.recvMsg(message);
	cout << "Reponse recu." << endl;
	cout << "Authentification...\n\n" << endl;
	if (!authenticate(message, "is Clement", mac_key))
	{
		cout << "\nsender is not Clement";
		return;
	}

	//Réception d'un message de Clément
	cout << "\n\n\nEn attente de la reponse de Clement..." << endl;
	client.recvMsg(message);
	cout << "Reponse recu." << endl;
	cout << "Verification et dechiffrement...\n\n" << endl;
	if (!verifyMAC(message, mac_key)) {
		cout << "\nsender is not Clement";
		return;
	}
	message = decrypt(extractMsg(message), bernard_key);
	cout << "\n\n\nMessage pour Bernard:" << endl;
	cout << message << endl;

	//Réception de la clé de session et MAC de session et infos réseau de Bernard
	cout << "\nReception de la cle de session..." << endl;
	
	client.recvMsg(message);
	cout << "Message recu." << endl;
	cout << "Verification et dechiffrement...\n\n" << endl;
	if (!verifyMAC(message, mac_key)) {
		cout << "\nsender is not Clement";
		return;
	}
	session_key = decrypt(extractMsg(message), bernard_key);
	cout << "\n\n\nCle de session recu..." << endl;
	cout << "Cle de session: ";
	cout << session_key << endl;

	cout << "\nReception de la cle MAC de session..." << endl;
	client.recvMsg(message);
	cout << "Message recu..." << endl;
	cout << "Verification et dechiffrement...\n\n" << endl;
	if (!verifyMAC(message, mac_key)) {
		cout << "\nsender is not Clement";
		return;
	}
	session_mac_key = decrypt(extractMsg(message), bernard_key);
	cout << "\n\n\nCle de session MAC recu." << endl;
	cout << "Cle MAC de session: ";
	cout << session_mac_key << endl;

	//Réception des informations réseau
	cout << "Reception des informations reseau..." << endl;
	client.recvMsg(message);
	cout << "Informations reseau recus." << endl;
	cout << "Verification et dechiffrement...\n\n" << endl;
	if (!verifyMAC(message, mac_key)) {
		cout << "\nsender is not Clement";
		return;
	}
	message = decrypt(extractMsg(message), bernard_key);
	agnesse_IP = informationReseauIPAddress(message);
	agnesse_port = informationReseauPort(message);
	if (agnesse_IP.size() > 0 && agnesse_port.size() > 0)
	{
		cout << "\nAdresse IP: ";
		cout << agnesse_IP << endl;
		cout << "Port: ";
		cout << agnesse_port << endl;
	}

	//Démarrage serveur de Bernard pour la communication entre Bernard et Agnesse
	cout << "\nEn attente de la connection..." << endl;
	try
	{
		ServerSocket server = ServerSocket(agnesse_IP.c_str(), (short)stoi(agnesse_port));
		Socket clientAgnesse = server.accept();

		runConversationAgnesseBernard(clientAgnesse, session_key, session_mac_key);
	}
	catch (const SocketException& e)
	{
		cout << endl << e.what();
	}	
}

void runConversationAgnesseBernard(Socket clientAgnesse, std::string session_key, std::string session_mac_key)
{
	string message = "";

	cout << "\n\n--------------------------------------------------------" << endl;
	cout << "\nLa connection avec Agnesse est maintenant etablie.\n" << endl;

	while (true)
	{
		//Bernard attend un reponse...
		cout << "En attente de la reception du message..." << endl;
		clientAgnesse.recvMsg(message);
		cout << "Message recu." << endl;
		cout << "Verification et dechiffrement...\n\n" << endl;

		if (!verifyMAC(message, session_mac_key)) {
			cout << "\nsender is not Agnesse";
			return;
		}
		message = decrypt(extractMsg(message), session_key);
		cout << "\nMessage recu: ";
		cout << message <<"\n" << endl;

		//Bernard écrit son message
		cout << "Pour terminer la conversation, tapez: 'end'." << endl;
		cout << "Quel est le message a transmettre?" << endl;
		getline(cin, message);

		if (message == "end") {
			//Simplement pour la condition du while
			cout << "\nEnvoie du message...\n\n" << endl;
			message = encrypt(message, session_key);
			clientAgnesse.sendMsg(message + generateMac(message, session_mac_key));
			cout << "\n\n\nMessage envoye." << endl;
			break;
		}
		else
		{
			cout << "\nEnvoie du message...\n\n" << endl;
			message = encrypt(message, session_key);
			clientAgnesse.sendMsg(message + generateMac(message, session_mac_key));
			cout << "\nMessage envoye." << endl;
		}

		
	}

	//Terminaison de la conversation
	clientAgnesse.close();
};