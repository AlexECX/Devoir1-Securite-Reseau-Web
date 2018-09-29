#pragma once
#include "Connection.h"
class SimpleSocket : public Connection
{
public:
	SimpleSocket();
	SimpleSocket(const SOCKET socket);
	SimpleSocket(const char *szServer, short cPort);
	virtual ~SimpleSocket();

	void connectSocket(const char *szServer, short cPort);
};

