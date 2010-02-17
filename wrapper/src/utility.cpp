#include "cryptoki.h"
#include <stdio.h>
#include <windows.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include "error.h"

#define BUFFER_SIZE 256
#define DEBUG_STDOUT_MSGS
#define SC_PIN_CODE "12345678"
#define SURVIVE_READ_FAIL

using namespace std;

HMODULE PKCSLibraryModule = 0;
CK_RV	returnValue;
CK_FUNCTION_LIST_PTR funcList;


CK_SLOT_ID_PTR pSlotWithTokenList;
CK_SESSION_HANDLE hSession;
int TokenCount = 0;

CK_BBOOL True = TRUE;		    
CK_BBOOL False = FALSE;

bool initCrypto() {
	if(PKCSLibraryModule) {
		setError(ALREADY_LOADED);
		return false;
	}
	//Load the DLL file.
	if (!(PKCSLibraryModule = LoadLibrary(L".\\acospkcs11.dll"))) {
		setError(DLL_LOAD_ERROR);
		return false;
	}
	//Get the function list
	CK_C_GetFunctionList pC_GetFunctionList = (CK_C_GetFunctionList)
		GetProcAddress(PKCSLibraryModule, "C_GetFunctionList");

	returnValue = (*pC_GetFunctionList)(&funcList);
	if (returnValue != CKR_OK) {
		setError(NO_FUNC_LIST);
		FreeLibrary(PKCSLibraryModule);
		PKCSLibraryModule = 0;  //set library to zero to be safe
		return false;
	}
	//initialize the cryptoEngine
	returnValue = (funcList->C_Initialize)(NULL_PTR);
	if (returnValue != CKR_OK) {
		setError(INIT_FAILED);
		FreeLibrary(PKCSLibraryModule);
		PKCSLibraryModule = 0;  //set library to zero to be safe
		return false;
	}
	return true;
}

void finalizeCrypto() {
	if(!PKCSLibraryModule) {
		return;  //if there is nothing loaded we are done
	}
	funcList->C_Finalize(NULL_PTR);
	FreeLibrary(PKCSLibraryModule);
	PKCSLibraryModule = 0;  //set library to zero to be safe
}

int getTokenCount(){
	CK_ULONG ulSlotCount;			//Number of connected readers
	//Search readers to get count
		returnValue = (funcList->C_GetSlotList)(TRUE,NULL_PTR,&ulSlotCount);
	return ulSlotCount;
}

//Get list of all slots with a token present
string *enumerateCards(void)
{
	CK_SLOT_INFO_PTR pSlotInfo;				//Pointer to slot info
	CK_ULONG ulSlotWithTokenCount;			//Number of connected readers

	pSlotWithTokenList = (CK_SLOT_ID_PTR) malloc(0);		
	ulSlotWithTokenCount = 0;
	string	*SlotsArray= new string[TokenCount];			// array that contains slots info (return value)



	while (1)
		{
			//Search readers and store result in pSlotWithTokenList;
			returnValue = (funcList->C_GetSlotList)(TRUE, pSlotWithTokenList, &ulSlotWithTokenCount);

			if (returnValue != CKR_BUFFER_TOO_SMALL)
				break;

			pSlotWithTokenList = (unsigned long *)malloc(sizeof(CK_SLOT_ID)*ulSlotWithTokenCount);
		}

	if (returnValue == CKR_OK && ulSlotWithTokenCount > 0) 
	{
			
			pSlotInfo = (CK_SLOT_INFO_PTR)malloc(sizeof(CK_SLOT_INFO));

			// Get all the slots info
			for(unsigned int i = 0; i < ulSlotWithTokenCount; i++)
			{
				returnValue = (funcList->C_GetSlotInfo)(pSlotWithTokenList[i],pSlotInfo);
				if(returnValue == CKR_OK)
				{
					SlotsArray[i] = ((char*)pSlotInfo->slotDescription);
					printf("Slot %d: ",i);
					cout << SlotsArray[i] << endl; // somehow it doesn't print with printf()!
				}
			}
	}
	else
		setError(NO_DEVICES_FOUND);

		//returns list of all cards on the system. An empty list indicates an error;
		return SlotsArray;
}


