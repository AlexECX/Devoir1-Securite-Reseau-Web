#pragma once
#include <string>


void initWSA();

std::string simpleHash(std::string message);

std::string simpleHMCA(std::string message, std::string key);

std::string extractMsg(std::string);

bool verifyMAC(std::string, std::string);