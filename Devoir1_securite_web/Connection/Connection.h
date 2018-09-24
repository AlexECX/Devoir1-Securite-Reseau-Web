#pragma once
#include <winsock.h>
#include <iostream>
#include <fstream>
#include <string>

class Connection
{
private:
	SOCKET      theSocket;
	SOCKET      listenSocket;
	SOCKADDR_IN saServer;

public:
	//mode listen()
	Connection(short cPort);

	//mode connect() 
	Connection(short cPort, char *szServer);

	~Connection();

	int runAsServer();

	int WaitRequest();

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

