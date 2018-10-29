#pragma once
#include <iostream>
#include <map>
#include <random>
#include <functional>
#include "encryption.h"
#include "base64.h"

using namespace std;


//Fonctions servant au cryptage /----------------------------------------/
string cbcEncrypt(const string& message, const string& key);
string cbcDecrypt(const string& message, const string& key);
void reseauFeistel(unsigned char* blockInput, const string& key, unsigned turn_count);
void vigenere(unsigned char*, const unsigned char*, unsigned);
void xorSbox(unsigned char*, const unsigned char*, unsigned);
void useSBox(unsigned char* msg, unsigned int len);

#define CHAIN_BLOCK 16	//octets (ou chars)
#define FEISTEL_BLOCK CHAIN_BLOCK / 2
#define FEISTEL_TURN 16
#define ALGO vigenere

//Interfaces
string generateKey(unsigned size)
{
	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution(0, 1000);
	auto init = std::bind(distribution, generator);

	string key(init(), size);
	for (size_t i = 0; i < size; i++)
	{
		key[i] ^= init();
	}

	return key;
}


string encrypt(const string& message, const string& key) {
	
	return cbcEncrypt(message, key);

}

string decrypt(const string& message, const string& key) {

	return cbcDecrypt(message, key);
}


//Algorithme de chiffrement par bloc. Utilise le mode d'operation CBC
//avec un r�seau de Feistel et une fonction F au choix (#define ALGO).

string cbcEncrypt(const string& message, const string& key)
{

	unsigned char blockInput[CHAIN_BLOCK];
	unsigned char blockXOR[CHAIN_BLOCK];
	unsigned int bytes;
	string cryptogram = "";

	//Initialisation Vector
	memset(blockXOR, 0x02, CHAIN_BLOCK);

	unsigned block_count = message.length() / CHAIN_BLOCK;
	unsigned residue = message.length() % CHAIN_BLOCK;

	cout << "\n****************************************************** \n" << endl;
	cout << "FONCTION 'cbcEncrypt': \n\n" << endl;
	cout << "Encryption du message:\n" << message << "\n" << endl;
	cout << "Avec la cle:\n" << key << "\n" << endl;

	for (unsigned i = 0; i <= block_count; i++) {

		if (i == block_count) { //si est le dernier bloc
			//methode de padding PKCS5

			//si input < que taille bloc, ajoute N bytes de padding de valeur N,
			//où N = le nombre de bytes pour completer le bloc.
			if (residue) { 
				memset(blockInput, CHAIN_BLOCK - residue, CHAIN_BLOCK);
				bytes = residue;
				//copie n bytes a partir du message dans le bloc d'entr�e
				memcpy(blockInput, &message.c_str()[i * CHAIN_BLOCK], bytes);
}			
			//sinon, le message est un facteur de taille bloc. 
			//On ajoute donc un bloc complet de padding où N = la taille d'un bloc.
			else {
				memset(blockInput, CHAIN_BLOCK, CHAIN_BLOCK);
				}
		}
		else {
			//initialise chaque octects du block d'entr�e a 0
			memset(blockInput, 0, CHAIN_BLOCK);
			bytes = CHAIN_BLOCK;
			//copie n bytes a partir du message dans le bloc d'entr�e
			memcpy(blockInput, &message.c_str()[i * CHAIN_BLOCK], bytes);
		}

		
		//xor avec entr�e pr�c�dente
		for (unsigned i = 0; i < CHAIN_BLOCK; i++) {
			blockInput[i] ^= blockXOR[i];
		}

		//passe le bloc par le reseau de Feistel
		reseauFeistel(blockInput, key, FEISTEL_TURN);

		//ajoute l'entr�e crypt� au cryptogramme
		cryptogram.append(reinterpret_cast<const char*>(blockInput), CHAIN_BLOCK);
		//l'entr�e crypt� sera utilis� pour le prochain xor
		memcpy(blockXOR, blockInput, CHAIN_BLOCK);
	}
	
	cout << "Cryptogramme:\n" << cryptogram << "\n" << endl;
	cout << "****************************************************** \n" << endl;
	return cryptogram;
}

