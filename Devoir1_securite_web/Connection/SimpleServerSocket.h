#pragma once
#include "Connection.h"
#include "SimpleSocket.h"

class SimpleServerSocket :
	public Connection
{
public:
	SimpleServerSocket();
	SimpleServerSocket(short cPort);
    SimpleServerSocket(const char * szServer, short cPort);
	virtual ~SimpleServerSocket();

	SimpleServerSocket& operator=(SimpleServerSocket& other) {
		Connection::operator = (other);
		return *this;
	}

	void bindSocket(const char * szServer, short cPort);

	SimpleSocket acceptSocket();


};

