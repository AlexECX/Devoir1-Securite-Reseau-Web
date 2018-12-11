#pragma once
#include <string>
#include <stdio.h>
#include "WSA_Utils.h"


//Parse un string pour retrouver les information IP adress
std::string informationReseauIPAddress(std::string);

//Parse un string pour retrouver les informations du port
std::string informationReseauPort(std::string);

std::string& tolower_str(std::string &str);