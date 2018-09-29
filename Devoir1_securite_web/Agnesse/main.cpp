#include <winsock.h>
#include <iostream>
#include <string>
#include <windows.h>
#include <thread>
#include "ConnectionException.h"
#include "Utils.h"
#include "SimpleSocket.h"

using namespace std;

#define HARD_CODED
#define HOST "127.0.0.1" //Can be Name or IP address
#define PORT 2030

void runClient(short nPort, const char* host);
void scriptedConvo(SimpleSocket client, string& transit);
void initialisationTableauVigenere(char[256][256]);
void affichageTableauVigenere(char[256][256]);
string ouvertureFichierEnregistrementString(string);
void divisionEnBlocs15(string, string[100]);
void cryptageVigenereCBC(string[100], string[100], char[256][256], string);
string reconstitutionnerString(string[100], string);
void desencryptionVigenereCBC(string[100], string[100], char[256][256], string);

int main(int argc, char **argv)
{
	short nPort;
	const char* host;

	//
	// Check for the host and port arguments
	//
#ifdef HARD_CODED
	nPort = PORT;
	host = HOST;
#else
	if (argc != 3) {
		cout << "\nSyntax: ServerName PortNumber";
		cout << endl;
		return 0;
	}
	nPort = atoi(argv[2]);
	host = argv[1];

#endif // HARD_CODED

	initWSA();

	runClient(nPort, host);

	// Release WinSock
	//
	WSACleanup();
	system("pause");
	return 0;

}

void runClient(short nPort, const char * host)
{
	
	try
	{
		string transit = "";
		bool connected = false;
		SimpleSocket client = SimpleSocket();

		while (!connected) {
			try
			{
				cout << "\nconnecting to " + string(host) + " on port " << nPort;
				client.connectSocket(host, nPort);
				connected = true;
			}
			catch (const ConnectionException e)
			{
				cout << endl << e.what();
				this_thread::sleep_for(5s);
			}
		}

		scriptedConvo(client, transit);

		}
	catch (const ConnectionException e)
	{
		cout << endl << e.what();
	}
	

}

void scriptedConvo(SimpleSocket client, string& transit) {
	
	//Variables
	string agnesse_key = "gadgettegdag";
	string cryptogram = "";
	string message = "";
	char tableauVigenere[256][256];	
	string tableauBlocMessageClair[100];
	string tableauBlocMessageCrypt[100];
	int i = 0;
	int k = 0;
	   
	//Initialisation tableau d'encryption
	initialisationTableauVigenere(tableauVigenere);

	cout << "\nQuel est le message d'Agnesse?" << endl;
	getline(cin, transit);
	cout << endl;
	cout << "Message a etre transmit (clair) a Clement pour Bob: \n";
	cout << transit << endl << endl;

	//Encryption
	divisionEnBlocs15(transit, tableauBlocMessageClair);
	cryptageVigenereCBC(tableauBlocMessageClair, tableauBlocMessageCrypt, tableauVigenere, agnesse_key);
	cryptogram = reconstitutionnerString(tableauBlocMessageCrypt, "crypt");

	cout << endl;
	cout << "\nMessage a etre transmit (crypt) a Clement pour Bob: \n\n";
	cout << cryptogram << endl << endl;

	//Envoie du message
	client.sendMessage(cryptogram);
	cout << "\nwaiting for Bob" << endl;

	//Réception
	client.recvMessage(transit);
	cout << "Message crypte recu de Clement (provenance Bob)\n" << endl;
	cout << transit << endl;

	//Désencryption
	cryptogram = transit;
	divisionEnBlocs15(cryptogram, tableauBlocMessageCrypt);
	desencryptionVigenereCBC(tableauBlocMessageClair, tableauBlocMessageCrypt, tableauVigenere, agnesse_key);
	message = reconstitutionnerString(tableauBlocMessageClair, "clair");
	cout << "Message clair recu de Clement (provenance Bob)\n" << endl;
	cout << message << endl;


	/**
	transit = "Hello from Agnesse";
	cout << "\nsending \"" << transit << "\" to Bob";
	transit = encrypt(transit, agnesse_key);
	cout << "\nsending \"" << transit << "\" to Bob";
	string mac = getMac(transit, agnesse_key);
	cout << "\n The MAC is " << mac;

	client.sendMessage(transit + mac);
	cout << "\nwaiting for Bob";
	client.recvMessage(transit);
	transit = extractMsg(transit);
	cout << "\nreceived \"" << transit << "\"";
	transit = decrypt(transit, agnesse_key);
	cout << "\nreceived \"" << transit << "\"";
	cout << "\nthe verifyMAC result is: " << ((verifyMAC(transit, agnesse_key) == true) ? "True" : "False");
	/**/
}

