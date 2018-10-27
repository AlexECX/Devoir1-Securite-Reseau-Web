#include <iostream>
#include <string>
#include <windows.h>
#include <thread>
#include "../Connection/ConnectionException.h"
#include "../Connection/Utils.h"
#include "../Connection/SimpleSocket.h"
#include "../Connection/SimpleServerSocket.h"
#include "../Encryption/encryption.h"

using namespace std;

#define HARD_CODED
#define HOST "127.0.0.1" //Can be Name or IP address
#define PORT 2030

void runClient(short nPort, const char* host);
void scriptedConvoTest(SimpleSocket client);
void runConversationAgnesseBob(SimpleSocket clientAgnesse, std::string, std::string);

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
		cout << "\nConnection successful.\n" << endl;
		scriptedConvoTest(client);
	}
	catch (const ConnectionException& e)
	{
		cout << endl << e.what();
	}
	

}

void scriptedConvoTest(SimpleSocket client) {
	string bob_key = "bobinoonibob";
	string session_key = "";
	string mac_key = "mac_key_B";
	string session_mac_key = "";
	string message = "";
	string mac = "";
	string agnesse_IP = "";
	string agnesse_port = "";

	//S'identifie aupres de Clement
	message = "is Bob";
	client.sendMessage(message + generateMac(message, mac_key));
	client.recvMessage(message);
	if (!authenticate(message, "is Clement", mac_key))
	{
		cout << "\nsender is not Clement";
		return;
	}

	//Réception d'un message de Clément
	client.recvMessage(message);
	if (!verifyMAC(message, mac_key)) {
		cout << "\nsender is not Clement";
		return;
	}
	message = decrypt(extractMsg(message), bob_key);
	cout << message << endl;

	//Réception de la clé de session et MAC de session et infos réseau de Bob
	cout << "\nEn attente d'une cle de session..." << endl;
	
	client.recvMessage(message);
	if (!verifyMAC(message, mac_key)) {
		cout << "\nsender is not Clement";
		return;
	}
	session_key = decrypt(extractMsg(message), bob_key);
	cout << "\nCle de session recu..." << endl;
	cout << "Cle de session: ";
	cout << session_key << endl;

	client.recvMessage(message);
	if (!verifyMAC(message, mac_key)) {
		cout << "\nsender is not Clement";
		return;
	}
	session_mac_key = decrypt(extractMsg(message), bob_key);
	cout << "\nCle de session MAC recu..." << endl;
	cout << "Cle MAC de session: ";
	cout << session_mac_key << endl;

	//Réception des informations réseau
	cout << "\nEn attente des informations reseau..." << endl;
	client.recvMessage(message);
	if (!verifyMAC(message, mac_key)) {
		cout << "\nsender is not Clement";
		return;
	}
	message = decrypt(extractMsg(message), bob_key);
	agnesse_IP = informationReseauIPAddress(message);
	agnesse_port = informationReseauPort(message);
	if (agnesse_IP.size() > 0 && agnesse_port.size() > 0)
	{
		cout << "\nInformations reseau recu..." << endl;
		cout << "Adresse IP: ";
		cout << agnesse_IP << endl;
		cout << "Port: ";
		cout << agnesse_port << endl;
	}

	//Démarrage serveur de Bob pour la communication Bob et Agnesse
	cout << "\nEn attente de la connection..." << endl;
	try
	{
		SimpleServerSocket server = SimpleServerSocket((short)stoi(agnesse_port));
		SimpleSocket clientAgnesse;

		SimpleSocket client = server.acceptSocket();
		client.recvMessage(message);
		if (authenticate(message, session_key, session_mac_key)) 
		{
			cout << "\n\nAuthentification process success." << endl;
			clientAgnesse = client;
			message = "is Bob";
			clientAgnesse.sendMessage(message + generateMac(message, session_mac_key));
			runConversationAgnesseBob(clientAgnesse, session_key, session_mac_key);
		}
		else {
			cout << "\nCriteria not met for connection (session key do not match...";
			client.close();
			return;
		}

	}
	catch (const ConnectionException& e)
	{
		cout << endl << e.what();
	}	
}

void runConversationAgnesseBob(SimpleSocket clientAgnesse, std::string session_key, std::string session_mac_key)
{
	string message = "";
	string message1 = "";

	cout << "La connection avec Agnsse est maintenant etablie.\n" << endl;

	while (message1 != "end")
	{
		//Bob attend un reponse...
		cout << "En attente de la reception du message..." << endl;
		clientAgnesse.recvMessage(message);
		if (!verifyMAC(message, session_mac_key)) {
			cout << "\nsender is not Clement";
			return;
		}
		message = decrypt(extractMsg(message), session_key);
		cout << "\nMessage recu:\n";
		cout << message << endl;

		//Bob écrit son message
		cout << "\nPour terminer la conversation, tapez: 'end'." << endl;
		cout << "\nQuel est le message a transmettre?" << endl;
		getline(cin, message);
		//Simplement pour la condition du while
		message1 = message;
		message = encrypt(message, session_key);
		clientAgnesse.sendMessage(message + generateMac(message, session_mac_key));
	}

	//Terminaison de la conversation
	clientAgnesse.close();
};