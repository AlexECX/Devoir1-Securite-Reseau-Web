#include "Connection.h"
#include "Connection_error.h"
#include <sstream>
#include <string>
// Helper macro for displaying errors
#define PRINTERROR(s)	std::cout<<"\n"<<s<<":"<<WSAGetLastError()

using namespace std;


Connection::Connection(short cPort)
{
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
	//
	// Create a TCP/IP stream socket to "listen" with
	//
	listenSocket = socket(AF_INET,                 // Address family
		SOCK_STREAM,          // Socket type
		IPPROTO_TCP);         // Protocol
	if (listenSocket == INVALID_SOCKET) {
		throw Connection_error("socket()");
		//return;
	}

	//
	// Fill in the address structure
	//
	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = INADDR_ANY;   // Indicates that connections can come from any local interface (IP address)
											 // (by the way,  INADDR_ANY is actually "0x00000000")
											 // ************ if you wish to specify a specific local interface (IP address) :
											 //              =>     saServer.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	saServer.sin_port = htons(cPort);        // Use port from command line

	//
	// bind the name to the socket
	//
	nRet = bind(listenSocket,                      // Socket
		(LPSOCKADDR)&saServer,       // Our address
		sizeof(struct sockaddr));    // Size of address structure
	if (nRet == SOCKET_ERROR) {
		closesocket(listenSocket);
		throw Connection_error("bind()");
	}

	std::cout << "\nlisten()";
	nRet = listen(listenSocket,                          // Bound socket
		SOMAXCONN);                            // Number of connection request queue
	if (nRet == SOCKET_ERROR) {
		closesocket(listenSocket);
		throw Connection_error("listen()");
	}

	//
	// This isn't normally done or required, but in this
	// example we're printing out where the server is waiting
	// so that you can connect the example client.
	//
	int nLen;
	nLen = sizeof(SOCKADDR);
	char szBuf[1024];

	nRet = gethostname(szBuf, sizeof(szBuf));
	if (nRet == SOCKET_ERROR) {
		closesocket(listenSocket);
		throw Connection_error("gethostname()");
	}

	//
	// Show the server name and port number
	//
	std::cout << "\nServer named " << szBuf << " waiting on port " << cPort;


}

Connection::Connection(short cPort, char *szServer) {
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

	//
	// Find the server
	//
	LPHOSTENT lpHostEntry;
	hostent *remoteHost;

	/*lpHostEntry = gethostbyname(szServer);
	if (lpHostEntry == NULL) {
		throw Connection_error("gethostbyname()");
	}*/

	//
	// Create a TCP/IP stream socket to "listen" with
	//
	theSocket = socket(AF_INET,                    // Address family
		SOCK_STREAM,              // Socket type
		IPPROTO_TCP);        // Protocol
	if (theSocket == INVALID_SOCKET) {
		throw Connection_error("socket()");
	}

	//
	// Fill in the address structure
	//
	/*lpHostEntry.s_addr = inet_addr(szServer);
	if (lpHostEntry.s_addr == INADDR_NONE) {
		printf("The IPv4 address entered must be a legal address\n");
		return;
	}
	else
		remoteHost = gethostbyaddr((char *)&lpHostEntry, 4, AF_INET);*/

	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = inet_addr(szServer);
	if (saServer.sin_addr.s_addr == INADDR_NONE) {
		cout << endl << szServer << " is not IPv4, hostname?";
		lpHostEntry = gethostbyname(szServer);
		if (lpHostEntry == NULL) {
			throw Connection_error(string(szServer) + " invalid parameter");
		}
		else {
			saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
		}
	}
	saServer.sin_port = htons(cPort);

	nRet = connect(theSocket,                      // Socket
		(LPSOCKADDR)&saServer,       // Our address
		sizeof(struct sockaddr));    // Size of address structure
	if (nRet == SOCKET_ERROR) {
		closesocket(theSocket);
		throw Connection_error("connect()");
	}
}

Connection::~Connection()
{
	closesocket(listenSocket);
	closesocket(theSocket);
	WSACleanup();
}

int Connection::WaitRequest() {
	//
	// Wait for an incoming request
	//

	std::cout << "\nBlocking at accept()";
	theSocket = accept(listenSocket,            // Listening socket
		NULL,                            // Optional client address
		NULL);
	if (theSocket == INVALID_SOCKET) {
		closesocket(theSocket);
		throw Connection_error("accept()");
	}
	else
		std::cout << "\nConnected with client" << endl;;

	return 0;
}

int Connection::runAsServer()
{
	//
	// We're connected to a client
	// New socket descriptor returned already
	// has clients address

	//
	// Receive data from the client
	//
	char* recvBuffer = nullptr;
	unsigned int BufferSize;

	recvFileRequest(recvBuffer, BufferSize);

	std::string FileName{ recvBuffer, BufferSize };
	delete[] recvBuffer;
	recvBuffer = nullptr;
	//
	// Display received data 
	//
	std::cout << "\nData received: " << FileName << std::endl;

	//
	// Send data back to the client
	//
	sendFile(FileName.c_str());		

	return 0;
}

