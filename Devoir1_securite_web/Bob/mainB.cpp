#include <winsock.h>
#include <iostream>
#include <string>
#include <windows.h>
#include <thread>
#include "ConnectionException.h"
#include "Utils.h"
#include "SimpleSocket.h"
#include "encryption.h"

using namespace std;

#define HARD_CODED
#define HOST "127.0.0.1" //Can be Name or IP address
#define PORT 2030

void runClient(short nPort, const char* host);
void scriptedConvo(SimpleSocket client, string& transit);
void scriptedConvoTest(SimpleSocket client);
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
		cout << "\nSyntax: ServerName PortNumber"; //test
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
			catch (const ConnectionException& e)
			{
				cout << endl << e.what();
				this_thread::sleep_for(5s);
			}

		}
		
		scriptedConvoTest(client);
	}
	catch (const ConnectionException& e)
	{
		cout << endl << e.what();
	}
	

}

void scriptedConvoTest(SimpleSocket client) {
	string bob_key = "bobinoonibob";
	string message = "";
	string mac = "";

	//R�ception
	cout << "\nwaiting for Agnesse";
	client.recvMessage(message);
	cout << "\n\nMessage recu (crypted)de Clement provenance Agnesse:\n";
	cout << "\"" << message << "\"" << endl << endl;

	//D�sencryption
	cout << "\nMessage clair recu de Clement (provenance Agnesse)\n\n" << endl;
	cout << "\"" << decrypt(extractMsg(message), bob_key) << "\"" << endl;

	//V�rif MAC
	cout << "\nthe verifyMAC result is: "
		<< (verifyMAC(message, bob_key) == true ? "True" : "False");

	//Message de Bob pour Agnesse
	cout << "\n\nQuel est le message de Bob?" << endl;
	getline(cin, message);
	cout << endl << endl;
	cout << "Message a etre transmit (clair) a Clement pour Agnesse: \n";
	cout << "\"" << message << "\"" << endl << endl;

	//Encryption
	message = encrypt(message, bob_key);

	//Envoie du message
	cout << endl << endl;
	cout << "\nMessage a etre transmit (crypt) a Clement pour Agnesse: \n\n";
	cout << "\"" << message << "\"" << endl << endl;
	client.sendMessage(message + generateMac(message, bob_key));

}

void scriptedConvo(SimpleSocket client, string& transit) {
	
	//Variables
	string bob_key = "bobinoonibob";
	string cryptogram = "";
	string message = "";
	char tableauVigenere[256][256];
	string tableauBlocMessageClair[100];
	string tableauBlocMessageCrypt[100];
	int i = 0;
	int k = 0;

	//Initialisation tableau d'encryption
	initialisationTableauVigenere(tableauVigenere);

	//R�ception
	cout << "\nwaiting for Agnesse";
	client.recvMessage(transit);
	cout << "\n\nMessage recu (crypted)de Clement provenance Agnesse:\n";
	cout << transit << endl;

	//D�sencryption
	cryptogram = transit;
	divisionEnBlocs15(cryptogram, tableauBlocMessageCrypt);
	desencryptionVigenereCBC(tableauBlocMessageClair, tableauBlocMessageCrypt, tableauVigenere, bob_key);
	message = reconstitutionnerString(tableauBlocMessageClair, "clair");
	cout << "\nMessage clair recu de Clement (provenance Agnesse)\n\n" << endl;
	cout << message << endl;

	//Message de Bob pour Agnesse
	cout << "\n\nQuel est le message de Bob?" << endl;
	getline(cin, transit);
	cout << endl << endl;
	cout << "Message a etre transmit (clair) a Clement pour Agnesse: \n";
	cout << transit <<endl;

	//Encryption
	divisionEnBlocs15(transit, tableauBlocMessageClair);
	cryptageVigenereCBC(tableauBlocMessageClair, tableauBlocMessageCrypt, tableauVigenere, bob_key);
	cryptogram = reconstitutionnerString(tableauBlocMessageCrypt, "crypt");

	//Envoie du message
	cout << endl << endl;
	cout << "\nMessage a etre transmit (crypt) a Clement pour Agnesse: \n\n";
	cout << cryptogram << endl;
	client.sendMessage(cryptogram);

	/**
	cout << "\nwaiting for Agnesse";
	client.recvMessage(transit);
	message = extractMsg(transit);
	cout << "\nreceived \"" << message << "\"";
	cout << "\nreceived \"" << decrypt(message, bob_key) << "\"";

	bool mac_result = verifyMAC(transit, bob_key);
	cout << "\nthe verifyMAC result is: " << (mac_result == true ? "True" : "False");

	if (mac_result == true) {
		transit = "Hello from Bob";
		cout << "\nsending \"" << transit << "\" to Agnesse";
		transit = encrypt(transit, bob_key);
		cout << "\nsending \"" << transit << "\" to Agnesse";
		
		string mac = getMac(transit, bob_key);
		cout << "\n The MAC is " << mac;
		client.sendMessage(transit + mac);
	}
	else
	{
		message = "MAC error from Bob";
		cout << "\nsending \"" << message << "\"";
		transit = encrypt(message, bob_key);
		client.sendMessage(transit + getMac(message, bob_key));
	}
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
	const string valeurXORD�part = "123456789012345";
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
		//Pour la premiere it�ration, utilisation de la valeur de d�part pour le XOR
		if (i == 0)
		{
			for (j = 0; j < blocCourant.size(); j++)
			{
				blocXOR.push_back(blocCourant[j] ^ valeurXORD�part[j]);
			}
		}
		//Pour les autres it�rations, utilisation du bloc crypt� pr�c�dent pour le XOR
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
			//Si on d�passe la grandeur de la cl�, on retourne � son premier caractere
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
		//Insertion en bloc crypt� dans le tableau
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
		charClair = charXOR ^ valeurXORD�part[0];
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

			//Enl�vement du padding en partant de la fin du message
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
	const string valeurXORD�part = "123456789012345";
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
			//Si on arrive au dernier caractere de la cl�, on la remet au caractere 0
			if (j == clef.size())
			{
				k = 0;
			}
			charClefInt = clef[k];
			k++;
			charCryptInt = blocCrypt[j];

			//Recherche dans le tableau de vigenere pour trouver un caractere d�cod�
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
				charClairInt = charXORInt ^ valeurXORD�part[j];
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
