#pragma once
#include <string>
#include <stdio.h>


void initWSA();

std::string formateWSAError(int error_code);

//Parse un string pour retrouver les information IP adress
std::string informationReseauIPAddress(std::string);

//Parse un string pour retrouver les informations du port
std::string informationReseauPort(std::string);

std::string& tolower_str(std::string &str);