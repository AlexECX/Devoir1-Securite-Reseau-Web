#pragma once
#include <iostream>
#include <map>
#include "encryption.h"
#include "base64.h"

using namespace std;


//Fonctions servant au cryptage /----------------------------------------/
string cbcEncrypt(const string& message, const string& key);
string cbcDecrypt(const string& message, const string& key);
void reseauFeistel(unsigned char* blockInput, unsigned char* raw_key, unsigned turn_count);
string vigenere(const unsigned char*, const unsigned char*, unsigned);
string xorSbox(const unsigned char*, const unsigned char*, unsigned);
string useSBox(const string& uc_msg, unsigned int len);

#define CHAIN_BLOCK 16	//octets (ou chars)
#define FEISTEL_BLOCK CHAIN_BLOCK / 2
#define FEISTEL_TURN 16
#define ALGO xorSbox

//Interface
string encrypt(const string& message, const string& key) {
	return cbcEncrypt(message, key);
}

string decrypt(const string& message, const string& key) {
	return string(cbcDecrypt(message, key).c_str());
}


//Algorithme de chiffrement par bloc. Utilise le mode d'operation CBC
//avec un réseau de Feistel.

string cbcEncrypt(const string& message, const string& key)
{

	unsigned char blockInput[CHAIN_BLOCK];
	unsigned char blockXOR[CHAIN_BLOCK];
	unsigned int bytes;
	string cryptogram = "";

	//pour s'assurer que la taille de la clé == taille d'un bloc,
	//la tronque si tros grosse, la complete avec des 0 si tros petite.
	unsigned char raw_key[FEISTEL_BLOCK];
	memset(raw_key, 0, FEISTEL_BLOCK);
	if (key.length() > FEISTEL_BLOCK) {
		memcpy(raw_key, key.c_str(), FEISTEL_BLOCK);
	}
	else {
		memcpy(raw_key, key.c_str(), key.length());
	}

	//Initialisation Vector
	memset(blockXOR, 0x02, CHAIN_BLOCK);

	unsigned int current_pos = 0;
	while (current_pos < message.length()) {
		//initialise chaque octects du block d'entrée a 0
		memset(blockInput, 0, CHAIN_BLOCK);

		if (message.length() - current_pos < CHAIN_BLOCK) {
			bytes = message.length() - current_pos;
		}
		else {
			bytes = CHAIN_BLOCK;
		}

		//copie n bytes a partir message dans le bloc d'entrée
		memcpy(blockInput, &message.c_str()[current_pos], bytes);
		current_pos += bytes;

		//xor avec entrée précédente
		for (unsigned i = 0; i < CHAIN_BLOCK; i++) {
			blockInput[i] ^= blockXOR[i];
		}

		//passe le bloc par le reseau de Feistel
		reseauFeistel(blockInput, raw_key, FEISTEL_TURN);

		//ajoute l'entrée crypté au cryptogramme
		cryptogram.append(reinterpret_cast<const char*>(blockInput), CHAIN_BLOCK);
		//l'entrée crypté sera utilisé pour le prochain xor
		memcpy(blockXOR, blockInput, CHAIN_BLOCK);
	}

	return cryptogram;
}

