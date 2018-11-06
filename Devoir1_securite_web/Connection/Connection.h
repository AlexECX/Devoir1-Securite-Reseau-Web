#pragma once
#include <winsock.h>
#include <string>
#include "ConnectionException.h"
#include <memory>  

class SocketWrap;

struct connectionInfo {
	std::string IP;
	std::string Port;
};


class Connection
{
private:

protected:
	
	std::shared_ptr<SocketWrap> mySocket_ptr = nullptr;
	SOCKET		mySocket;
	SOCKADDR_IN addrInfo = {0};
	

public:
	
	Connection();
	Connection(const SOCKET& socket);

	virtual ~Connection();

	Connection& operator=(const Connection& other);
	
	connectionInfo getIPinfo();

	connectionInfo getIPinfoLocal();

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

