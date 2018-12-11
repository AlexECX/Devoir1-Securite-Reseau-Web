#pragma once
#include <winsock.h>
#include <string>
#include "SocketException.h"
#include <memory>  

class SocketWrap;

struct connectionInfo {
	std::string IP;
	std::string Port;
};


class BaseSocket
{
private:

	bool is_valid;
	std::string socket_err = "";

protected:

	int af, type, protocol;
	std::shared_ptr<SocketWrap> mySocket_ptr = nullptr;
	SOCKET		mySocket;
	SOCKADDR_IN addrInfo = { 0 };

	BaseSocket(int af=AF_INET, int type=SOCK_STREAM, int protocol=IPPROTO_TCP);
	BaseSocket(const SOCKET& socket);
	BaseSocket& operator=(const BaseSocket& other);
	void socketError(const std::string& msg, std::string f);

public:

	virtual ~BaseSocket();

	connectionInfo getIPinfo();

	connectionInfo getIPinfoLocal();

	std::string getSocketErr() { return this->socket_err; }

	int getWSAError() { return WSAGetLastError(); }

	void close();

	bool valid_socket() { return this->is_valid; }

	bool sendFile(std::string FilePath);

	bool recvFile(std::string FilePath);

	//permet d'envoyer un fichier. Si trop grand,
	//l'envoie en plusieurs paquets.
	bool sendMsg_noExcept_noExcept(const std::string &message);

	//permet de recevoir un fichier. Si trop grand,
	//est pret a recevoir en plusieurs paquets.
	bool recvMsg_noExcept(std::string &message);

	void sendMsg_noExcept(const std::string &message);

	void recvMsg(std::string &message);

	std::string recvMsg();

};