//Selects a card to use for subsequent operations.  Returns false on failure and sets 
bool selectCard(int SlotID)
{
		//Open session for selected card
		returnValue = (funcList->C_OpenSession)(pSlotWithTokenList[SlotID],
						CKF_SERIAL_SESSION| CKF_RW_SESSION, NULL, NULL, &hSession);

		if (returnValue == CKR_OK) 
		{
			CK_UTF8CHAR UserPIN[] = SC_PIN_CODE;
			
			returnValue = (funcList->C_Login)(hSession, CKU_USER, UserPIN, sizeof(UserPIN)-1);

			if (returnValue != CKR_OK) 
			{
				setError(WRONG_PASSWORD);
				return false;
			}
			else				
				return true;
		}
		else
		{
			setError(OPEN_SESSION_FAILED);
			return false;
		}
		returnValue = (funcList->C_Login)(hSession, CKU_USER, (CK_CHAR_PTR)"123", 3);
		cout << returnValue << endl;
}

bool getPublicKey(string keyName, CK_OBJECT_HANDLE &pubKey);
bool getPrivateKey(string keyName, CK_OBJECT_HANDLE &privKey);

string encrypt( string plainText, string keyLabel )
{
	CK_ULONG encrypted = 128;
	CK_OBJECT_HANDLE key;
	CK_ULONG numKeysFound;
	CK_ULONG plainTextLength;
	CK_BYTE* inBuffer;
	CK_BYTE* outBuffer; 
	CK_MECHANISM mechanism = {CKM_RSA_PKCS, NULL_PTR, 0}; /* encrypt using RSA PCKS */	
	CK_UTF8CHAR* label = (CK_UTF8CHAR*)keyLabel.c_str();
	CK_ATTRIBUTE searchKey[] = {CKA_LABEL, label, sizeof(label) - 1};
	
	/* search for label which matches our key */
	getPublicKey(keyLabel, key);

	/* begin encryption */
	returnValue = (funcList->C_EncryptInit)(hSession, &mechanism, key);
	if(returnValue != CKR_OK)
	{
		setError(COULD_NOT_INIT_ENCRYPT);
		return "";
	}
	
	plainTextLength = sizeof(plainText.c_str());
	inBuffer = (CK_BYTE*)plainText.c_str();
	
	/* encrypt once to get the size */
	returnValue = (funcList->C_Encrypt)(hSession, inBuffer, plainTextLength, NULL_PTR, &encrypted);
	if(returnValue != CKR_OK) 
	{
			setError(FAILED_TO_ENCRYPT);
			return "";
	}
	
	outBuffer = (CK_BYTE*)malloc(sizeof(CK_BYTE) * encrypted);
	returnValue = (funcList->C_Encrypt)(hSession, inBuffer, plainTextLength, outBuffer, &encrypted);
	if(returnValue != CKR_OK) 
	{
			setError(FAILED_TO_ENCRYPT);
			return "";
	}
	
	string output = (char*)outBuffer;
	return output;
}

