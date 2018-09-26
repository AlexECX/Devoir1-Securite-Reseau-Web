#pragma once
#include "Connection.h"
class SimpleSocket : public Connection
{
public:
	SimpleSocket();
	SimpleSocket(SOCKET socket);
	SimpleSocket(char *szServer, short cPort);
	virtual ~SimpleSocket();

	void connectSocket(char *szServer, short cPort);
};

