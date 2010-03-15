/*
 * Copyright © 2010 Dylan Enloe, Vincent Cao, Muath Alissa
 * ALL RIGHTS RESERVED
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * CryptoWrapper.h
 * This file contains the definition of the the CryptoWrapper Class.  This
 * Class is a wrapper to wrap the functionality of the acospkcs11.dll.
 * It contains functions to encrypt, decrypt, sign, verify files.  As well
 * as function to load, initialize and log into ACOS5 sim tokens.
 * */

#include <windows.h>
#include <vector>

#include "SignedData.h"
#include "EncryptedData.h"
#include "errorcodes.h"
#include "cryptoki.h"

#pragma once

using namespace std;

class CryptoWrapper
{
public:
	CryptoWrapper(void);
	~CryptoWrapper(void);
	//functions for handling errors
	int getLastError() {return lastError;}
	void clearError() {lastError = OK; }
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
	vector<string> listKeys();
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
	string decryptFile(EncryptedData &data);
	//functions for signing stuff
	CK_BYTE* sign(string plainText, string keyLabel, int &size);
	CK_BYTE* Digest(string plainText, CK_ULONG &size);
	bool Verify(SignedData &data);

	//member variables
	int lastError;						//Holds the last error
	HMODULE PKCSLibraryModule;			//Holds the library which we will load
	CK_FUNCTION_LIST_PTR funcList;  	//Holds enterance point for the functions
	CK_SLOT_ID_PTR SlotWithTokenList;	//Holds a list of slots with tokens
	CK_SESSION_HANDLE hSession;			//Holds the session with the token
};
