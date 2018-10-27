#pragma once
#include <winsock.h>
#include <iostream>
#include "Utils.h"

using namespace std;


void initWSA(){
    WORD wVersionRequested = MAKEWORD(1, 1);
	WSADATA wsaData;
	int nRet;
	//
	// Initialize WinSock and check the version
	//
	nRet = WSAStartup(wVersionRequested, &wsaData);
	if (wsaData.wVersion != wVersionRequested) {
		cout << "Wrong version";
		return;
	}
}

string informationReseauIPAddress(std::string message)
{
	int i = 0;
	string IP = "";

	while (message[i] != ':')
	{
		IP += message[i];
		i++;
	}

	return IP;
}

string informationReseauPort(std::string message)
{
	int i = 0;
	string port = "";

	while (message[i] != ':')
	{
		i++;
	}
	i++;
	while (i < message.size())
	{
		port += message[i];
		i++;
	}

	return port;
}