string cbcDecrypt(const string& message, const string& key)
{

	unsigned char blockInput[CHAIN_BLOCK];
	unsigned char blockXOR[CHAIN_BLOCK];
	unsigned char cpyInput[CHAIN_BLOCK];
	unsigned int bytes;
	string cryptogram = "";

	//pour s'assurer que la taille de la clé == taille d'un bloc,
	//la tronque si tros grosse, la complete avec des 0 si tros petite.
	unsigned char raw_key[FEISTEL_BLOCK];
	memset(raw_key, 0, FEISTEL_BLOCK);
	if (key.length() > FEISTEL_BLOCK) {
		memcpy(raw_key, key.c_str(), FEISTEL_BLOCK);
	}
	else {
		memcpy(raw_key, key.c_str(), key.length());
	}

	//Initialisation Vector
	memset(blockXOR, 0x02, CHAIN_BLOCK);

	unsigned int current_pos = 0;
	while (current_pos < message.length()) {
		//initialise chaque octects du block d'entrée a 0
		memset(blockInput, 0, CHAIN_BLOCK);

		if (message.length() - current_pos < CHAIN_BLOCK) {
			bytes = message.length() - current_pos;
		}
		else {
			bytes = CHAIN_BLOCK;
		}

		//copie n bytes a partir message dans le bloc d'entrée
		memcpy(blockInput, &message.c_str()[current_pos], bytes);
		current_pos += bytes;

		//garde une copie du message crypté, IV pour le prochain
		memcpy(cpyInput, blockInput, CHAIN_BLOCK);


		//passe le bloc par le reseau de Feistel
		reseauFeistel(blockInput, raw_key, FEISTEL_TURN);

		//xor avec entrée précédente
		for (unsigned i = 0; i < CHAIN_BLOCK; i++) {
			blockInput[i] ^= blockXOR[i];
		}

		//ajoute l'entrée crypté au cryptogramme
		cryptogram.append(reinterpret_cast<const char*>(blockInput), CHAIN_BLOCK);
		//l'entrée crypté sera utilisé pour le prochain xor
		memcpy(blockXOR, cpyInput, CHAIN_BLOCK);
	}

	return cryptogram;
}

void reseauFeistel(unsigned char* blockInput, unsigned char* raw_key, unsigned turn_count)
{

	unsigned char blockL[FEISTEL_BLOCK];
	unsigned char blockR[FEISTEL_BLOCK];

	//commence les tours de Feistel en utilisant le block d'entrée
	for (unsigned i = 0; i < turn_count; i++) {
		memset(blockL, 0, FEISTEL_BLOCK);
		memset(blockR, 0, FEISTEL_BLOCK);
		memcpy(blockL, blockInput, FEISTEL_BLOCK);
		memcpy(blockR, &blockInput[FEISTEL_BLOCK], FEISTEL_BLOCK);

		//R goes thru the F function
		string tmp = ALGO(blockR, raw_key, FEISTEL_BLOCK);
		memset(blockR, 0, FEISTEL_BLOCK);
		memcpy(blockR, tmp.c_str(), tmp.length());

		//unaltered R block becomes the next L block
		memcpy(blockInput, &blockInput[FEISTEL_BLOCK], FEISTEL_BLOCK);
		//xor L block with transformed R block and
		//put result in next iteration R block.
		for (unsigned i = 0; i < FEISTEL_BLOCK; i++) {
			blockInput[FEISTEL_BLOCK + i] = blockL[i] ^ blockR[i];
		}
	}
	//final switch
	memcpy(blockR, blockInput, FEISTEL_BLOCK);
	memcpy(blockInput, &blockInput[FEISTEL_BLOCK], FEISTEL_BLOCK);
	memcpy(&blockInput[FEISTEL_BLOCK], blockR, FEISTEL_BLOCK);

	return;
}


string vigenere(const unsigned char* block, const unsigned char* key,
	unsigned lenght
) {
	static char tableauVigenere[256][256];
	static bool iter = false;

	if (!iter) {
		int i;
		int j;
		char caractere = -128;


		for (i = 0; i < 256; i++)
		{
			for (j = 0; j < 256; j++)
			{

				tableauVigenere[i][j] = caractere;
				caractere++;
			}
			caractere++;
		}
	}


	string combined_block(lenght, 'x');

	for (unsigned i = 0; i < lenght; i++) {
		combined_block[i] = tableauVigenere[block[i]][key[i]];
	}

	return combined_block;

}

string xorSbox(const unsigned char* block, const unsigned char* key,
						unsigned lenght) 
{
	string combined_block(lenght, 'x');

	//xor le bloc avec la clé
	for (unsigned i = 0; i < lenght; i++) {
		combined_block[i] = block[i] ^ key[i];
	}

	return useSBox(combined_block, lenght);
	//return string(reinterpret_cast<const char*>(message), lenght);

}