string cbcDecrypt(const string& message, const string& key)
{

	unsigned char blockInput[CHAIN_BLOCK];
	unsigned char blockXOR[CHAIN_BLOCK];
	unsigned char cpyInput[CHAIN_BLOCK];
	unsigned int bytes;
	string cryptogram = "";

	//Initialisation Vector
	memset(blockXOR, 0x02, CHAIN_BLOCK);

	cout << "\n****************************************************** \n" << endl;
	cout << "FONCTION 'cbcDecrypt': \n\n" << endl;
	cout << "Desencryption du message:\n" << message << "\n" << endl;
	cout << "Avec la cle:\n" << key << "\n" << endl;

	unsigned block_count = message.length() / CHAIN_BLOCK;
	for (unsigned i = 0; i < block_count; i++) {

		//initialise chaque octects du block d'entr�e a 0
		memset(blockInput, 0, CHAIN_BLOCK);
		bytes = CHAIN_BLOCK;
		//copie n bytes a partir du message dans le bloc d'entr�e
		memcpy(blockInput, &message.c_str()[i * CHAIN_BLOCK], bytes);
	

		//garde une copie du message crypt�, IV pour le prochain
		memcpy(cpyInput, blockInput, CHAIN_BLOCK);

		//passe le bloc par le reseau de Feistel
		reseauFeistel(blockInput, key, FEISTEL_TURN);

		//xor avec entr�e pr�c�dente
		for (unsigned i = 0; i < CHAIN_BLOCK; i++) {
			blockInput[i] ^= blockXOR[i];
		}

		//ajoute l'entr�e crypt� au cryptogramme
		cryptogram.append(reinterpret_cast<const char*>(blockInput), CHAIN_BLOCK);
		//l'entr�e crypt� sera utilis� pour le prochain xor
		memcpy(blockXOR, cpyInput, CHAIN_BLOCK);
	}
	//Le cryptogram contient les octets de "padding" utilise lors de l'encryption. Cette
	//operation retranche ces octets.
	cryptogram = cryptogram.substr(0, cryptogram.length() - int(cryptogram.back()));

	cout << "Message desencrypte:\n" << cryptogram << "\n" << endl;
	cout << "****************************************************** \n" << endl;
	return cryptogram;
}

