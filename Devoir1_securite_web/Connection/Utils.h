#pragma once
#include <string>


void initWSA();

//Parse un string pour retrouver les information IP adress
std::string informationReseauIPAddress(std::string);

//Parse un string pour retrouver les informations du port
std::string informationReseauPort(std::string);
