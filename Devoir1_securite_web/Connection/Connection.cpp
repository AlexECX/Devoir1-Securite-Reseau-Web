#include "Connection.h"
#include "ConnectionException.h"
#include <sstream>
#include <string>
// Helper macro for displaying errors
#define PRINTERROR(s)	std::cout<<"\n"<<s<<":"<<WSAGetLastError()


using namespace std;

Connection::Connection(SOCKET socket)
{
	mySocket = socket;
	if (mySocket == INVALID_SOCKET || mySocket == SOCKET_ERROR) {
		throw ConnectionException(WSA_ERROR, __FILE__, __LINE__);
	}
}

Connection::Connection()
{
	//
// Create a TCP/IP stream socket to "listen" with
//
	mySocket = socket(AF_INET,                 // Address family
		SOCK_STREAM,          // Socket type
		IPPROTO_TCP);         // Protocol

	if (mySocket == INVALID_SOCKET || mySocket == SOCKET_ERROR) {
		throw ConnectionException(WSA_ERROR, __FILE__, __LINE__);
	}
}


Connection::~Connection()
{
	if (autoClose) {
		closesocket(mySocket);
	}
}


void Connection::close()
{
	closesocket(mySocket);
}

int Connection::sendFile(string FilePath) {
	int nRet;
	unsigned int offset = 0;
	unsigned int File_size = 0;

	std::ifstream File_source(FilePath.c_str(), std::ios::binary);
	if (File_source.fail()) {
		//Send 0 to client, to tell no file was found
		offset = 0;
		File_size = 0;
		while (offset < sizeof(unsigned int)) {
			nRet = send(mySocket, (char*)&File_size + offset, 
						sizeof(unsigned int), 0);
			if (nRet == SOCKET_ERROR) {
				throw ConnectionException(WSA_ERROR, __FILE__, __LINE__);
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
			nRet = send(mySocket, (char*)&File_size + offset,
						sizeof(unsigned int) - offset, 0);
			if (nRet == SOCKET_ERROR) {
				throw ConnectionException(WSA_ERROR, __FILE__, __LINE__);
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
		nRet = recv(mySocket, (char*)&File_size + offset,
					sizeof(unsigned int) - offset, 0);
		if (nRet == INVALID_SOCKET) {
			throw ConnectionException(WSA_ERROR, __FILE__, __LINE__);
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
		nRet = send(mySocket, (char*)&file_size + offset, 
					sizeof(unsigned int) - offset, 0);
		if (nRet == SOCKET_ERROR) {
			throw ConnectionException(WSA_ERROR, __FILE__, __LINE__);
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
			nRet = send(mySocket, send_file + SentBytes + offset, Msg_size - offset, 0);
			if (nRet == SOCKET_ERROR) {
				throw ConnectionException(WSA_ERROR, __FILE__, __LINE__);
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
		nRet = recv(mySocket, (char*)&Msg_size + offset,                                        
					sizeof(unsigned int) - offset, 0);                                           
		if (nRet == INVALID_SOCKET) {
			throw ConnectionException(WSA_ERROR, __FILE__, __LINE__);
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
		cout << ex.what();
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
			nRet = recv(mySocket, reception + ReceivedBytes + offset,
						Msg_size - offset, 0);
			if (nRet == INVALID_SOCKET) {
				throw ConnectionException(WSA_ERROR, __FILE__, __LINE__);
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
	Recpt = send(mySocket,                   // Connected socket
		(char*)&Msg_size,                             // Data buffer
		sizeof(unsigned int),               // Buffer length
		0);                                // Flags
	if (Recpt == SOCKET_ERROR) {
		throw ConnectionException(WSA_ERROR, __FILE__, __LINE__);
	}

	Recpt = send(mySocket,                   // Connected socket
		file_name.c_str(),                             // Data buffer
		Msg_size,               // Buffer length
		0);                                // Flags
	if (Recpt == SOCKET_ERROR) {
		throw ConnectionException(WSA_ERROR, __FILE__, __LINE__);
	}
	return true;
}

bool Connection::recvFileRequest(char* &buffer, unsigned int &buffer_size) {
	unsigned int Msg_size;
	int Recpt;

	Recpt = recv(mySocket,                            // Connected client
		(char*)&Msg_size,                                         // Receive buffer
		sizeof(unsigned int),                           // Buffer length
		0);                                            // Flags

	if (Recpt == INVALID_SOCKET) {
		throw ConnectionException(WSA_ERROR, __FILE__, __LINE__);
	}

	buffer_size = Msg_size;
	buffer = new char[buffer_size];


	//memset(szBuf, 0, sizeof(szBuf));
	Recpt = recv(mySocket,                            // Connected client
		buffer,                                         // Receive buffer
		buffer_size,                           // Buffer length
		0);                                            // Flags
	if (Recpt == INVALID_SOCKET) {
		throw ConnectionException(WSA_ERROR, __FILE__, __LINE__);
	}

	return true;
}