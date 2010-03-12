/*
 * Copyright © 2010 Dylan Enloe, Vincent Cao, Muath Alissa
 * ALL RIGHTS RESERVED
 *
 * FileLoading.cpp
 * This class contains the functions for loading files that are either plain
 * text or were created using the encrypt or sign functions of the
 * CryptoWrapper class.  They are also part of the CryptoWrapper class.
 *
 * Functions:
 *		LoadFile
 *		decryptFile
 *		Verify
 * */

#include "cryptoki.h"
#include <string>
#include <sstream>
#include <fstream>

#include "SignedData.h"
#include "EncryptedData.h"
#include "CryptoWrapper.h"


/*!
*	@brief
*	Functions for dealing with files
*	LoadFile() do the following:
*		- Opens the file that wanted to be Decryped or Verify, reads the data and saves it, then closes the file.
*		- Parse out whether decrypted or verify.
*		- Skips over the first encoding of the data.
*		- Skips over the cipherText of the data.
*		- Decrypts the data by calling Decrypt function.
*		- Verify the signature by calling verify function.
*	@param [in] filename Name of file to load
*	@retval string
*	@remarks
*/
string CryptoWrapper::LoadFile(string filename) {
	stringstream inputStream;
	string buffer;

	ifstream file;
	file.exceptions ( ifstream::eofbit | ifstream::failbit | ifstream::badbit );

	//read file get retrieve data
	try 
	{
		file.open( filename.c_str(), fstream::binary | fstream::in );
	}
	catch (ifstream::failure e) 
	{
		setError(FAILED_TO_OPEN_READ_FILE);
		return "";
	}

	//parsing data retrieve
	char first;
	file.read(&first,1);
	if(first == 9) {
		char type[10];
		file.read(type, 9);
		if(strncmp(type, "encrypted", 9) == 0){	//parsing...if file data is encrypted then decrypt
			EncryptedData data;
			data.readFromFile(file);
			return decryptFile(data);			
		} else if(strncmp(type, "signature", 9) == 0) {//else then the user wants to verify signature
			SignedData data;
			data.readFromFile(file);
			if(!Verify(data)) {
				return "";
			}
			return data.getPlainText();
		}
	}
	file.close();

	//falls through above if file is not one of our filetypes
	try 
	{
		file.open( filename.c_str() );
		inputStream << file.rdbuf();
		buffer = inputStream.str();
		file.close();
	}
	catch (ifstream::failure e) 
	{
		setError(FAILED_TO_OPEN_READ_FILE);
		return "";
	}
	return inputStream.str();
}


/*!
*	@brief
*	Functions for dealing with decryption
*	decryptFile() do the following:
*		- Gets the private key.
*		- Initializes a decryption operation.
*		- Decrypts once to get the size.
*		- Decrypts the data.
* 	@param [in] &data EncryptedData type passed in to be decrypt
*	@retval string Returns decyphered string
*	@remarks
* 	Passes the inputed buffer string and decrypts with private key using acospkcs11 function calls.
*/
string CryptoWrapper::decryptFile(EncryptedData &data)
{
	CK_RV	returnValue;									//holds the return value
	CK_OBJECT_HANDLE key;
	CK_MECHANISM mechanism = {CKM_RSA_PKCS, NULL_PTR, 0};	//we will be decrypting using RSA PKCS
	
	getPrivateKey(data.getCert(), key); //get the key
	
	if(funcList->C_DecryptInit(hSession, &mechanism, key) != CKR_OK) {  //begin decryption
		setError(COULD_NOT_INIT_DECRYPTION);
		return "";
	}
	CK_ULONG decrypted;

	//decrypt once to get the size
	returnValue = (funcList->C_Decrypt)(hSession, data.getCipherText(), data.getCipherSize(), NULL_PTR, &decrypted);
	if(returnValue != CKR_OK) 
	{
			setError(FAILED_TO_ENCRYPT);
			return "";
	}
	CK_BYTE* outBuffer = (CK_BYTE*)malloc(decrypted); //alloc the buffer memory for decrypt 

	//decrypt the data
	if(funcList->C_Decrypt(hSession, data.getCipherText(), data.getCipherSize(), outBuffer, &decrypted)
		!= CKR_OK) {
			setError(FAILED_TO_DECRYPT);
			return "";
	}
	string output = "";/*(char*)outBuffer;*/
	for(int i = 0; i < decrypted; i++) {
		output = output + (char)outBuffer[i];
	}
	return output;
}


/*!
*	@brief
*	Function for verifying signature
*	Verify() do the following:
*		- Get public key for verifying against signature
*		- Digest the plaintext given
*		- Verify digest data against given signature using given public key
*	@param [in] &data Type signData passed in to be verify
*	@retval bool Returns True if signature verified, False if not
*/
bool CryptoWrapper::Verify(SignedData &data)
{
	CK_RV	returnValue;
	CK_OBJECT_HANDLE key;
	CK_ULONG digestSize = 0;								
	CK_BYTE* digestBuffer = NULL;
	CK_MECHANISM mechanism = {CKM_RSA_PKCS, NULL_PTR, 0};
	
	//get private keys for signing, verify using public key 
	getPublicKey(data.getCert(), key);

	//digest data
	digestBuffer = Digest(data.getPlainText(), digestSize);
	if(digestBuffer == NULL) {
		return false;
	}

	//Initialize verification
	returnValue = (funcList->C_VerifyInit)(hSession, &mechanism, key);	
	if (returnValue != CKR_OK) {
		setError(FAILED_TO_VERIFY);
		return false;
	}	

	//Verifying
	returnValue = (funcList->C_Verify)(hSession, digestBuffer, digestSize, data.getSignature(), data.getSigSize());
	if(returnValue != CKR_OK) {
		setError(FAILED_TO_VERIFY);
		return false; 
	}
	return true;
}