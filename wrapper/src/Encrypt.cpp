/*
 * Copyright © 2010 Dylan Enloe, Vincent Cao, Muath Alissa
 * ALL RIGHTS RESERVED
 *
 * Encrypt.cpp
 * This file contains the implementation of the functions for encrypting data
 * as part of the CryptoWrapper class.
 *
 * Functions:
 *		encrypt
 *		encryptFile
 * */

#include "cryptoki.h"
#include <string>
#include <sstream>
#include <fstream>

#include "EncryptedData.h"
#include "CryptoWrapper.h"


/*!
*	@brief
*	Functions for encrypting data with the key given
*	encrypt() do the following:
*		- Search for label which matches our key.
*		- Initializes an encryption operation.
*		- Encrypts once to get the size.
*		- Encrypts the data.
*		- Returns the encrypted data.
*	@param [in] plainText Text string to be encrypt
* 	@param [in] keyLabel Name of certificate to be used
*   @param [in] &size Size of plaintext
*	@retval CK_BYTE Returns encrypted buffer pointer
*	@remarks
* 	Passes the inputed buffer string and encrypts with public key using acospkcs
* 	functional calls.
*/
CK_BYTE* CryptoWrapper::encrypt( string plainText, string keyLabel, int &size)
{
	CK_ULONG encrypted;
	CK_OBJECT_HANDLE key;
	CK_ULONG plainTextLength;
	CK_BYTE* inBuffer;
	CK_BYTE* outBuffer; 
	CK_MECHANISM mechanism = {CKM_RSA_PKCS, NULL_PTR, 0}; /* encrypt using RSA PCKS */	
	CK_UTF8CHAR* label = (CK_UTF8CHAR*)keyLabel.c_str();
	CK_ATTRIBUTE searchKey[] = {CKA_LABEL, label, sizeof(label) - 1};

	CK_RV	returnValue;
	
	size = 0;
	/* search for label which matches our key */
	getPublicKey(keyLabel, key);

	/* begin encryption */
	if((funcList->C_EncryptInit)(hSession, &mechanism, key) != CKR_OK) 
	{
		setError(COULD_NOT_INIT_ENCRYPT);
		return NULL;
	}
	
	plainTextLength = plainText.size();
	inBuffer = (CK_BYTE*)plainText.c_str();
	//encrypt once to get the size
	returnValue = (funcList->C_Encrypt)(hSession, inBuffer, plainTextLength, NULL_PTR, &encrypted);
	if(returnValue != CKR_OK) 
	{
			setError(FAILED_TO_ENCRYPT);
			return NULL;
	}
	outBuffer = (CK_BYTE*)malloc(sizeof(CK_BYTE) * encrypted);
	
	//now really going to encrypt with given buffer
	returnValue = (funcList->C_Encrypt)(hSession, inBuffer, plainTextLength, outBuffer, &encrypted);
	if(returnValue != CKR_OK) 
	{
			setError(FAILED_TO_ENCRYPT);
			return NULL;
	}
	
	size = encrypted;
	return outBuffer;
}


/*!
*	@brief
*	Functions for dealing with encryption
*	encryptFile() do the following:
*		- Opens the file that wanted to be encrypted, then reads the data, saves it, and then close the file.
*		- Encrypts the saved data by calling the encrypt function.
*		- Creates a new file to write the encryped data with a header and a footer.
*		- Closes the encrypted file.
*	@param [in] fileToEncrypt Name of file to be encrypted
* 	@param [out] encryptedFile Name of file to write encrypted data
* 	@param [in] strKeyLabel Certificate Name to use
*	@retval	bool Returns false on failure and sets
*	@remarks Encrypts data with the key given by keyLabel
*/
bool CryptoWrapper::encryptFile(string fileToEncrypt, string encryptedFile, string strKeyLabel)
{
	int retval = 0;
	ifstream fileRead;
	ofstream fileWrite;
	string strToEncrypt;
	stringstream inputStream;
	CK_BYTE* strEncrypted;

	
	fileRead.exceptions ( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
	fileWrite.exceptions ( ofstream::eofbit | ofstream::failbit | ofstream::badbit );
	
	/* opens the file to read content */
	try 
	{
		fileRead.open( fileToEncrypt.c_str() );
		if(fileRead.fail()){
			setError(FAILED_TO_READ_FILE);
			return false;
		}
		inputStream << fileRead.rdbuf();
		strToEncrypt = inputStream.str();
	
		fileRead.close();
	}
	catch (ifstream::failure e) /* catching error */
	{
		setError(FAILED_TO_OPEN_READ_FILE);
		return false;
	}
	
	/* encrypt sucka */
	int encSize;
	strEncrypted = encrypt(strToEncrypt, strKeyLabel, encSize);

	if(strEncrypted == NULL) {
		return false;
	}

	/* writing header, data, footer to file */
	try
	{
		/* create file */
		fileWrite.open( encryptedFile.c_str(), fstream::binary | fstream::out | fstream::trunc );
		//load the data into the handy file creator class
		EncryptedData data;
		data.setCipherText(strEncrypted,encSize);
		data.setCert(strKeyLabel);
		data.writeToFile(fileWrite);
		//don't forget to clean up the memory
		free(strEncrypted);
		//all done
		fileWrite.close();
	}
	catch (ofstream:: failure e) 
	{
		setError(FAILED_TO_OPEN_WRITE_FILE);
		return false;
	}
	return true;
}