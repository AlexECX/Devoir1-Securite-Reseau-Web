#pragma once
#include <winsock.h>
#include <iostream>
#include <Utils.h>

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


void rotate_l(unsigned char *object, size_t size);
void rotate_r(unsigned char *object, size_t size);

#define BLOCK_SIZE 256

string extractMsg(string str) {
	return string(str, 0, str.length() - BLOCK_SIZE);
}

bool verifyMAC(string str, string key) {
	string the_msg = str.substr(0, str.length() - BLOCK_SIZE);
	string the_MAC = str.substr(str.length() - BLOCK_SIZE, BLOCK_SIZE);

	return  the_MAC.compare(simpleHMCA(the_msg, key)) == 0;
}

string simpleHMCA(string message, string key) {
	unsigned char key_plus[BLOCK_SIZE];
	memset(key_plus, 0, BLOCK_SIZE);
	memcpy(key_plus, key.c_str(), key.length());

	unsigned char Si[BLOCK_SIZE];
	memset(Si, 0, BLOCK_SIZE);
	for (unsigned i = 0; i < BLOCK_SIZE; i++) {
		Si[i] = key_plus[i] ^ 0x36;
	}

	string str_out = simpleHash(string((char*)Si, BLOCK_SIZE) + message);

	memset(Si, 0, BLOCK_SIZE);

	for (unsigned i = 0; i < BLOCK_SIZE; i++) {
		Si[i] = key_plus[i] ^ 0x5C;
	}

	str_out = simpleHash(string((char*)Si, BLOCK_SIZE) + str_out);

	return str_out;
}

string simpleHash(string message) {

	unsigned char* raw_message = (unsigned char*)message.c_str();
	unsigned char block[BLOCK_SIZE];
	unsigned char result[BLOCK_SIZE];
	memset(result, 0, BLOCK_SIZE); //initialise chaque octects a 0

	unsigned int current_pos = 0;
	unsigned int len = message.length();
	while (current_pos < len)
	{
		memset(block, 0, BLOCK_SIZE);
		unsigned int bytes = len - current_pos < BLOCK_SIZE ? len - current_pos : BLOCK_SIZE;
		//copie n bytes a partir de raw_message[] dans le bloque
		memcpy(block, &raw_message[current_pos], bytes);
		current_pos += bytes;

		rotate_l(block, BLOCK_SIZE);

		for (unsigned i = 0; i < BLOCK_SIZE; i++) {
			result[i] ^= block[i];
		}

	}
	string str((char*)result, BLOCK_SIZE);
	return str;
}

//Internet code start
void rotate_l(unsigned char *object, size_t size) {
	unsigned char *byte, *last;
	unsigned char firstBit;
	if (size != 0) {
		byte = object;
		last = byte + size - 1;
		firstBit = byte[0] >> (CHAR_BIT - 1);
		while (byte < last) {
			byte[0] = (byte[0] << 1) | (byte[1] >> (CHAR_BIT - 1));
			++byte;
		}
		(*last) = (*last) << 1 | firstBit;
	}
}

void rotate_r(unsigned char *object, size_t size) {
	unsigned char *byte, *end;
	unsigned char bit, nextBit;
	if (size != 0) {
		byte = object;
		end = byte + size;
		bit = *(end - 1) & 1;
		while (byte < end) {
			nextBit = byte[0] & 1;
			byte[0] = (byte[0] >> 1) | (bit << (CHAR_BIT - 1));
			++byte;
			bit = nextBit;
		}
	}
}

//Internet code end