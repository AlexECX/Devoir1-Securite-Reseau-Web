#pragma once
#include <winsock.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include "Utils.h"
#include <map>

using namespace std;


string informationReseauIPAddress(std::string message)
{
	int i = 0;
	string IP = "";

	while (message[i] != ':')
	{
		IP += message[i];
		i++;
	}

	return IP;
}

string informationReseauPort(std::string message)
{
	unsigned i = 0;
	string port = "";

	while (message[i] != ':')
	{
		i++;
	}
	i++;
	while (i < message.size())
	{
		port += message[i];
		i++;
	}

	return port;
}

std::string& tolower_str(std::string &str)
{
	for (auto &ch : str)
	{
		ch = tolower(ch);
	}
	return str;
}
