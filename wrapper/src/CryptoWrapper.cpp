#include "cryptoki.h"
#include <stdio.h>
#include <windows.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include "CryptoWrapper.h"

CryptoWrapper::CryptoWrapper(void)
{
}

CryptoWrapper::~CryptoWrapper(void)
{
	//destroy the crypto environment
	finalizeCrypto();
}

bool CryptoWrapper::initCrypto() {
	CK_RV	returnValue;	//holds the return value
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

void CryptoWrapper::finalizeCrypto() {
	if(!PKCSLibraryModule) {
		return;  //if there is nothing loaded we are done
	}
	funcList->C_Finalize(NULL_PTR);
	FreeLibrary(PKCSLibraryModule);
	PKCSLibraryModule = 0;  //set library to zero to be safe
}

//count the number of cards which have tokens
int CryptoWrapper::getTokenCount(){
	CK_ULONG SlotCount;	//Number of connected readers with tokens
	//Search readers to get count
	(funcList->C_GetSlotList)(TRUE,NULL_PTR,&SlotCount);
	return SlotCount;
}

//Get list of all slots with a token present
string* CryptoWrapper::enumerateCards(void)
{
	CK_RV	returnValue;			//holds the return value
	CK_SLOT_INFO_PTR SlotInfo;		//Pointer to slot info
	CK_ULONG SlotWithTokenCount;	//Number of connected readers

	int TokenCount = getTokenCount();

	SlotWithTokenList = (CK_SLOT_ID_PTR) malloc(0);		
	SlotWithTokenCount = 0;
	//array that contains slots info (return value)
	string	*SlotsArray = new string[TokenCount]; 

	while (1) {
		//Search readers and store result in SlotWithTokenList;
		returnValue = (funcList->C_GetSlotList)(TRUE, SlotWithTokenList, &SlotWithTokenCount);

		if (returnValue != CKR_BUFFER_TOO_SMALL) {
			break;
		}		

		SlotWithTokenList = (unsigned long *)malloc(sizeof(CK_SLOT_ID)*SlotWithTokenCount);
	}

	if (returnValue == CKR_OK && SlotWithTokenCount > 0) {
		SlotInfo = (CK_SLOT_INFO_PTR)malloc(sizeof(CK_SLOT_INFO));

		// Get all the slots info
		for(unsigned int i = 0; i < SlotWithTokenCount; i++) {
			returnValue = (funcList->C_GetSlotInfo)(SlotWithTokenList[i],SlotInfo);
			if(returnValue == CKR_OK) {
				SlotsArray[i] = ((char*)SlotInfo->slotDescription);
				printf("Slot %d: ",i);
				cout << SlotsArray[i] << endl; // somehow it doesn't print with printf()!
			}
		}
	}
	else {
		setError(NO_DEVICES_FOUND);
		return NULL;
	}

	//returns list of all cards on the system. An empty list indicates an error;
	return SlotsArray;
}

//Selects a card to use for subsequent operations.  Returns false on failure and sets 
bool CryptoWrapper::selectCard(int SlotID)
{
	CK_RV	returnValue;	//holds the return value
	//Open session for selected card
	returnValue = (funcList->C_OpenSession)(SlotWithTokenList[SlotID],
					CKF_SERIAL_SESSION| CKF_RW_SESSION, NULL, NULL, &hSession);

	if (returnValue == CKR_OK) {
		CK_UTF8CHAR UserPIN[] = SC_PIN_CODE;
		
		returnValue = (funcList->C_Login)(hSession, CKU_USER, UserPIN, sizeof(UserPIN)-1);

		if (returnValue != CKR_OK) {
			setError(WRONG_PASSWORD);
			return false;
		}
		else {			
			return true;
		}
	}
	else {
		setError(OPEN_SESSION_FAILED);
		return false;
	}
}

//encrypts data with the key given by keyLabel
string CryptoWrapper::encrypt( string plainText, string keyLabel )
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
	
	/* search for label which matches our key */
	getPublicKey(keyLabel, key);

//	funcList->C_FindObjectsFinal(hSession);
	/* begin encryption */
	if((funcList->C_EncryptInit)(hSession, &mechanism, key) != CKR_OK) 
	{
		setError(COULD_NOT_INIT_ENCRYPT);
		return "";
	}
	
	plainTextLength = sizeof(plainText.c_str());
	inBuffer = (CK_BYTE*)plainText.c_str();
	//encrypt once to get the size
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

bool CryptoWrapper::encryptFile(string fileToEncrypt, string encryptedFile, string strKeyLabel)
{
	int retval = 0;
	ifstream fileRead;
	ofstream fileWrite;
	string strToEncrypt;
	stringstream inputStream;
	string strEncrypted;

	
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
	strEncrypted = encrypt(strToEncrypt, strKeyLabel);

	if(strEncrypted == "") {
		return false;
	}

	/* writing header, data, footer to file */
	try
	{
		/* create file */
		fileWrite.open( encryptedFile.c_str(), fstream::in | fstream::out | fstream::trunc );
		
		fileWrite << "<type>encrypted</type>";
		fileWrite << "<cert>" << strKeyLabel.c_str() << "</cert>";
		fileWrite << "<enc>" << strEncrypted.c_str() << "</enc>";
		
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

string CryptoWrapper::decrypt(string cipherText, string keyLabel)
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
	CK_ULONG length = cipherText.size();
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

string CryptoWrapper::LoadFile(string filename) {
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

bool CryptoWrapper::getPublicKey(string keyName, CK_OBJECT_HANDLE &pubKey) {
	CK_OBJECT_CLASS pubKeyClass  = CKO_PUBLIC_KEY;
	CK_KEY_TYPE KeyType = CKK_RSA;
	CK_BBOOL True = TRUE;
	CK_ULONG count;
	CK_RV	returnValue;

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

bool CryptoWrapper::getPrivateKey(string keyName, CK_OBJECT_HANDLE &privKey) {
	CK_RV	returnValue;	//holds the return value
	CK_OBJECT_CLASS keyClass  = CKO_PRIVATE_KEY;
	CK_KEY_TYPE keyType = CKK_RSA;
	CK_BBOOL True = TRUE;
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

string CryptoWrapper::sign( string plainText, string keyLabel )
{
	CK_RV	returnValue;	//holds the return value
	CK_OBJECT_HANDLE key;
	CK_ULONG PlainTextLength;
	CK_ULONG DigestLen;			
	CK_ULONG SignatureLen;								
	CK_BYTE* inBuffer;
	CK_BYTE* signBuffer;
	CK_BYTE* digestBuffer;
	CK_MECHANISM mechanism_digest = {CKM_SHA_1, NULL_PTR, 0};
	CK_MECHANISM mechanism_sign = {CKM_RSA_PKCS, NULL_PTR, 0};

	/* get private keys for signing, verify using public key */
	getPrivateKey(keyLabel, key);
	
	/* init digest */
	returnValue = (funcList->C_DigestInit)(hSession, &mechanism_digest);
	if(returnValue != CKR_OK)
	{
		setError(COULD_NOT_INIT_DIGEST);
		return"";
	}

	PlainTextLength = plainText.size();
	inBuffer = (CK_BYTE*)plainText.c_str();
	
	/* do to get length for digest buffer */
	returnValue = (funcList->C_Digest)(hSession, inBuffer, PlainTextLength, NULL_PTR, &DigestLen);
	if(returnValue != CKR_OK)
	{ 
		setError(FAILED_TO_DIGEST);
		return "";
	}
	
	/* digest data */
	digestBuffer = (CK_BYTE*)malloc(sizeof(CK_BYTE) * DigestLen);
	returnValue = (funcList->C_Digest)(hSession, inBuffer, PlainTextLength, digestBuffer, &DigestLen);
	if(returnValue != CKR_OK)
	{
		setError(FAILED_TO_DIGEST);
		return ""; 
	}
	
	/* init sign function */
	returnValue = (funcList->C_SignInit)(hSession, &mechanism_sign, key);
	if(returnValue != CKR_OK)
	{
		setError(COULD_NOT_INIT_SIGN);
		return ""; 
	}
	
	/* get length for sign buffer */
	returnValue = (funcList->C_Sign)(hSession, digestBuffer, DigestLen, NULL_PTR, &SignatureLen);
	if(returnValue != CKR_OK)
	{
		setError(FAILED_TO_SIGN);
		return ""; 
	}
	
	/* sign */
	signBuffer = (CK_BYTE*)malloc(sizeof(CK_BYTE) * SignatureLen);
	returnValue = (funcList->C_Sign)(hSession, digestBuffer, DigestLen, signBuffer, &SignatureLen);
	if(returnValue != CKR_OK)
	{
		setError(FAILED_TO_SIGN);
		return ""; 
	}
	
	string output = (char*)signBuffer;
	return output;
}

bool CryptoWrapper::signFile( string fileToSign, string signedFile, string strKeyLabel )
{
	ifstream fileRead;
	ofstream fileWrite;
	string strSigned;
	string strToDigestSign;
	stringstream inputStream;
	
	fileRead.exceptions ( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
	fileWrite.exceptions ( ofstream::eofbit | ofstream::failbit | ofstream::badbit );

	try
	{
		fileRead.open( fileToSign.c_str() );
		if (fileRead.fail()) 
		{ 

			return false;
		}
		inputStream << fileRead.rdbuf();
		strToDigestSign = inputStream.str();

		fileRead.close();
	}
	catch (ifstream::failure e)
	{
		setError(FAILED_TO_OPEN_READ_FILE);
	       	return	false;
	}

	strSigned = sign(strToDigestSign, strKeyLabel);
	
	if (strSigned == "")
		return false;

	try 
	{
		fileWrite.open( signedFile.c_str(), fstream::in | fstream::out | fstream::trunc );
		
		/* write stream to file, TRUNCATE MODE */
		fileWrite << strToDigestSign;
		fileWrite << "<type>signature</type>";
		fileWrite << "<cert>" << strKeyLabel.c_str() << "</cert>";
		fileWrite << "<sign>" << strSigned.c_str() << "</sign>";
		
		fileWrite.close();
	}
	catch (ofstream:: failure e) 
	{
		setError(FAILED_TO_OPEN_WRITE_FILE);
		return false;
	}

	return true;
}