void initialisationTableauVigenere(char tableauVigenere[256][256])
{
	int i;
	int j;
	char caractere = -128;

	/*
	if (type == "minuscules")
	{
		caractere = 97;
	}
	else
	{
		caractere = 65;
	}
	*/

	for (i = 0; i < 256; i++)
	{
		for (j = 0; j < 256; j++)
		{

			tableauVigenere[i][j] = caractere;
			caractere++;
		}
		caractere++;
	}
};
void affichageTableauVigenere(char tableauVigenere[256][256])
{
	int i;
	int j;

	for (i = 0; i < 256; i++)
	{
		for (j = 0; j < 256; j++)
		{
			cout << tableauVigenere[i][j];
			cout << " ";
		}
		cout << endl;
	}
	cout << endl;
};
string ouvertureFichierEnregistrementString(string nomFichier)
{
	ifstream file;
	string contenuFichier;
	char c;

	c = 0;

	file.open(nomFichier, ios::in);
	if (file.fail())
	{
		cout << "Erreur d'ouverture de fichier." << endl;
	}
	else
	{
		while (!file.eof())
		{
			c = file.get();
			contenuFichier = contenuFichier + c;
			//getline(file, contenuFichier);
		}
	}
	return contenuFichier;
};
void divisionEnBlocs15(string texte, string tableauBlocs[100])
{
	int i;
	int j;
	int k;
	string bloc = "";

	i = 0;
	j = 0;
	k = 0;

	for (i = 0; i < texte.size(); i++)
	{
		bloc = bloc + texte[i];
		j++;
		if (j == 15)
		{
			tableauBlocs[k] = bloc;
			k++;
			bloc.clear();
			j = 0;
		}
	}

	//Ajout grandeur de texte ne se divise pas sans reste par 15
	if ((texte.size() % 15) != 0)
	{
		while (j < 15)
		{
			bloc = bloc + '%';
			j++;
		}
		tableauBlocs[k] = bloc;
		k++;
		bloc.clear();
		j = 0;

	}

	//Marque de fin du message dans le dernier bloc
	tableauBlocs[k] = "\n";
};
void cryptageVigenereCBC(string tableauBlocsClair[100], string tableauBlocsCryp[100], char tableauVigenere[256][256], string clef)
{
	const string valeurXORDépart = "123456789012345";
	string blocCourant = "";
	string blocXOR = "";
	string blocCrypt = "";
	int charXORInt = ' ';
	int charClefInt = ' ';
	int charCryptInt = ' ';
	int i = 0;
	int j = 0;
	int k = 0;

	//Bloc par bloc
	while (tableauBlocsClair[i] != "\n")
	{
		blocCourant = tableauBlocsClair[i];

		//XOR operation
		//Pour la premiere itération, utilisation de la valeur de départ pour le XOR
		if (i == 0)
		{
			for (j = 0; j < blocCourant.size(); j++)
			{
				blocXOR.push_back(blocCourant[j] ^ valeurXORDépart[j]);
			}
		}
		//Pour les autres itérations, utilisation du bloc crypté précédent pour le XOR
		else
		{
			//blocCrypt = tableauBlocsCryp[i - 1];
			for (j = 0; j < blocCourant.size(); j++)
			{
				blocXOR.push_back(blocCourant[j] ^ blocCrypt[j]);
			}
			blocCrypt.clear();
		}


		//Encryption lettre par lettre
		for (j = 0; j < blocXOR.size(); j++)
		{
			charXORInt = 128 + blocXOR[j];
			//Si on dépasse la grandeur de la clé, on retourne à son premier caractere
			if (j == clef.size())
			{
				k = 0;
			}
			charClefInt = clef[k];
			k++;
			//Encryption de la lettre
			charCryptInt = tableauVigenere[charXORInt][charClefInt];
			blocCrypt.push_back(charCryptInt);
		}
		k = 0;
		blocXOR.clear();
		//Insertion en bloc crypté dans le tableau
		tableauBlocsCryp[i] = blocCrypt;



		//Encryption de base pour essai
		/*
		charXOR = 128 + blocXOR[0];
		charClef = clef[0];
		charCrypt = tableauVigenere[charXOR][charClef];

		charClef = clef[0];
		//charCrypt = blocCrypt[0];
		for (k = 0; k < 255; k++)
		{
			if (tableauVigenere[charClef][k] == charCrypt)
			{
				charXOR = k;
				break;
			}
		}

		//Reverse XOR operation
		charXOR = charXOR - 128;
		charClair = charXOR ^ valeurXORDépart[0];
		cout << charClair;
		*/

		i++;
	}
	//On marque la fin du message
	tableauBlocsCryp[i] = "\n";
};
string reconstitutionnerString(string tableauBlocs[100], string type)
{
	int i = 0;
	string message = "";

	if (type == "crypt")
	{
		while (tableauBlocs[i] != "\n")
		{
			message = message + tableauBlocs[i];
			i++;
		}
	}
	else
		if (type == "clair")
		{
			while (tableauBlocs[i] != "\n")
			{
				message = message + tableauBlocs[i];
				i++;
			}

			//Enlèvement du padding en partant de la fin du message
			i = message.size() - 1;
			while (message[i] == '%')
			{
				message.pop_back();
				i--;
			}
		}
		else
		{
			cout << "Erreur type fonction reconstirutionnerString.";
			cout << endl;
		}


	return message;
};
void desencryptionVigenereCBC(string tableauBlocsClair[100], string tableauBlocsCryp[100], char tableauVigenere[256][256], string clef)
{
	const string valeurXORDépart = "123456789012345";
	string blocClair = "";
	string blocXOR = "";
	string blocCrypt = "";
	char charClair = ' ';
	int charClairInt = ' ';
	int charXORInt = ' ';
	int charClefInt = ' ';
	int charCryptInt = ' ';
	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;

	//Bloc par Bloc
	while (tableauBlocsCryp[i] != "\n")
	{
		blocCrypt = tableauBlocsCryp[i];

		//Lettre par lettre
		for (j = 0; j < 15; j++)
		{
			//Si on arrive au dernier caractere de la clé, on la remet au caractere 0
			if (j == clef.size())
			{
				k = 0;
			}
			charClefInt = clef[k];
			k++;
			charCryptInt = blocCrypt[j];

			//Recherche dans le tableau de vigenere pour trouver un caractere décodé
			for (l = 0; l < 255; l++)
			{
				if (tableauVigenere[charClefInt][l] == charCryptInt)
				{
					charXORInt = l;
					break;
				}
			}

			//Reverse XOR operation
			charXORInt = charXORInt - 128;
			if (i == 0)
			{
				charClairInt = charXORInt ^ valeurXORDépart[j];
			}
			else
			{
				charClairInt = charXORInt ^ tableauBlocsCryp[i - 1][j];
			}
			//Construction du bloc message clair
			charClair = charClairInt;
			blocClair.push_back(charClair);
		}
		k = 0;
		tableauBlocsClair[i] = blocClair;
		blocClair.clear();
		i++;
	}
	tableauBlocsClair[i] = "\n";
};