bool encryptFile( string fileToEncrypt, string encryptedFile, string strKeyLabel )
{
	int retval = 0;
	ifstream fileRead;
	ofstream fileWrite;
#ifdef SURVIVE_READ_FAIL
	string strToEncrypt = "BOB";
#else
	string strToEncrypt = "BOB";
#endif
	stringstream inputStream;
	string strEncrypted;

	
	fileRead.exceptions ( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
	fileWrite.exceptions ( ofstream::eofbit | ofstream::failbit | ofstream::badbit );
	
	try 
	{
		fileRead.open( fileToEncrypt.c_str() );
		if(fileRead.fail()){
#ifdef DEBUG_STDOUT_MSGS
			cout << "Failed to open file " << fileToEncrypt << endl;
#endif
			return false;
		}
		inputStream << fileRead.rdbuf();
		strToEncrypt = inputStream.str();
	
		fileRead.close();
	}
	catch (ifstream::failure e) 
	{
#ifdef DEBUG_STDOUT_MSGS
		cout << "Exception opening/reading file";
#endif
		setError(FAILED_TO_OPEN_READ_FILE);
#ifndef SURVIVE_READ_FAIL
		return false;
#endif
	}
#ifdef DEBUG_STDOUT_MSGS
	cout << "Read file: " << strToEncrypt << endl;
#endif
	
	/* encrypt sucka */
	strEncrypted = encrypt(strToEncrypt, strKeyLabel);

	if(strEncrypted == "") {
		return false;
	}

	/* writing header, data, footer to file */
	try
	{
		/* create file */
		fileWrite.open( encryptedFile.c_str(), fstream::in | fstream::out | fstream::app );
		
		/* write stream to file, APPEND MODE */
		fileWrite << "<type>encrypted</type>";
		fileWrite << "<cert>" << strKeyLabel.c_str() << "</cert>";
		fileWrite << "<enc>" << strEncrypted.c_str() << "</enc>";
		
		/* close */
		fileWrite.close();
	}
	catch (ofstream:: failure e) 
	{
#ifdef DEBUG_STDOUT_MSGS
		cout << "Exception opening/writing to file";
#endif
		setError(FAILED_TO_OPEN_WRITE_FILE);
		return false;
	}

	return true;
}

string decrypt(string cipherText, string keyLabel)
{
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
	CK_ULONG length = sizeof(cipherText.c_str());
	CK_ULONG decrypted;
	CK_BYTE* inBuffer = (CK_BYTE*)cipherText.c_str();
	//encrypt once to get the size
	returnValue = (funcList->C_Encrypt)(hSession, inBuffer, length, NULL_PTR, &decrypted);
	if(returnValue != CKR_OK) 
	{
			setError(FAILED_TO_ENCRYPT);
			return "";
	}
	CK_BYTE* outBuffer = (CK_BYTE*)malloc(decrypted);

	if(funcList->C_Decrypt(hSession, inBuffer, length, outBuffer, &decrypted)
		!= CKR_OK) {
			setError(FAILED_TO_DECRYPT);
			return "";
	}
	string output = (char*)outBuffer;
	return output;
}

string LoadFile(string filename) {
	stringstream inputStream;
	string buffer;

	ifstream file;
	file.exceptions ( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
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
	if(buffer.substr(0,6) == "<type>") {
		if(buffer.substr(6,9) == "encrypted") {
			string temp;
			//skip over the first encoding
			string::size_type pos = buffer.find_first_of("<",22);
			if(string::npos == pos) {
				return inputStream.str();
			}
			buffer = buffer.substr(pos);
			pos = buffer.find_first_of("<",6);
			string cert = buffer.substr(6,pos-6);
			//skip over the cipherText
			pos = buffer.find_first_of("<",pos + 1);
			if(string::npos == pos) {
				return inputStream.str();
			}
			buffer = buffer.substr(pos);
			string cipherText = buffer.substr(5,buffer.find_last_of("<") - 5);
			return decrypt(cipherText, cert);
		}
	}
	return inputStream.str();
}

void listCerts(void);

int main(){
	if(initCrypto()){
		TokenCount = getTokenCount();
		printf("Token count: %d\n", TokenCount);

		if (TokenCount != 0){
			string *slots;
			slots = enumerateCards();
			
			selectCard(0);

			delete [] slots;
		}
	}
	else{
		printf("Initialization error: %d", getLastError());
	}
	listCerts();
	printf("about to encrypt...\n");
	encryptFile( "bob.txt", "bob.enc", "bob" );
	finalizeCrypto();

	
	cout << "Press almost any key to continue..." << endl;
	getchar();
	LoadFile("bob.enc");
	return 0;
}

bool getPublicKey(string keyName, CK_OBJECT_HANDLE &pubKey) {
	CK_OBJECT_CLASS pubKeyClass  = CKO_PUBLIC_KEY;
	CK_KEY_TYPE KeyType = CKK_RSA;
	CK_ULONG count;

	CK_ATTRIBUTE pubKeyTemplate[] = {
		{CKA_CLASS,		&pubKeyClass,	sizeof(CK_OBJECT_CLASS)},
		{CKA_KEY_TYPE,	&KeyType,		sizeof(CK_KEY_TYPE)},
		{CKA_TOKEN,		&True,			sizeof (True) },
	};

	returnValue = (funcList->C_FindObjectsInit)(hSession, pubKeyTemplate, 3);
	if (returnValue != CKR_OK) {
		return false;
	}

	returnValue = (funcList->C_FindObjects)(hSession, &pubKey, 1, &count);		
	if (returnValue != CKR_OK) {
		return false;
	}
	
	returnValue = (funcList->C_FindObjectsFinal)(hSession);
	if (returnValue != CKR_OK) {
		return false;
	}

	if (count < 1 ) {
		setError(KEY_NOT_FOUND);
		return false;
	}

	return true;
}

bool getPrivateKey(string keyName, CK_OBJECT_HANDLE &privKey) {
	CK_OBJECT_CLASS keyClass  = CKO_PRIVATE_KEY;
	CK_KEY_TYPE keyType = CKK_RSA;
	CK_ULONG count;

	CK_ATTRIBUTE keyTemplate[] = {
		{CKA_CLASS,		&keyClass,		sizeof(CK_OBJECT_CLASS)},
		{CKA_KEY_TYPE,	&keyType,		sizeof(CK_KEY_TYPE)},
		{CKA_TOKEN,		&True,			sizeof (True) },
	};

	returnValue = (funcList->C_FindObjectsInit)(hSession, keyTemplate, 3);
	if (returnValue != CKR_OK) {
		return false;
	}

	returnValue = (funcList->C_FindObjects)(hSession, &privKey, 1, &count);		
	if (returnValue != CKR_OK) {
		return false;
	}
	
	returnValue = (funcList->C_FindObjectsFinal)(hSession);
	if (returnValue != CKR_OK) {
		return false;
	}

	if (count < 1 ) {
		setError(KEY_NOT_FOUND);
		return false;
	}

	return true;
}

void listCerts(void) {
	CK_RV	rv;
	CK_BBOOL bTrue = TRUE;		    
	CK_BBOOL bFalse = FALSE;
	CK_OBJECT_HANDLE hPubKey, hPrivKey;
	
	CK_ULONG ulObjectCount;			
	CK_OBJECT_CLASS pubKeyClass  = CKO_PUBLIC_KEY;
	CK_OBJECT_CLASS privKeyClass = CKO_PRIVATE_KEY;					
	CK_KEY_TYPE KeyType = CKK_RSA;
	CK_BYTE *tempKeyID;
	CK_UTF8CHAR *tempLabel;

	// public key search template 
	CK_ATTRIBUTE pubKeyTemplate[] = {
		{CKA_CLASS,		&pubKeyClass,	sizeof(CK_OBJECT_CLASS)},
		{CKA_KEY_TYPE,	&KeyType,		sizeof(CK_KEY_TYPE)},
		{CKA_TOKEN,		&bTrue,			sizeof (bTrue) },
	};
	
	CK_ATTRIBUTE idTemplate[] = {
		{CKA_ID,		NULL,			0},		
	};

	// private key search template 
	CK_ATTRIBUTE privKeyTemplate[] = {
		{CKA_CLASS,		&privKeyClass,	sizeof(CK_OBJECT_CLASS)},
		{CKA_KEY_TYPE,	&KeyType,		sizeof(CK_KEY_TYPE)},
		{CKA_TOKEN,		&bTrue,			sizeof (bTrue) },
		{CKA_ID,		NULL,			0 },
	};

	CK_ATTRIBUTE labelTemplate[] = {
		{CKA_LABEL,		NULL,			0},		
	};

	//1. Initialize public key search 
		rv = (funcList->C_FindObjectsInit)(hSession, pubKeyTemplate, 3);
		if (rv != CKR_OK) return;


	//2. Search for public key
		rv = (funcList->C_FindObjects)(hSession, &hPubKey, 2, &ulObjectCount);		
		if (rv != CKR_OK) return;
		
		rv = (funcList->C_FindObjectsFinal)(hSession);
		if (rv != CKR_OK) return;

		if (ulObjectCount < 1 )
		{
			cout << "No public key found.";		
			return;
		} else
			cout << "PUBLIC KEY FOUND.";	
	

	//3. Get CKA_ID of public key to match with corresponding private key
		rv = (funcList->C_GetAttributeValue)(hSession, hPubKey, idTemplate, 1);
		if (rv != CKR_OK) return;

		tempKeyID = (CK_BYTE*)malloc(sizeof(CK_BYTE)*idTemplate[0].ulValueLen);
		idTemplate[0].pValue = tempKeyID;

		cout << endl;
		for(int i = 0; i < idTemplate[0].ulValueLen; i++) {
			cout << (int)tempKeyID[i];
		}
		cout << endl;

		rv = (funcList->C_GetAttributeValue)(hSession, hPubKey, idTemplate, 1);
		if (rv != CKR_OK) return;


	//4. Initialize private key search 
		privKeyTemplate[3].pValue = tempKeyID;
		privKeyTemplate[3].ulValueLen = idTemplate[0].ulValueLen;

		rv = (funcList->C_FindObjectsInit)(hSession, privKeyTemplate, 4);
		if (rv != CKR_OK) return;


	//5. Search for private key
		ulObjectCount = 0;

		rv = (funcList->C_FindObjects)(hSession, &hPrivKey, 1, &ulObjectCount);		
		if (rv != CKR_OK) return;
		
		rv = (funcList->C_FindObjectsFinal)(hSession);
		if (rv != CKR_OK) return;

		if (ulObjectCount < 1 )
		{
			cout << "No private key found.";		
			return;
		} else
			cout << "PRIVATE KEY FOUND.";	

}
