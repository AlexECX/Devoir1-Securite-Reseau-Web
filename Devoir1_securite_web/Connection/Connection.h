#pragma once
#include <winsock.h>
#include <iostream>
#include <fstream>
#include <string>

#define WSA_ERROR "Winsock error "+std::to_string(WSAGetLastError())

class Connection
{
private:

	bool autoClose = true;

protected:
	
	SOCKET		mySocket;
	SOCKADDR_IN saServer;
	

public:
	
	Connection();
	Connection(SOCKET socket);

	virtual ~Connection();

	//int waitRequest();

	//void connectTo(short cPort, char *szServer);

	//void listenOn(short cPort);

	void operator=(Connection& other) {
		mySocket = other.mySocket;
		other.autoClose = false;
	}

	void close();

	int sendFile(std::string FilePath);

	int recvFile(std::string FilePath);

	//Permet d'envoyer le nom du fichier à envoyé
	bool sendFileRequest(std::string file_name);

	//Permet de recevoir le nom du fichier à envoyé
	bool recvFileRequest(char* &buffer, unsigned int &buffer_size);

	//permet d'envoyer un fichier. Si trop grand,
	//l'envoie en plusieurs paquets.
	bool sendMessage(std::string file);

	//permet de recevoir un fichier. Si trop grand,
	//est pret a recevoir en plusieurs paquets.
	bool recvMessage(std::string &message, bool show_progress=false);


};

