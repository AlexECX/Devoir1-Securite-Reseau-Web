#pragma once
#include <winsock.h>
#include <string>
#include "ConnectionException.h"

class SocketWrap;


class Connection
{
private:

protected:
	
	std::shared_ptr<SocketWrap> mySocket_ptr = nullptr;
	SOCKET		mySocket;
	SOCKADDR_IN addrInfo;
	

public:
	
	Connection();
	Connection(const SOCKET& socket);

	virtual ~Connection();

	Connection& operator=(const Connection& other);

	void close();

	//
	int sendFile(std::string FilePath);

	int recvFile(std::string FilePath);

	//permet d'envoyer un fichier. Si trop grand,
	//l'envoie en plusieurs paquets.
	bool sendMessage(const std::string &message);

	//permet de recevoir un fichier. Si trop grand,
	//est pret a recevoir en plusieurs paquets.
	bool recvMessage(std::string &message);

};

