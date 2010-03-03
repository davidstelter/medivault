#include "cryptoki.h"
#include <string>
#include <sstream>
#include <fstream>

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
		return false;
	}
	char first;
	file.read(&first,1);
	if(first == 9) {
		char type[10];
		file.read(type, 9);
		if(strncmp(type, "encrypted", 9) == 0){
			int certSize;
			file.read((char*)&certSize, 4);
			char *temp = (char *)malloc((certSize + 1) * sizeof(char));
			file.read(temp, certSize);
			temp[certSize] = '\0';
			string cert(temp);
			free(temp);
			CK_LONG encSize;
			file.read((char*)&encSize, 4);
			CK_BYTE *encData = (CK_BYTE *)malloc(encSize * sizeof(CK_BYTE));
			file.read((char *)encData, encSize);
			return decryptFile(encData,encSize,cert);			
		} else if(strncmp(type, "signature", 9) == 0) {
			int certSize;
			file.read((char*)&certSize, 4);
			char *temp = (char *)malloc((certSize + 1) * sizeof(char));
			file.read(temp, certSize);
			temp[certSize] = '\0';
			string cert(temp);
			free(temp);
			CK_LONG dataSize;
			file.read((char*)&dataSize, 4);
			char *data = (char *)malloc((dataSize + 1) * sizeof(char));
			file.read((char *)data, dataSize);
			data[dataSize] = '\0';
			string plainText(data);
			CK_LONG sigSize;
			file.read((char*)&sigSize, 4);
			CK_BYTE *sig = (CK_BYTE *)malloc(sigSize * sizeof(CK_BYTE));
			file.read((char *)sig,sigSize);
			if(!Verify(plainText,sig,sigSize,cert)) {
				return "";
			}
			return plainText;
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
		return false;
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
string CryptoWrapper::decryptFile(CK_BYTE* cipherText,CK_ULONG size, string keyLabel)
{
	CK_RV	returnValue;	//holds the return value
	CK_OBJECT_HANDLE key;
	//we will be decrypting using RSA PKCS
	CK_MECHANISM mechanism = {CKM_RSA_PKCS, NULL_PTR, 0};
	//get the key
	getPrivateKey(keyLabel, key);
	//begin decryption
	if(funcList->C_DecryptInit(hSession, &mechanism, key) != CKR_OK) {
		setError(COULD_NOT_INIT_DECRYPTION);
		return "";
	}
	CK_ULONG decrypted;
	//encrypt once to get the size
	returnValue = (funcList->C_Decrypt)(hSession, cipherText, size, NULL_PTR, &decrypted);
	if(returnValue != CKR_OK) 
	{
			setError(FAILED_TO_ENCRYPT);
			return "";
	}
	CK_BYTE* outBuffer = (CK_BYTE*)malloc(decrypted);

	if(funcList->C_Decrypt(hSession, cipherText, size, outBuffer, &decrypted)
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

bool CryptoWrapper::Verify(string plainText, CK_BYTE* signature, CK_ULONG sigSize, string keyLabel)
{
	CK_RV	returnValue;
	CK_OBJECT_HANDLE key;
	CK_ULONG digestSize = 0;								
	CK_BYTE* digestBuffer = NULL;
	CK_MECHANISM mechanism = {CKM_RSA_PKCS, NULL_PTR, 0};
	CK_UTF8CHAR* label = (CK_UTF8CHAR*)keyLabel.c_str();
	CK_ATTRIBUTE searchKey[] = {CKA_LABEL, label, sizeof(label) - 1};
	
	//get private keys for signing, verify using public key 
	getPublicKey(keyLabel, key);

	//digest data
	digestBuffer = Digest(plainText, digestSize);
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
	returnValue = (funcList->C_Verify)(hSession, digestBuffer, digestSize, signature, sigSize);
	if(returnValue != CKR_OK) {
		setError(FAILED_TO_VERIFY);
		return false; 
	}
	return true;
}