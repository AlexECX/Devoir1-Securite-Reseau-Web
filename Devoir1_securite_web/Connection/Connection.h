#pragma once
#include <winsock.h>
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

	void operator=(Connection& other) {
		mySocket = other.mySocket;
		other.autoClose = false;
	}

	void close();

	//
	int sendFile(std::string FilePath);

	int recvFile(std::string FilePath);

	//permet d'envoyer un fichier. Si trop grand,
	//l'envoie en plusieurs paquets.
	bool sendMessage(const std::string &message);

	//permet de recevoir un fichier. Si trop grand,
	//est pret a recevoir en plusieurs paquets.
	bool recvMessage(std::string &message, bool show_progress=false);


};