int Connection::sendFile(string FilePath) {
	int nRet;
	unsigned int offset = 0;
	unsigned int File_size = 0;

	std::ifstream File_source(FilePath.c_str(), std::ios::binary);
	if (File_source.fail()) {
		offset = 0;
		File_size = 0;
		while (offset < sizeof(unsigned int)) {
			nRet = send(theSocket, (char*)&File_size + offset, 
						sizeof(unsigned int), 0);
			if (nRet == SOCKET_ERROR) {
				int ws_error = WSAGetLastError();
				closesocket(theSocket);
				throw Connection_error("\nsendFile() 01: Winsock error "
					+ std::to_string(ws_error));
			}
			else {
				offset += nRet;
			}
		}
		std::cout << "failed fetch " << FilePath;
	}
	else {
		// Get the requested file 

		// Get and send file size
		File_source.seekg(0, std::ios::end);
		File_size = unsigned int(File_source.tellg());

		offset = 0;
		while (offset < sizeof(unsigned int)) {
			nRet = send(theSocket, (char*)&File_size + offset,
						sizeof(unsigned int) - offset, 0);
			if (nRet == SOCKET_ERROR) {
				int ws_error = WSAGetLastError();
				closesocket(theSocket);
				throw Connection_error("\nIn sendFile() 02: Winsock error "
					+ std::to_string(ws_error));
			}
			else {
				offset += nRet;
			}
		}

		// Send file in multiple pakcets
		unsigned int SentBytes = 0;
		unsigned int Msg_size = 0;
		unsigned int Progress = 1;
		char* File_buffer;

		/** Tool for progress bar 1/2*/
		double ProgresStep = (double)File_size / 10.0;
		double Progression = 0;
		cout << "Progression '*'x10 [ ";
		/**/

		while (SentBytes < File_size) {
			if (File_size - SentBytes >= 5000000) {
				Msg_size = 5000000;
			}
			else {
				Msg_size = File_size - SentBytes;
			}

			File_buffer = new char[Msg_size];
			File_source.seekg(SentBytes, std::ios::beg);
			File_source.read(File_buffer, Msg_size);

			sendMessage(string(File_buffer, Msg_size));
			SentBytes += Msg_size;
			delete[] File_buffer;

			/** Tool for progress bar 2 / 2*/
			if (SentBytes > ProgresStep*Progression) {
				while (ProgresStep*Progression < SentBytes) {
					if (Progression > 8)
						cout << "* ]" << endl;
					else
						cout << "* ";
					Progression++;
				}
			}
			/**/
		}
		File_source.close();
		File_buffer = nullptr;
	}

	return 1;
}

int Connection::recvFile(std::string FilePath) {
	string FileContent;
	int nRet;
	unsigned int File_size;

	unsigned int offset = 0;
	while (offset < sizeof(unsigned int)) {
		nRet = recv(theSocket, (char*)&File_size + offset,
					sizeof(unsigned int) - offset, 0);
		if (nRet == INVALID_SOCKET) {
			int ws_error = WSAGetLastError();
			closesocket(theSocket);
			throw Connection_error("\nIn recvMessage(): Winsock error "
				+ std::to_string(ws_error));
		}
		else if (File_size == 0) {
			cout << "\nfailed to fetch " << FilePath;
			return 0;
		}
		else {
			offset += nRet;
		}
	}

	ofstream IMG_dest(FilePath.c_str(), ios::binary);
	if (IMG_dest.fail()) {
		cout << "\nfailed to open " << FilePath << " for writing";
		return 0;
	}

	// Send file in multiple pakcets
	unsigned int RecvBytes = 0;
	unsigned int Progress = 1;
	unsigned int Msg_size = 0;
	IMG_dest.seekp(0, ios::beg);

	/** Tool for progress bar 1/2*/
	double ProgresStep = (double)File_size / 10.0;
	double Progression = 0;
	cout << "Progression '*'x10 [ ";
	/**/

	while (RecvBytes < File_size) {
		if (File_size - RecvBytes >= 5000000) {
			Msg_size = 5000000;
		}
		else {
			Msg_size = File_size - RecvBytes;
		}

		if (recvMessage(FileContent)) {
			// Write to local files
			IMG_dest.write(FileContent.c_str(), FileContent.size());
			RecvBytes += Msg_size;
		}
		/** Tool for progress bar 2 / 2*/
		if (RecvBytes > ProgresStep*Progression) {
			while (ProgresStep*Progression < RecvBytes) {
				if (Progression > 8)
					cout << "* ]" << endl;
				else
					cout << "* ";
				Progression++;
			}
		}
		/**/

	}
	IMG_dest.close();

	return 1;
}

