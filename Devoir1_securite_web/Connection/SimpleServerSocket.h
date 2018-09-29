#pragma once
#include "Connection.h"
#include "SimpleSocket.h"

class SimpleServerSocket :
	public Connection
{
public:
	SimpleServerSocket();
	SimpleServerSocket(short cPort);
	virtual ~SimpleServerSocket();

	void bindSocket(const char * szServer, short cPort);

	SimpleSocket acceptSocket();


};

