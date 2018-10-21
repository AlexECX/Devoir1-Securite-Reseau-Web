#include "Connection.h"
#include "ConnectionException.h"
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

Connection::Connection(const SOCKET& socket)
{
	mySocket_ptr = make_shared<SocketWrap>(socket);
	mySocket = (mySocket_ptr->getTheSocket());
	if (mySocket == INVALID_SOCKET || mySocket == SOCKET_ERROR) {
		throw ConnectionException(WSA_ERROR, TRACEBACK);
	}
}

Connection::Connection()
{
	mySocket_ptr = make_shared<SocketWrap>(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
	mySocket = (mySocket_ptr->getTheSocket());
	// Address family // Socket type // Protocol

	if (mySocket == INVALID_SOCKET || mySocket == SOCKET_ERROR) {
		throw ConnectionException(WSA_ERROR, TRACEBACK);
	}
}


Connection::~Connection()
{
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
				throw ConnectionException(WSA_ERROR, TRACEBACK);
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
				throw ConnectionException(WSA_ERROR, TRACEBACK);
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
			throw ConnectionException(WSA_ERROR, TRACEBACK);
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

bool Connection::sendMessage(const string &message) {
	int nRet = 0;
	unsigned int file_size = message.size();
	const char* send_file = message.c_str();
	unsigned int offset = 0;

	while (offset < sizeof(unsigned int)) {
		nRet = send(mySocket, (char*)&file_size + offset, 
					sizeof(unsigned int) - offset, 0);
		if (nRet == SOCKET_ERROR) {
			throw ConnectionException(WSA_ERROR, TRACEBACK);
		}
		else {
			offset += nRet;
		}
	}
	
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
				throw ConnectionException(WSA_ERROR, TRACEBACK);
			}
			else {
				offset += nRet;
			}
		}
		SentBytes += Msg_size;
	}

	return true;
}

bool Connection::recvMessage(string &message) {
	int nRet;
	unsigned int Msg_size = 0;
	unsigned int offset = 0;

	message.clear();

	while (offset < sizeof(unsigned int)) {
		nRet = recv(mySocket, (char*)&Msg_size + offset,                                        
					sizeof(unsigned int) - offset, 0);                                           
		if (nRet == INVALID_SOCKET) {
			throw ConnectionException(WSA_ERROR, TRACEBACK);
		}
		else {
			offset += nRet;
		}
	}
	
	unsigned int file_size = Msg_size;
	unsigned int ReceivedBytes = 0;
	char reception[10000];
	//memset(reception, '\0', 10000);
	
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
			nRet = recv(mySocket, reception + offset, Msg_size - offset, 0);
			if (nRet == INVALID_SOCKET) {
				throw ConnectionException(WSA_ERROR, TRACEBACK);
			}
			else {
				message.append(reception, Msg_size);
				offset += nRet;
			}
		}
		ReceivedBytes += Msg_size;
	}

	return true;
}