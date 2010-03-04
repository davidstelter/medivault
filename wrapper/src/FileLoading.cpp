#include "cryptoki.h"
#include <string>
#include <sstream>
#include <fstream>

#include "SignedData.h"
#include "EncryptedData.h"
#include "CryptoWrapper.h"

/*
LoadFile() do the following:
	- Opens the file that wanted to be Decryped, reads the data and saves it, then closes the file.
	- Skips over the first encoding of the data.
	- Skips over the cipherText of the data.
	- Decrypts the data by calling Decrypt function.
*/
string CryptoWrapper::LoadFile(string filename) {
	stringstream inputStream;
	string buffer;

	ifstream file;
	file.exceptions ( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
	try 
	{
		file.open( filename.c_str(), fstream::binary | fstream::in );
	}
	catch (ifstream::failure e) 
	{
		setError(FAILED_TO_OPEN_READ_FILE);
		return "";
	}
	char first;
	file.read(&first,1);
	if(first == 9) {
		char type[10];
		file.read(type, 9);
		if(strncmp(type, "encrypted", 9) == 0){
			EncryptedData data;
			data.readFromFile(file);
			return decryptFile(data);			
		} else if(strncmp(type, "signature", 9) == 0) {
			SignedData data;
			data.readFromFile(file);
			if(!Verify(data)) {
				return "";
			}
			return data.getPlainText();
		}
	}
	file.close();
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

/*
decryptFile() do the following:
	- Gets the private key.
	- Initializes a decryption operation.
	- Decrypts once to get the size.
	- Decrypts the data.
*/
string CryptoWrapper::decryptFile(EncryptedData &data)
{
	CK_RV	returnValue;	//holds the return value
	CK_OBJECT_HANDLE key;
	//we will be decrypting using RSA PKCS
	CK_MECHANISM mechanism = {CKM_RSA_PKCS, NULL_PTR, 0};
	//get the key
	getPrivateKey(data.getCert(), key);
	//begin decryption
	if(funcList->C_DecryptInit(hSession, &mechanism, key) != CKR_OK) {
		setError(COULD_NOT_INIT_DECRYPTION);
		return "";
	}
	CK_ULONG decrypted;
	//encrypt once to get the size
	returnValue = (funcList->C_Decrypt)(hSession, data.getCipherText(), data.getCipherSize(), NULL_PTR, &decrypted);
	if(returnValue != CKR_OK) 
	{
			setError(FAILED_TO_ENCRYPT);
			return "";
	}
	CK_BYTE* outBuffer = (CK_BYTE*)malloc(decrypted);

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

	//1. Initialize verification
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