#include <iostream>
#include <string>
#include <windows.h>
#include <thread>
#include "../Connection/SocketException.h"
#include "../Connection/Utils.h"
#include "../Connection/Socket.h"
#include "../Encryption/encryption.h"

using namespace std;

#define HARD_CODED
#define HOST "127.0.0.1" //Can be Name or IP address
#define PORT 2030

void runClient(unsigned nPort, const string& host);
void scriptedConvoTest(Socket client);
void runConversationAgnesseBernard(Socket clientBernard, std::string, std::string);
string getFileName(const string& s);

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
	string agnesse_key = "gadgettegdag";
	string session_key = "";
	string mac_key = "mac_key_A";
	string session_mac_key = "";
	string message = "";
	string mac = "";
	string bernard_IP = "";
	string bernard_port = "";

	//S'identifie aupres de Clement
	cout << "\nAuthentification d'Agnesse aupres de Clement..." << endl;
	cout << "Envoie d'un message..." << endl;
	message = "is Agnesse";
	client.sendMsg_noExcept(message + generateMac(message, mac_key));
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


	//Agnesse reçoit une question de la part de Clément
	cout << "\n\n\nReception d'un message de la part de Clement..." << endl;
	client.recvMsg(message);
	cout << "Message recu." << endl;
	cout << "Verification et dechiffrement...\n\n" << endl;
	if (!verifyMAC(message, mac_key)) {
		cout << "\nsender is not Clement";
		return;
	}
	message = decrypt(extractMsg(message), agnesse_key);
	cout << "\n\n\nMessage pour Agnesse:\n" << message << endl;

	//Agnesse mentionne a qui elle désire parler
	getline(cin, message);
	cout << "\nEnvoie d'un message...\n\n" << endl;
	message = encrypt(message, agnesse_key);
	client.sendMsg_noExcept(message + generateMac(message, mac_key));
	cout << "\n\n\nMessage envoye a Clement." << endl;

	//Attente de la réponse de Clément
	cout << "En attente de la reponse de Clement..." << endl;
	client.recvMsg(message);
	cout << "Reponse recu." << endl;
	cout << "Verification et dechiffrement...\n\n" << endl;
	if (!verifyMAC(message, mac_key)) {
		cout << "\nsender is not Clement";
		return;
	}
	message = decrypt(extractMsg(message), agnesse_key);
	cout << "\n\n\nMessage pour Agnesse:" << endl;
	cout <<  message << endl;

	//Réception de la clé de session et MAC de session et infos réseau de Bernard
	cout << "\nReception de la cle de session..." << endl;

	client.recvMsg(message);
	cout << "Message recu." << endl;
	cout << "Verification et dechiffrement...\n\n" << endl;
	if (!verifyMAC(message, mac_key)) {
		cout << "\nsender is not Clement";
		return;
	}
	session_key = decrypt(extractMsg(message), agnesse_key);
	cout << "\n\n\nCle de session: ";
	cout << session_key << endl;

	cout << "Reception de la cle MAC de session..." << endl;
	client.recvMsg(message);
	cout << "Message recu..." << endl;
	cout << "Verification et dechiffrement...\n\n" << endl;
	if (!verifyMAC(message, mac_key)) {
		cout << "\nsender is not Clement";
		return;
	}
	session_mac_key = decrypt(extractMsg(message), agnesse_key);
	cout << "\n\n\nCle de session MAC recu." << endl;
	cout << "Cle MAC de session: ";
	cout << session_mac_key << endl;

	//Réception des information de connection reseau...
	cout << "Reception des informations reseau..." << endl;
	client.recvMsg(message);
	cout << "Informations reseau recus." << endl;
	cout << "Verification et dechiffrement...\n\n" << endl;
	if (!verifyMAC(message, mac_key)) {
		cout << "\nsender is not Clement";
		return;
	}
	message = decrypt(extractMsg(message), agnesse_key);
	bernard_IP = informationReseauIPAddress(message);
	bernard_port = informationReseauPort(message);
	if (bernard_IP.size() > 0 && bernard_port.size() > 0)
	{
		cout << "\n\nAdresse IP: ";
		cout << bernard_IP << endl;
		cout << "Port: ";
		cout << bernard_port << endl;
	}
	
	//Connection au serveur Bernard...
	cout << "\nTentative de connection a Bernard..." << endl;
	try
	{
		bool connected = false;
		Socket clientB = Socket();

		while (!connected) {
			try
			{
				cout << "\nconnecting to " + bernard_IP + " on port " << bernard_port;
				clientB.connectSocket(bernard_IP.c_str(), stoi(bernard_port));
				connected = true;
			}
			catch (const SocketException& e)
			{
				cout << endl << e.what();
				this_thread::sleep_for(5s);
			}

			runConversationAgnesseBernard(clientB, session_key, session_mac_key);

		}
	}
	catch (const SocketException& e)
	{
		cout << endl << e.what();
	}
}

void runConversationAgnesseBernard(Socket clientBernard, std::string session_key, std::string session_mac_key)
{
	string message = "";

	cout << "\n\n--------------------------------------------------------" << endl;
	cout << "\n\nLa connection avec Bernard est maintenant etablie.\n" << endl;

	//Échange de messages tant que un des interloccuteurs ne mentionne pas la fin de la connection
	while (true)
	{
		cout << "Pour terminer la conversation, tapez: 'end'." << endl;

		//Agnesse écrit son message
		cout << "Quel est le message a transmettre?" << endl;
		getline(cin, message);

		if (message == "end") {
			cout << "\nEnvoie du message...\n\n" << endl;
			message = encrypt(message, session_key);
			clientBernard.sendMsg_noExcept(message + generateMac(message, session_mac_key));
			cout << "\n\n\nMessage envoye." << endl;
			break;
		}
		else
		{
			cout << "\nEnvoie du message...\n\n" << endl;
			message = encrypt(message, session_key);
			clientBernard.sendMsg_noExcept(message + generateMac(message, session_mac_key));
			cout << "\nMessage envoye." << endl;

			//Agnesse attend un reponse...
			cout << "\nEn attente de la reception du message..." << endl;
			clientBernard.recvMsg(message);
			cout << "Message recu." << endl;
			cout << "Verification et dechiffrement...\n\n" << endl;

			if (!verifyMAC(message, session_mac_key)) {
				cout << "\nsender is not Bernard";
				return;
			}
			message = decrypt(extractMsg(message), session_key);
			cout << "\n\nMessage recu: ";
			cout << message << "\n\n" << endl;
		}
		
	}

	//Terminaison de la conversation
	clientBernard.close();
};


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