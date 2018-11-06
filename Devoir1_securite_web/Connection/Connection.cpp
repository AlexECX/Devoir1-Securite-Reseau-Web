#include "Connection.h"
#include "ConnectionException.h"
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

//Wrapper to help manage SOCKET lifetime
class SocketWrap
{
private:
	SOCKET theSocket;
public:
	SocketWrap(SOCKET socket) {
		theSocket = socket;
	}
	~SocketWrap() {
		closesocket(theSocket);
	}

	SOCKET& getTheSocket() {
		return theSocket;
	}
};

Connection::Connection(const SOCKET& socket)
{
	mySocket_ptr = make_shared<SocketWrap>(socket);
	mySocket = (mySocket_ptr->getTheSocket());
	if (mySocket == INVALID_SOCKET || mySocket == SOCKET_ERROR) {
		throw ConnectionException(WSA_ERROR, TRACEBACK);
	}
	int tmp = sizeof(addrInfo);
	getpeername(mySocket, (sockaddr *)&addrInfo, &tmp);
}

Connection::Connection()
{
	mySocket_ptr = make_shared<SocketWrap>(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
	mySocket = (mySocket_ptr->getTheSocket());
	// Address family // Socket type // Protocol

	if (mySocket == INVALID_SOCKET || mySocket == SOCKET_ERROR) {
		throw ConnectionException(WSA_ERROR, TRACEBACK);
	}
	int tmp = sizeof(addrInfo);
	getpeername(mySocket, (sockaddr *)&addrInfo, &tmp);
}


Connection::~Connection()
{
}

Connection & Connection::operator=(const Connection & other) {
	mySocket_ptr = other.mySocket_ptr;
	mySocket = mySocket_ptr->getTheSocket();
	return *this;
}

connectionInfo Connection::getIPinfo()
{
	int tmp = sizeof(addrInfo);
	getpeername(mySocket, (sockaddr *)&addrInfo, &tmp);
	connectionInfo name;
	name.IP = inet_ntoa(addrInfo.sin_addr);
	name.Port = to_string(ntohs(addrInfo.sin_port));
	return name;
}

connectionInfo Connection::getIPinfoLocal()
{
	int tmp = sizeof(addrInfo);
	getsockname(mySocket, (sockaddr *)&addrInfo, &tmp);
	connectionInfo name;
	name.IP = inet_ntoa(addrInfo.sin_addr);
	name.Port = to_string(ntohs(addrInfo.sin_port));
	return name;
}


void Connection::close()
{
	closesocket(mySocket);
}

int Connection::sendFile(string FilePath) {
	int nRet;
	unsigned offset = 0;
	unsigned File_size = 0;

	std::ifstream File_source(FilePath.c_str(), std::ios::binary);
	if (File_source.fail()) {
		//Send 0 to client, to tell no file was found
		offset = 0;
		File_size = 0;
		while (offset < sizeof(unsigned)) {
			nRet = send(mySocket, (char*)&File_size + offset, 
						sizeof(unsigned), 0);
			if (nRet == SOCKET_ERROR) {
				throw ConnectionException(WSA_ERROR, TRACEBACK);
			}
			else {
				offset += nRet;
			}
		}
		std::cout << "failed fetch " << FilePath;
		return 0;
	}
	else {
		// Get the requested file 

		// Get and send file size
		File_source.seekg(0, std::ios::end);
		File_size = unsigned(File_source.tellg());

		offset = 0;
		while (offset < sizeof(unsigned)) {
			nRet = send(mySocket, (char*)&File_size + offset,
						sizeof(unsigned) - offset, 0);
			if (nRet == SOCKET_ERROR) {
				throw ConnectionException(WSA_ERROR, TRACEBACK);
			}
			else {
				offset += nRet;
			}
		}

		// Send file in multiple pakcets
		unsigned SentBytes = 0;
		unsigned Msg_size = 0;
		unsigned Progress = 1;
		auto File_buffer = std::make_unique<char[]>(5000000);

		/** Tool for progress bar 1/2*/
		double ProgresStep = static_cast<double>(File_size)/ 10.0;
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

			File_source.seekg(SentBytes, std::ios::beg);
			File_source.read(File_buffer.get(), Msg_size);

			sendMessage(string(File_buffer.get(), Msg_size));
			SentBytes += Msg_size;

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

		return 1;
	}
	
}

int Connection::recvFile(std::string FilePath) {
	string FileContent;
	int nRet;
	unsigned File_size;

	unsigned offset = 0;
	while (offset < sizeof(unsigned)) {
		nRet = recv(mySocket, (char*)&File_size + offset,
					sizeof(unsigned) - offset, 0);
		if (nRet == INVALID_SOCKET || nRet == 0) {
			throw ConnectionException(WSA_ERROR, TRACEBACK);
		}
		else {
			offset += nRet;
		}
	}

	if (File_size == 0) {
	cout << "\nfailed to fetch " << FilePath;
	return 0;
		}

	ofstream IMG_dest(FilePath.c_str(), ios::binary);
	if (IMG_dest.fail()) {
		cout << "\nfailed to open " << FilePath << " for writing";
		return 0;
	}

	// Send file in multiple pakcets
	unsigned RecvBytes = 0;
	unsigned Progress = 1;
	unsigned Msg_size = 0;
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
	unsigned file_size = message.size();
	const char* send_file = message.c_str();
	unsigned offset = 0;

	while (offset < sizeof(unsigned)) {
		nRet = send(mySocket, (char*)&file_size + offset, 
					sizeof(unsigned) - offset, 0);
		if (nRet == SOCKET_ERROR) {
			throw ConnectionException(WSA_ERROR, TRACEBACK);
		}
		else {
			offset += nRet;
		}
	}
	
	unsigned SentBytes = 0;
	unsigned Msg_size = 0;

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
	unsigned Msg_size = 0;
	unsigned offset = 0;

	message.clear();

	while (offset < sizeof(unsigned)) {
		nRet = recv(mySocket, (char*)&Msg_size + offset,                                        
					sizeof(unsigned) - offset, 0);                                           
		if (nRet == INVALID_SOCKET || nRet == 0) {
			throw ConnectionException(WSA_ERROR, TRACEBACK);
		}
		else {
			offset += nRet;
		}
	}
	
	unsigned file_size = Msg_size;
	unsigned ReceivedBytes = 0;
	char reception[10000];
	//char* reception = new char[file_size];
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
			if (nRet == INVALID_SOCKET || nRet == 0) {
				throw ConnectionException(WSA_ERROR, TRACEBACK);
			}
			else {
				offset += nRet;
			}
		}
		message.append(reception, Msg_size);
		ReceivedBytes += Msg_size;
	}

	return true;
}

