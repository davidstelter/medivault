#include "cryptoki.h"
#include <string>
#include <sstream>
#include <fstream>

#include "CryptoWrapper.h"

/*
encrypt() do the following:
	- Search for label which matches our key.
	- Initializes an encryption operation.
	- Encrypts once to get the size.
	- Encrypts the data.
	- Returns the encrypted data.
*/

//encrypts data with the key given by keyLabel
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

	returnValue = (funcList->C_Encrypt)(hSession, inBuffer, plainTextLength, outBuffer, &encrypted);
	if(returnValue != CKR_OK) 
	{
			setError(FAILED_TO_ENCRYPT);
			return NULL;
	}
	
	size = encrypted;
	return outBuffer;
}


/*
encryptFile() do the following:
	- Opens the file that wanted to be encrypted, then reads the data, saves it, and then close the file.
	- Encrypts the saved data by calling the encrypt function.
	- Creates a new file to write the encryped data with a header and a footer.
	- Closes the encrypted file.
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
	catch (ifstream::failure e) 
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
		
		fileWrite << (char)9 <<"encrypted";
		int size = strKeyLabel.size();
		fileWrite.write((char*)&size,4);
		fileWrite << strKeyLabel.c_str();
		fileWrite.write((char*)&encSize,4);
		fileWrite.write((char*)strEncrypted, encSize * sizeof(CK_BYTE));
		
		/* close */
		fileWrite.close();
	}
	catch (ofstream:: failure e) 
	{
		setError(FAILED_TO_OPEN_WRITE_FILE);
		return false;
	}
	return true;
}