#pragma once
#include "Socket.h"

class ServerSocket :
	public BaseSocket
{
public:
	ServerSocket();
	ServerSocket(unsigned cPort);
    ServerSocket(const std::string& server_addr, unsigned cPort);
	virtual ~ServerSocket();

	ServerSocket& operator=(ServerSocket& other) {
		BaseSocket::operator = (other);
		return *this;
	}

	bool bindSocket(const std::string& server_addr, unsigned cPort, unsigned queue_size=5);

	Socket acceptSocket();


};

