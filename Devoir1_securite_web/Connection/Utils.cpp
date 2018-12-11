#pragma once
#include <winsock.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include "Utils.h"
#include <map>

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

static map<int, std::string> formatedWSAErrorCode = {
	{6, "WSA_INVALID_HANDLE"}, {8, "WSA_NOT_ENOUGH_MEMORY"}, {87, "WSA_INVALID_PARAMETER"},
	{995, "WSA_OPERATION_ABORTED"}, {996, "WSA_IO_INCOMPLETE"}, {997, "WSA_IO_PENDING"},
	{10004, "WSAEINTR"}, {10009, "WSAEBADF"}, {10013, "WSAEACCES"},
	{10014, "WSAEFAULT"}, {10022, "WSAEINVAL"}, {10024, "WSAEMFILE"},
	{10035, "WSAEWOULDBLOCK"}, {10036, "WSAEINPROGRESS"}, {10037, "WSAEALREADY"},
	{10038, "WSAENOTSOCK"}, {10039, "WSAEDESTADDRREQ"}, {10040, "WSAEMSGSIZE"},
	{10041, "WSAEPROTOTYPE"}, {10042, "WSAENOPROTOOPT"}, {10043, "WSAEPROTONOSUPPORT"},
	{10044, "WSAESOCKTNOSUPPORT"}, {10045, "WSAEOPNOTSUPP"}, {10046, "WSAEPFNOSUPPORT"},
	{10047, "WSAEAFNOSUPPORT"}, {10048, "WSAEADDRINUSE"}, {10049, "WSAEADDRNOTAVAIL"},
	{10050, "WSAENETDOWN"}, {10051, "WSAENETUNREACH"}, {10052, "WSAENETRESET"},
	{10053, "WSAECONNABORTED"}, {10054, "WSAECONNRESET"}, {10055, "WSAENOBUFS"},
	{10056, "WSAEISCONN"}, {10057, "WSAENOTCONN"}, {10058, "WSAESHUTDOWN"},
	{10059, "WSAETOOMANYREFS"}, {10060, "WSAETIMEDOUT"}, {10061, "WSAECONNREFUSED"},
	{10062, "WSAELOOP"}, {10063, "WSAENAMETOOLONG"}, {10064, "WSAEHOSTDOWN"},
	{10065, "WSAEHOSTUNREACH"}, {10066, "WSAENOTEMPTY"}, {10067, "WSAEPROCLIM"},
	//{10000, "NONE"}, {10000, "NONE"}, {10000, "NONE"},
	/*TODO implement rest of error codes*/
	
	
};

std::string formateWSAError(int error_code) {
	std::map<int, std::string>::iterator it;
	it = formatedWSAErrorCode.find(error_code);
	if (it == formatedWSAErrorCode.end()) {
		return std::to_string(error_code);
	}
	else {
		return it->second;
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
	unsigned i = 0;
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

std::string& tolower_str(std::string &str)
{
	for (auto &ch : str)
	{
		ch = tolower(ch);
	}
	return str;
}