void reseauFeistel(unsigned char* blockInput, const string& key, unsigned turn_count)
{

	unsigned char blockL[FEISTEL_BLOCK];
	unsigned char blockR[FEISTEL_BLOCK];
	unsigned char raw_key[FEISTEL_BLOCK];

	//Pour s'assurer d'avoir une clé de la taille d'un bloc
	memset(raw_key, 0, FEISTEL_BLOCK);
	if (key.length() > FEISTEL_BLOCK) {
		memcpy(raw_key, key.c_str(), FEISTEL_BLOCK);
	}
	else {
		memcpy(raw_key, key.c_str(), key.length());
	}

	//commence les tours de Feistel en utilisant le block d'entr�e
	for (unsigned i = 0; i < turn_count; i++) {
		memcpy(blockL, blockInput, FEISTEL_BLOCK);
		memcpy(blockR, &blockInput[FEISTEL_BLOCK], FEISTEL_BLOCK);

		//R goes thru the F function
		ALGO(blockR, raw_key, FEISTEL_BLOCK);

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


void vigenere(unsigned char* block, const unsigned char* key,
		  unsigned lenght)
{
	static char tableauVigenere[256][256];
	static bool first_iter = true;

	if (first_iter) {
		first_iter = false;
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

	for (unsigned i = 0; i < lenght; i++) {
		block[i] = tableauVigenere[block[i]][key[i]];
	}

	return;

}

void xorSbox(unsigned char* block, const unsigned char* key,
						unsigned lenght) 
{
	//xor le bloc avec la cl�
	for (unsigned i = 0; i < lenght; i++) {
		block[i] = block[i] ^ key[i];
	}

	useSBox(block, lenght);
	return;
	//return string(reinterpret_cast<const char*>(message), lenght);

}


void useSBox(unsigned char* msg, unsigned int len) {

	//Bas� sur une S-box de DES. Prend en entr�e un char en base 64, qui repr�sente
	//un 6 bits, et le map a la valeur de 4 bits correspondante de la S-Box.
	static map<char, unsigned int> Sbox = {
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

	string msg64 = base64_encode(msg, len); //<-internet code

	string crypto_block(msg64.length() / 2, 'x');
	unsigned long long bit8;
	for (string::size_type i = 0; i < msg64.length(); i += 2) {
		bit8 = Sbox[msg64[i]] << 4;
		bit8 += Sbox[msg64[i + 1]];
		crypto_block[i / 2] = static_cast<char>(bit8);
	}
	memcpy(msg, base64_decode(crypto_block).c_str(), //<-internet code
			len); 
}

//-----------------------------/----------------------------------------/




//Fonctions servant au hashage /----------------------------------------/
void rotate_l(unsigned char *object, size_t size);
void rotate_r(unsigned char *object, size_t size);
string simpleHash(const string& message);
string simpleHMCA(const string& message, const string& key);

#define BLOCK_SIZE 32	//octets (ou chars)

//Interfaces//

//Permet de vérifier que le message est bien celui attendu et qu'il 
//provient de la bonne personne. Utiliser pour différencer A de B lors
//de l'assignation de SimpleSockets.
bool authenticate(string msg, string name, string mac_key)
{
	if (extractMsg(msg).compare(name) == 0
		&& verifyMAC(msg, mac_key))
	{
		return true;
	}
	else {
		return false;
	}
}

string generateMac(const string& message, const string& key) {

	return simpleHMCA(message, key);
}

bool verifyMAC(const string& str, const string& key) {
	try
	{
		string the_msg = str.substr(0, str.length() - BLOCK_SIZE);
		string the_MAC = str.substr(str.length() - BLOCK_SIZE, BLOCK_SIZE);

		return  the_MAC.compare(simpleHMCA(the_msg, key)) == 0;
	}
	catch (const std::out_of_range& e)
	{
		cout << endl << e.what();
		return false;
	}
}

string extractMsg(const string& str) {
	return string(str, 0, str.length() - BLOCK_SIZE);
}

//Traitement successif des donn�es par un rotation des bits d'un bloc
//suivit d'un XOR avec le code de hashage (initialis� � 0).
string simpleHash(const string& message) {

	unsigned char block[BLOCK_SIZE];
	string result(BLOCK_SIZE, '\0'); //initialise chaque octects a 0
	unsigned int bytes;

	unsigned int current_pos = 0;
	while (current_pos < message.length())
	{
		memset(block, 0, BLOCK_SIZE);
		if (message.length() - current_pos < BLOCK_SIZE){
			bytes = message.length() - current_pos;
		}
		else {
			bytes = BLOCK_SIZE;
		}
		//copie n bytes a partir de raw_message[] dans le bloc
		memcpy(block, &message.c_str()[current_pos], bytes);
		current_pos += bytes;

		rotate_l(block, BLOCK_SIZE); //<-internet code

		for (unsigned i = 0; i < BLOCK_SIZE; i++) {
			result[i] ^= block[i];
		}

	}
	return result;
}

//Implementation d'un HMAC
string simpleHMCA(const string& message, const string& key) {

	cout << "\n******************************** \n" << endl;
	cout << "FONCTION HMAC: \n\n" << endl;
	cout << "Entree:\n" << message << "\n" << endl;
	cout << "Avec la cle:\n" << key << "\n" << endl;

	unsigned char key_plus[BLOCK_SIZE];
	memset(key_plus, 0, BLOCK_SIZE);
	if (key.length() < BLOCK_SIZE) {
		memcpy(key_plus, key.c_str(), key.length());
	}
	else
	{
		memcpy(key_plus, key.c_str(), BLOCK_SIZE);
	}

	string Si(BLOCK_SIZE, '\0');
	for (unsigned i = 0; i < BLOCK_SIZE; i++) {
		Si[i] = key_plus[i] ^ 0x36;
	}

	string str_out = simpleHash(Si + message);

	Si.assign(BLOCK_SIZE, '\0');

	for (unsigned i = 0; i < BLOCK_SIZE; i++) {
		Si[i] = key_plus[i] ^ 0x5C;
	}

	str_out = simpleHash(Si + str_out);

	cout << "String de sortie:\n" << str_out << "\n" << endl;
	cout << "******************************** \n" << endl;

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