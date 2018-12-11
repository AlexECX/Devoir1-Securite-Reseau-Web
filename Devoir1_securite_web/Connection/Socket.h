#pragma once
#include "BaseSocket.h"

class Socket : public BaseSocket
{
public:
	Socket();
	Socket(const SOCKET socket);
	Socket(const std::string& server_addr, unsigned cPort);
	virtual ~Socket();

	Socket& operator=(const Socket& other) {
		BaseSocket::operator = (other);
		return *this;
	}

	bool connectSocket(const std::string& server_addr, unsigned cPort);
};

