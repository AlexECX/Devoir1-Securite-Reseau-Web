#pragma once
#include <string>


void initWSA();

std::string encrypt(std::string);

std::string encryptionAlgo(const unsigned char*, unsigned);

std::string reseauFeistel(
	std::string message,
	std::string(*cryptAlgo)(const unsigned char*, unsigned),
	unsigned turn_count
	);

std::string simpleHash(std::string message);

std::string simpleHMCA(std::string message, std::string key);

std::string extractMsg(std::string);

bool verifyMAC(std::string, std::string);