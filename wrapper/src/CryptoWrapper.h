#include <windows.h>

#include "errorcodes.h"
#include "cryptoki.h"
#include "subject.h"

#pragma once

using namespace std;

class CryptoWrapper
{
public:
	CryptoWrapper(void);
	~CryptoWrapper(void);
	//functions for handling errors
	int getLastError() {return lastError;}
	//utility functions
	int getTokenCount();
	//functions for setting up the crypto environment
	bool initCrypto();
	string* enumerateCards();
	bool selectCard(int SlotID, CK_UTF8CHAR* pin, int pinlen);
	//functions for taking down the crypto environment
	void finalizeCrypto();
	//functions for dealing with encryption
	bool encryptFile(string fileToEncrypt, string encryptedFile, string strKeyLabel);
	//functions for dealing with files
	string LoadFile(string filename);
	//functions for signing files
	bool signFile(string fileToSign, string signedFile, string strKeyLabel);
	//functions for finding keys
	string* listKeys();
private:
	//error functions
	void setError(int errorCode) {lastError = errorCode;}
	//functions for finding keys
	bool getPublicKey(string keyName, CK_OBJECT_HANDLE &pubKey);
	bool getPrivateKey(string keyName, CK_OBJECT_HANDLE &privKey);
	bool getKey(string keyName, CK_OBJECT_CLASS keyClass, CK_OBJECT_HANDLE &key);
	//functions for encrypting stuff
	CK_BYTE* encrypt( string plainText, string keyLabel, int &size);
	//functions for dealing with decryption
	string decrypt(string cipherText, string keyLabel);
	string decryptFile(CK_BYTE* cipherText,CK_ULONG size, string keyLabel);
	//functions for signing stuff
	CK_BYTE* sign(string plainText, string keyLabel, int &size);
	CK_BYTE* Digest(string plainText, CK_ULONG &size);
	bool Verify(string plainText, CK_BYTE* signature, CK_ULONG sigSize, string keyLabel);

	//member variables
	int lastError;					//Holds the last error
	HMODULE PKCSLibraryModule;		//Holds the library which we will load
	CK_FUNCTION_LIST_PTR funcList;  //Holds enterance point for the functions
	CK_SLOT_ID_PTR SlotWithTokenList;	//Holds a list of slots with tokens
	CK_SESSION_HANDLE hSession;		//Holds the session with the token
};
