#pragma once
#include <string>


void initWSA();

std::string encrypt(const std::string&, const std::string&);

std::string decrypt(const std::string&, const std::string&);

std::string reseauFeistel(
	const std::string& message,
	const std::string& key,
	std::string(*cryptAlgo)(const unsigned char*, const unsigned char*, unsigned),
	unsigned turn_count
	);

std::string encryptionAlgo(const unsigned char*, const unsigned char*, unsigned);

std::string simpleHash(const std::string& message);

std::string simpleHMCA(const std::string& message, const std::string& key);

std::string extractMsg(const std::string&);

std::string getMac(const std::string& message, const std::string& key);

bool verifyMAC(const std::string&, const std::string&);