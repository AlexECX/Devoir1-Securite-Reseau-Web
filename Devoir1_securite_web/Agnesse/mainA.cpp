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
void runConversationAgnesseBob(SimpleSocket clientBob, std::string, std::string);

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
		cout << "\nConnection successful." << endl;
		scriptedConvoTest(client);

		}
	catch (const ConnectionException& e)
	{
		cout << endl << e.what();
	}
	

}

void scriptedConvoTest(SimpleSocket client) {
	string agnesse_key = "gadgettegdag";
	string session_key = "";
	string mac_key = "mac_key_A";
	string session_mac_key = "";
	string message = "";
	string mac = "";
	string bob_IP = "";
	string bob_port = "";

	//S'identifie aupres de Clement
	message = "is Agnesse";
	client.sendMessage(message + generateMac(message, mac_key));
	client.recvMessage(message);
	if (!authenticate(message, "is Clement", mac_key))
	{
		cout << "\nsender is not Clement";
		return;
	}

	//Agnesse reçoit une question de la part de Clément
	client.recvMessage(message);
	if (!verifyMAC(message, mac_key)) {
		cout << "\nsender is not Clement";
		return;
	}
	message = decrypt(extractMsg(message), agnesse_key);
	cout << "\n" << message << endl;

	//Agnesse mentionne a qui elle désire parler
	getline(cin, message);
	message = encrypt(message, agnesse_key);
	client.sendMessage(message + generateMac(message, mac_key));

	//Attente de la réponse de Clément
	cout << "\nEn attente de la reponse..." << endl;
	client.recvMessage(message);
	if (!verifyMAC(message, mac_key)) {
		cout << "\nsender is not Clement";
		return;
	}
	message = decrypt(extractMsg(message), agnesse_key);
	cout << "\n\n" << message << endl;


	//Réception de la clé de session et MAC de session et infos réseau de Bob
	cout << "\nEn attente d'une cle de session..." << endl;

	client.recvMessage(message);
	if (!verifyMAC(message, mac_key)) {
		cout << "\nsender is not Clement";
		return;
	}
	session_key = decrypt(extractMsg(message), agnesse_key);
	cout << "\nCle de session recu..." << endl;
	cout << "Cle de session: ";
	cout << session_key << endl;

	client.recvMessage(message);
	if (!verifyMAC(message, mac_key)) {
		cout << "\nsender is not Clement";
		return;
	}
	session_mac_key = decrypt(extractMsg(message), agnesse_key);
	cout << "\nCle de session MAC recu..." << endl;
	cout << "Cle MAC de session: ";
	cout << session_mac_key << endl;

	//Réception des information de connection reseau...
	cout << "\nEn attente des informations reseau..." << endl;
	client.recvMessage(message);
	if (!verifyMAC(message, mac_key)) {
		cout << "\nsender is not Clement";
		return;
	}
	message = decrypt(extractMsg(message), agnesse_key);
	bob_IP = informationReseauIPAddress(message);
	bob_port = informationReseauPort(message);
	if (bob_IP.size() > 0 && bob_port.size() > 0)
	{
		cout << "\nInformations reseau recu..." << endl;
		cout << "Adresse IP: ";
		cout << bob_IP << endl;
		cout << "Port: ";
		cout << bob_port << endl;
	}
	
	//Connection au serveur Bob...
	cout << "\nTentative de connection a Bob..." << endl;
	try
	{
		bool connected = false;
		SimpleSocket client = SimpleSocket();

		while (!connected) {
			try
			{
				cout << "\nconnecting to " + bob_IP + " on port " << bob_port;
				client.connectSocket(bob_IP.c_str(), stoi(bob_port));
				connected = true;
				runConversationAgnesseBob(client, session_key, session_mac_key);
			}
			catch (const ConnectionException& e)
			{
				cout << endl << e.what();
				this_thread::sleep_for(5s);
			}
		}
	}
	catch (const ConnectionException& e)
	{
		cout << endl << e.what();
	}
}

void runConversationAgnesseBob(SimpleSocket clientBob, std::string session_key, std::string session_mac_key)
{
	string message = "";


	//S'identifie aupres de Bob avec session_key et session_mac_key
	message = session_key;
	clientBob.sendMessage(message + generateMac(message, session_mac_key));
	clientBob.recvMessage(message);
	if (!authenticate(message, "is Bob", session_mac_key))
	{
		cout << "\nsender is not Bob";
		return;
	}
	else
	{
		cout << "\n\nAuthentication process success." << endl;
	}

	cout << "La connection avec Bob est maintenant etablie.\n" << endl;

	//Échange de messages tant que un des interloccuteurs ne mentionne pas la fin de la connection
	while (message != "end")
	{
		cout << "Pour terminer la conversation, tapez: 'end'." << endl;

		//Agnesse écrit son message
		cout << "Quel est le message a transmettre?" << endl;
		getline(cin, message);
		message = encrypt(message, session_key);
		clientBob.sendMessage(message + generateMac(message, session_mac_key));

		//Agnesse attend un reponse...
		cout << "\nEn attente de la reception du message..." << endl;
		clientBob.recvMessage(message);
		if (!verifyMAC(message, session_mac_key)) {
			cout << "\nsender is not Clement";
			return;
		}
		message = decrypt(extractMsg(message), session_key);
		cout << "Message recu:" << endl;
		cout << message << endl << endl;
	}

	//Terminaison de la conversation
	clientBob.close();
};