bool Connection::sendMessage(string file) {
	int nRet = 0;
	unsigned int file_size = file.size();
	const char* send_file = file.c_str();
	unsigned int offset = 0;

	while (offset < sizeof(unsigned int)) {
		nRet = send(theSocket, (char*)&file_size + offset, 
					sizeof(unsigned int) - offset, 0);
		if (nRet == SOCKET_ERROR) {
			int ws_error = WSAGetLastError();
			closesocket(theSocket);
			throw Connection_error("\nIn sendMessage() 01: Winsock error "
				+ std::to_string(ws_error));
		}
		else {
			offset += nRet;
		}
	}
	
	//std::cout << "Sending file of size: " << file_size << std::endl;

	unsigned int SentBytes = 0;
	unsigned int Msg_size = 0;

	while (SentBytes < file_size) {
		if (file_size - SentBytes >= 10000) {
			Msg_size = 10000;
		}
		else {
			Msg_size = file_size - SentBytes;
		}

		offset = 0;
		while (offset < Msg_size) {
			nRet = send(theSocket, send_file + SentBytes + offset, Msg_size - offset, 0);
			if (nRet == SOCKET_ERROR) {
				int ws_error = WSAGetLastError();
				closesocket(theSocket);
				throw Connection_error("\nIn sendMessage() 02: Winsock error "
					+ std::to_string(ws_error));
			}
			else {
				offset += nRet;
			}
		}
		
		SentBytes += Msg_size;
		Msg_size = 0;
	}

	return true;
}

bool Connection::recvMessage(string &message, bool show_progress) {
	int nRet;
	unsigned int Msg_size = 0;
	unsigned int file_size = 0;
	unsigned int offset = 0;
	char* reception;

	while (offset < sizeof(unsigned int)) {
		nRet = recv(theSocket, (char*)&Msg_size + offset,                                        
					sizeof(unsigned int) - offset, 0);                                           
		if (nRet == INVALID_SOCKET) {
			int ws_error = WSAGetLastError();
			closesocket(theSocket);
			throw Connection_error("\nIn recvMessage(): Winsock error "
				+ std::to_string(ws_error));
		}
		else {
			offset += nRet;
		}
	}
	
	//cout << "\nsize of incoming file: " << Msg_size << endl;
	file_size = Msg_size;
	if (file_size == 0) {
		cout << "\nNot Found";
		return false;
	}

	reception = nullptr;

	try {
		reception = new char[file_size]; //std::bad_alloc exception here
	}
	catch (std::bad_alloc ex) {
		int p = 0;
	}
	
	unsigned int ReceivedBytes = 0;

	// If the file is bigger than "X amount", will send in multiple packages
	while (ReceivedBytes < file_size) {
			
		if (file_size - ReceivedBytes >= 10000) {
			Msg_size = 10000;
		}
		else {
			Msg_size = file_size - ReceivedBytes;
		}

		// Receive bytes and append to currently downloading file
		offset = 0;
		while (offset < Msg_size) {
			nRet = recv(theSocket, reception + ReceivedBytes + offset,
						Msg_size - offset, 0);
			if (nRet == INVALID_SOCKET) {
				int ws_error = WSAGetLastError();
				closesocket(theSocket);
				throw Connection_error("\nIn recvMessage(): Winsock error "
					+ std::to_string(ws_error));
			}
			else {
				offset += nRet;
			}
		}
		ReceivedBytes += Msg_size;
	}
	message = string(reception, file_size);
	delete[] reception;
	return true;
}

bool Connection::sendFileRequest(string file_name) {
	unsigned int Msg_size = file_name.length();

	int Recpt;
	Recpt = send(theSocket,                   // Connected socket
		(char*)&Msg_size,                             // Data buffer
		sizeof(unsigned int),               // Buffer length
		0);                                // Flags
	if (Recpt == SOCKET_ERROR) {
		int ws_error = WSAGetLastError();
		closesocket(theSocket);
		throw Connection_error("\nIn sendFileRequest() 01: Winsock error "
			+ std::to_string(ws_error));
	}

	Recpt = send(theSocket,                   // Connected socket
		file_name.c_str(),                             // Data buffer
		Msg_size,               // Buffer length
		0);                                // Flags
	if (Recpt == SOCKET_ERROR) {
		int ws_error = WSAGetLastError();
		closesocket(theSocket);
		throw Connection_error("\nIn sendFileRequest() 02: Winsock error "
			+ std::to_string(ws_error));
	}
	return true;
}

bool Connection::recvFileRequest(char* &buffer, unsigned int &buffer_size) {
	unsigned int Msg_size;
	int Recpt;

	Recpt = recv(theSocket,                            // Connected client
		(char*)&Msg_size,                                         // Receive buffer
		sizeof(unsigned int),                           // Buffer length
		0);                                            // Flags

	if (Recpt == INVALID_SOCKET) {
		int ws_error = WSAGetLastError();
		closesocket(theSocket);
		throw Connection_error("\nIn recvFileRequest() 01: Winsock error "
			+ std::to_string(ws_error));
	}

	buffer_size = Msg_size;
	buffer = new char[buffer_size];


	//memset(szBuf, 0, sizeof(szBuf));
	Recpt = recv(theSocket,                            // Connected client
		buffer,                                         // Receive buffer
		buffer_size,                           // Buffer length
		0);                                            // Flags
	if (Recpt == INVALID_SOCKET) {
		int ws_error = WSAGetLastError();
		closesocket(theSocket);
		throw Connection_error("\nIn recvFileRequest() 02: Winsock error "
			+ std::to_string(ws_error));
	}

	return true;
}