#include <winsock.h>
#include <iostream>
#include <string>
#include <thread>
#include <windows.h>
#include "ConnectionException.h"
#include "SimpleServerSocket.h"
#include "Utils.h"

using namespace std;

#define HARD_CODED
#define PORT 2030

void runServer(short nPort);
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

	//
	// Check for port argument
	//
#ifdef HARD_CODED
	nPort = PORT;
#else
	if (argc != 2) {
		cout << "\nSyntax: PortNumber"; 
		cout << endl;
		return 0;
	}
	nPort = atoi(argv[1]);

#endif // HARD_CODED

	initWSA();

	runServer(nPort);

	// Release WinSock
	//
	WSACleanup();
	system("pause");
	return 0;
}

void runServer(short nPort)
{
	string transit = "";
	string message = "";
	string cryptogram = "";
	char tableauVigenere[256][256];
	string tableauBlocMessageClair[100];
	string tableauBlocMessageCrypt[100];
	int i = 0;
	int k = 0;

	try
	{
		SimpleServerSocket server = SimpleServerSocket(nPort);
		SimpleSocket s1 = server.acceptSocket();
		SimpleSocket s2 = server.acceptSocket();

		string bob_key = "bobinoonibob";
		string agnesse_key = "gadgettegdag";

		//Initialisation de la table de Videnere
		initialisationTableauVigenere(tableauVigenere);

		//Réception de Agnesse
		s1.recvMessage(transit);
		cout << "Crypted message received from Agnesse for Bob: \n" << endl;
		cout << transit << endl << endl;
		
		//Désencryption d'Agnesse
		cryptogram = transit;
		divisionEnBlocs15(cryptogram, tableauBlocMessageCrypt);
		desencryptionVigenereCBC(tableauBlocMessageClair, tableauBlocMessageCrypt, tableauVigenere, agnesse_key);
		message = reconstitutionnerString(tableauBlocMessageClair, "clair");

		cout << "Clear message received from Agnesse for Bob: \n" << endl;
		cout << message << endl << endl;

		//Encryption pour Bob
		divisionEnBlocs15(message, tableauBlocMessageClair);
		cryptageVigenereCBC(tableauBlocMessageClair, tableauBlocMessageCrypt, tableauVigenere, bob_key);
		cryptogram = reconstitutionnerString(tableauBlocMessageCrypt, "crypt");
		cout << "Message a etre transmit (crypt) a Bob de Agnesse: \n\n";
		cout << cryptogram << endl << endl;

		//Envoie à Bob
		transit = cryptogram;
		s2.sendMessage(transit);

		//Réception de Bob
		s2.recvMessage(transit);
		cout << "Crypted message received from Bob for Agnesse: \n" << endl;
		cout << transit << endl << endl;

		//Désencryption de Bob
		cryptogram = transit;
		divisionEnBlocs15(cryptogram, tableauBlocMessageCrypt);
		desencryptionVigenereCBC(tableauBlocMessageClair, tableauBlocMessageCrypt, tableauVigenere, bob_key);
		message = reconstitutionnerString(tableauBlocMessageClair, "clair");

		cout << "Clear message received from Bob for Agnesse: \n" << endl;
		cout << message << endl << endl;

		//Encryption pour Bob
		divisionEnBlocs15(message, tableauBlocMessageClair);
		cryptageVigenereCBC(tableauBlocMessageClair, tableauBlocMessageCrypt, tableauVigenere, agnesse_key);
		cryptogram = reconstitutionnerString(tableauBlocMessageCrypt, "crypt");
		cout << "\nMessage a etre transmit (crypt) a Agnesse de Bob: \n\n";
		cout << cryptogram << endl << endl;

		//Envoie à Agnesse
		transit = cryptogram;
		s1.sendMessage(transit);



















		/**
		s1.recvMessage(transit);
		if (verifyMAC(transit, agnesse_key)) {
			message = extractMsg(transit);
			cryptogram = encrypt(decrypt(message, agnesse_key), bob_key);
			s2.sendMessage(cryptogram + getMac(transit, bob_key));
		}
		else
		{
			cryptogram = encrypt("MAC error from Clement", agnesse_key);
			s1.sendMessage(cryptogram + getMac(transit, agnesse_key));
			cryptogram = encrypt("Agnesse MAC was refused", bob_key);
			s2.sendMessage(cryptogram + getMac(transit, bob_key));
		}

		s2.recvMessage(transit);
		if (verifyMAC(transit, bob_key)) {
			transit = extractMsg(transit);
			cryptogram = encrypt(decrypt(transit, bob_key), agnesse_key);
			s1.sendMessage(cryptogram + getMac(transit, agnesse_key));
		}
		else
		{
			transit = "Bob MAC was refused";
			cryptogram = encrypt(transit, bob_key);
			s1.sendMessage(cryptogram + getMac(transit, agnesse_key));
		}		
		/**/
		
	}
	catch (const ConnectionException e)
	{
		cout << endl << e.what();
	}
	
	
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
