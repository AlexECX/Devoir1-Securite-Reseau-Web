#include "BaseSocket.h"
#include "SocketException.h"
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

	SOCKET& getSocket() {
		return theSocket;
	}
};


BaseSocket::BaseSocket(const SOCKET& socket)
{
	mySocket_ptr = make_shared<SocketWrap>(socket);
	mySocket = (mySocket_ptr->getSocket());
	if (mySocket == INVALID_SOCKET || mySocket == SOCKET_ERROR) {
		this->socketError(WSA_ERROR, __FUNCTION__);
		//throw SocketException(WSA_ERROR, __FUNCTION__);
	}

}


BaseSocket::BaseSocket(int af, int type, int protocol)
{
	this->af = af;
	this->type = type;
	this->protocol = protocol;
	mySocket_ptr = make_shared<SocketWrap>(::socket(af, type, protocol));
	mySocket = (mySocket_ptr->getSocket());

	if (mySocket == INVALID_SOCKET || mySocket == SOCKET_ERROR) {
		this->socketError(WSA_ERROR, __FUNCTION__);
		//throw SocketException(WSA_ERROR, __FUNCTION__);
	}

}


BaseSocket::~BaseSocket()
{
}

BaseSocket & BaseSocket::operator=(const BaseSocket & other) {
	mySocket_ptr = other.mySocket_ptr;
	mySocket = mySocket_ptr->getSocket();
	return *this;
}


void BaseSocket::socketError(const string& msg, string f)
{
	this->socket_err = f + " " + msg + "\n";
}


connectionInfo BaseSocket::getIPinfo()
{
	int tmp = sizeof(addrInfo);
	::getpeername(mySocket, (sockaddr *)&addrInfo, &tmp);
	connectionInfo info;
	info.IP = ::inet_ntoa(addrInfo.sin_addr);
	info.Port = to_string(::ntohs(addrInfo.sin_port));
	return info;
}

connectionInfo BaseSocket::getIPinfoLocal()
{
	int tmp = sizeof(addrInfo);
	getsockname(mySocket, (sockaddr *)&addrInfo, &tmp);
	connectionInfo info;
	info.IP = ::inet_ntoa(addrInfo.sin_addr);
	info.Port = to_string(::ntohs(addrInfo.sin_port));
	return info;
}

std::string BaseSocket::getSocketErr()
{
	return this->socket_err.substr(this->socket_err.find(" ") + 1);
}

int BaseSocket::shutdown(int how)
{
	int status = ::shutdown(this->mySocket, how);
	if (status < 0) {
		this->socketError(WSA_ERROR, __FUNCTION__);
		return status;
	}
	return status;
}

void BaseSocket::close()
{
	::closesocket(mySocket);
}

int BaseSocket::sendFile(string FilePath) {
	int nRet;
	unsigned offset = 0;
	unsigned File_size = 0;

	std::ifstream File_source(FilePath.c_str(), std::ios::binary);
	if (File_source.fail()) {
		//Send 0 to client, to tell no file was found
		offset = 0;
		File_size = 0;
		while (offset < sizeof(unsigned)) {
			nRet = ::send(mySocket, (char*)&File_size + offset, 
						sizeof(unsigned), 0);
			if (nRet < 0) {
				this->socketError(WSA_ERROR, __FUNCTION__);
				return nRet;
				//throw SocketException(WSA_ERROR, __FUNCTION__);
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
			nRet = ::send(mySocket, (char*)&File_size + offset,
						sizeof(unsigned) - offset, 0);
			if (nRet < 0) {
				this->socketError(WSA_ERROR, __FUNCTION__);
				return nRet;
				//throw SocketException(WSA_ERROR, __FUNCTION__);
			}
			else {
				offset += nRet;
			}
		}

		// Send file in multiple packets
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

			nRet = sendMsg_noExcept(string(File_buffer.get(), Msg_size));
			if (nRet > 0) {
				SentBytes += Msg_size;
			}

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

		return nRet;
	}
	
}

int BaseSocket::recvFile(std::string FilePath) {
	string FileContent;
	int nRet;
	unsigned File_size;

	unsigned offset = 0;
	while (offset < sizeof(unsigned)) {
		nRet = ::recv(mySocket, (char*)&File_size + offset,
					sizeof(unsigned) - offset, 0);
		if (nRet <= 0) {
			this->socketError(WSA_ERROR, __FUNCTION__);
			return nRet;
			//throw SocketException(WSA_ERROR, __FUNCTION__);
		}
		else {
			offset += nRet;
		}
	}

	if (File_size == 0) {
		this->socketError("RemoteFileNotFound", __FUNCTION__);
		return -2;
		}

	ofstream IMG_dest(FilePath.c_str(), ios::binary);
	if (IMG_dest.fail()) {
		this->socketError("CannotOpenFile", __FUNCTION__);
		return -2;
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

		if (recvMsg_noExcept(FileContent) > 0) {
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

int BaseSocket::sendMsg_noExcept(const string &message) {
	int nRet = 0;
	unsigned file_size = message.size();
	const char* send_file = message.c_str();
	unsigned offset = 0;

	while (offset < sizeof(unsigned)) {
		nRet = ::send(mySocket, (char*)&file_size + offset, 
					sizeof(unsigned) - offset, 0);
		if (nRet < 0) {
			this->socketError(WSA_ERROR, __FUNCTION__);
			return nRet;
			//throw SocketException(WSA_ERROR, __FUNCTION__);
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
			nRet = ::send(mySocket, send_file + SentBytes + offset, Msg_size - offset, 0);
			if (nRet < 0) {
				this->socketError(WSA_ERROR, __FUNCTION__);
				return nRet;
				//throw SocketException(WSA_ERROR, __FUNCTION__);
			}
			else {
				offset += nRet;
			}
		}
		SentBytes += Msg_size;
	}

	return nRet;
}

int BaseSocket::recvMsg_noExcept(string &message) {
	int nRet;
	unsigned Msg_size = 0;
	unsigned offset = 0;

	message.clear();

	while (offset < sizeof(unsigned)) {
		nRet = ::recv(mySocket, (char*)&Msg_size + offset,                                        
					sizeof(unsigned) - offset, 0);                                           
		if (nRet <= 0) {
			this->socketError(WSA_ERROR, __FUNCTION__);
			return nRet;
			//throw SocketException(WSA_ERROR, __FUNCTION__);
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
			nRet = ::recv(mySocket, reception + offset, Msg_size - offset, 0);
			if (nRet <= 0) {
				this->socketError(WSA_ERROR, __FUNCTION__);
				return nRet;
				//throw SocketException(WSA_ERROR, __FUNCTION__);
			}
			else {
				offset += nRet;
			}
		}
		message.append(reception, Msg_size);
		ReceivedBytes += Msg_size;
	}

	return nRet;
}

void BaseSocket::sendMsg(const std::string & message)
{
	if (this->sendMsg_noExcept(message) < 0) {
		throw SocketException(this->socket_err, TRACEBACK);
	}
}

void BaseSocket::recvMsg(std::string & message)
{
	if (this->recvMsg_noExcept(message) <= 0) {
		throw SocketException(this->socket_err, TRACEBACK);
	}
}

std::string BaseSocket::recvMsg()
{
	string message = "";
	this->recvMsg(message);
	return message;
}