//Basé sur une S-box de DES. Prend en entrée un char en base 64, qui représente
//un 6 bits, et le map a la valeur de 4 bits correspondante de la S-Box.
map<char, unsigned int> Sbox = {
		{'A', 0x02}, {'B', 0x240}, {'C', 0x04}, {'D', 0x01}, {'E', 0x07},
		{'F', 0x0A}, {'G', 0x0B}, {'H', 0x06}, {'I', 0x08}, {'J', 0x05},
		{'K', 0x03}, {'L', 0x0F}, {'M', 0x0D}, {'N', 0x00}, {'O', 0x0E},
		{'P', 0x09},
		//
		{'Q', 0x0E}, {'R', 0x0B}, {'S', 0x02}, {'T', 0x240}, {'U', 0x04},
		{'V', 0x07}, {'W', 0x0D}, {'X', 0x01}, {'Y', 0x05}, {'Z', 0x00},
		{'a', 0x0F}, {'b', 0x0A}, {'c', 0x03}, {'d', 0x09}, {'e', 0x08},
		{'f', 0x06},
		//
		{'g', 0x04}, {'h', 0x02}, {'i', 0x01}, {'j', 0x0B}, {'k', 0x0A},
		{'l', 0x0D}, {'m', 0x07}, {'n', 0x08}, {'o', 0x0F}, {'p', 0x09},
		{'q', 0x240}, {'r', 0x05}, {'s', 0x06}, {'t', 0x03}, {'u', 0x00},
		{'v', 0x0E},
		//
		{'w', 0x0B}, {'x', 0x08}, {'y', 0x240}, {'z', 0x07}, {'0', 0x01},
		{'1', 0x0E}, {'2', 0x02}, {'3', 0x0D}, {'4', 0x06}, {'5', 0x0F},
		{'6', 0x00}, {'7', 0x09}, {'8', 0x0A}, {'9', 0x04}, {'+', 0x05},
		{'/', 0x03},
		//		
		{'=', 0x00},
};

string useSBox(const string& msg, unsigned int len) {
	const unsigned char* uc_msg;
	uc_msg = reinterpret_cast<const unsigned char*>(msg.c_str());

	string msg64 = base64_encode(uc_msg, len); //<-internet code

	string crypto_block(msg64.length() / 2, 'x');
	unsigned long long bit8;
	for (string::size_type i = 0; i < msg64.length(); i += 2) {
		bit8 = Sbox[msg64[i]] << 4;
		bit8 += Sbox[msg64[i + 1]];
		crypto_block[i / 2] = static_cast<char>(bit8);
	}
	return crypto_block;
}

//-----------------------------/----------------------------------------/




//Fonctions servant au hashage /----------------------------------------/
void rotate_l(unsigned char *object, size_t size);
void rotate_r(unsigned char *object, size_t size);
string simpleHash(const string& message);
string simpleHMCA(const string& message, const string& key);

#define BLOCK_SIZE 128	//octets (ou chars)

//Interface
string generateMac(const string& message, const string& key) {
	return simpleHMCA(message, key);
}

bool verifyMAC(const string& str, const string& key) {
	string the_msg = str.substr(0, str.length() - BLOCK_SIZE);
	string the_MAC = str.substr(str.length() - BLOCK_SIZE, BLOCK_SIZE);

	return  the_MAC.compare(simpleHMCA(the_msg, key)) == 0;
}

string extractMsg(const string& str) {
	return string(str, 0, str.length() - BLOCK_SIZE);
}

//Traitement successif des données par un rotation des bits d'un bloc
//suivit d'un XOR avec le code de hashage (initialisé à 0).
string simpleHash(const string& message) {

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
		//copie n bytes a partir de raw_message[] dans le bloc
		memcpy(block, &raw_message[current_pos], bytes);
		current_pos += bytes;

		rotate_l(block, BLOCK_SIZE); //<-internet code

		for (unsigned i = 0; i < BLOCK_SIZE; i++) {
			result[i] ^= block[i];
		}

	}
	string str((char*)result, BLOCK_SIZE);
	return str;
}

//Implementation d'un HMAC
string simpleHMCA(const string& message, const string& key) {
	unsigned char key_plus[BLOCK_SIZE];
	memset(key_plus, 0, BLOCK_SIZE);
	if (key.length() < BLOCK_SIZE) {
		memcpy(key_plus, key.c_str(), key.length());
	}
	else
	{
		memcpy(key_plus, key.c_str(), BLOCK_SIZE);
	}

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

//-----------------------------/----------------------------------------/