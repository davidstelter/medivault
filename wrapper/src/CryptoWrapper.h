#include "errorcodes.h"

#pragma once

#define SC_PIN_CODE "12345678"  //pin code for testing

using namespace std;

class CryptoWrapper
{
public:
	CryptoWrapper(void);
	~CryptoWrapper(void);
	//functions for handling errors
	int getLastError() {return lastError;}
	//functions for setting up the crypto environment
	bool initCrypto();
	string* enumerateCards();
	bool selectCard(int SlotID);
	//functions for taking down the crypto environment
	void finalizeCrypto();
	//functions for dealing with encryption
	bool encryptFile(string fileToEncrypt, string encryptedFile, string strKeyLabel);
	//functions for dealing with files
	string LoadFile(string filename);
private:
	//error functions
	void setError(int errorCode) {lastError = errorCode;}
	//utility functions
	int getTokenCount();
	//functions for finding keys
	bool getPublicKey(string keyName, CK_OBJECT_HANDLE &pubKey);
	bool getPrivateKey(string keyName, CK_OBJECT_HANDLE &privKey);
	//functions for encrypting stuff
	string encrypt(string plainText, string keyLabel);
	//functions for dealing with decryption
	string decrypt(string cipherText, string keyLabel);\

	//member variables
	int lastError;					//Holds the last error
	HMODULE PKCSLibraryModule;		//Holds the library which we will load
	CK_FUNCTION_LIST_PTR funcList;  //Holds enterance point for the functions
	CK_SLOT_ID_PTR SlotWithTokenList;	//Holds a list of slots with tokens
	CK_SESSION_HANDLE hSession;		//Holds the session with the token
};
