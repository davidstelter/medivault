#include "cryptoki.h"
#include <windows.h>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

#include "subject.h"
#include "CryptoWrapper.h"

CryptoWrapper::CryptoWrapper(void)
{
	this->PKCSLibraryModule = 0;
	lastError = OK;
}

CryptoWrapper::~CryptoWrapper(void)
{
	//destroy the crypto environment
	finalizeCrypto();
}

/*
initCrypto() do the following:
	- Makes sure that the DLL was not loaded before.
	- Loads the DLL file.
	- Gets the functions list from the DLL file.
	- Initializes the Cryptoki library
*/
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

/*
finalizeCrypto() do the following:
	- Will finalize the Crypto library if it was loaded.
	- Frees the the library.
*/
void CryptoWrapper::finalizeCrypto() {
	if(!PKCSLibraryModule) {
		return;  //if there is nothing loaded we are done
	}
	if(!funcList) {
		return;
	}
	funcList->C_Finalize(NULL_PTR);
	FreeLibrary(PKCSLibraryModule);
	PKCSLibraryModule = 0;  //set library to zero to be safe
}

/*
getTokenCount() do the following:
	- Returns the number of cards which have tokens
*/
int CryptoWrapper::getTokenCount(){
	CK_RV	returnValue;	//holds the return value
	CK_ULONG ulSlotCount;	//Number of connected readers with tokens
	//Search readers to get count
	returnValue = (funcList->C_GetSlotList)(TRUE,NULL_PTR,&ulSlotCount);
	return ulSlotCount;
}

/*
enumerateCards() do the following:
	- Gets the number of cards which have tokens.
	- Creats a pointer to an array that contains slots info (return value).
	- Search readers and store result in SlotWithTokenList.
	- Gets all the slots info
	- Returns list of all slots with a token present
*/

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

/*
selectCard() do the following:
	- lists all slots with a token present.
	- Opens a session for the selected card to use for subsequent operations.
	- Logs into the selected token. 
*/
bool CryptoWrapper::selectCard(int SlotID, CK_UTF8CHAR* UserPIN, int pinlen)
{
	enumerateCards(); //this does a little housekeeping for us...
	CK_RV	returnValue;	//holds the return value
	//Open session for selected card
	returnValue = (funcList->C_OpenSession)(SlotWithTokenList[SlotID],
					CKF_SERIAL_SESSION| CKF_RW_SESSION, NULL, NULL, &hSession);

	if (returnValue == CKR_OK) {
		//CK_UTF8CHAR UserPIN[] = SC_PIN_CODE;
		
		//returnValue = (funcList->C_Login)(hSession, CKU_USER, UserPIN, sizeof(UserPIN)-1);
		returnValue = (funcList->C_Login)(hSession, CKU_USER, UserPIN, pinlen);

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

/*
encrypt() do the following:
	- Search for label which matches our key.
	- Initializes an encryption operation.
	- Encrypts once to get the size.
	- Encrypts the data.
	- Returns the encrypted data.
*/

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

/*
decrypt() do the following:
	- Gets the private key.
	- Initializes a decryption operation.
	- Encrypts once to get the size.
	- Decrypts the data.
*/
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

/*
getPublicKey() do the following:
	- Gets the public key.
*/
bool CryptoWrapper::getPublicKey(string keyName, CK_OBJECT_HANDLE &pubKey) {
	CK_OBJECT_CLASS pubKeyClass  = CKO_PUBLIC_KEY;
	return getKey(keyName, pubKeyClass, pubKey);
}

/*
getPrivateKey() do the following:
	- Gets the private key.
*/
bool CryptoWrapper::getPrivateKey(string keyName, CK_OBJECT_HANDLE &privKey) {
	CK_OBJECT_CLASS keyClass  = CKO_PRIVATE_KEY;
	return getKey(keyName,keyClass, privKey);
}

/*
getKey() do the following:
	- Sets up the template for the key and the certificate.
	- Initializes the certificate search operation.
	- Starts searching for certificate.
	- Gets the subject field for each certificate it finds.
	- Compares if it is the desired key.
	- If no, Keep searching for another certificate.
	- If yes, it finishes the certificate search operation.
	- Gets the Key ID.
	- Initializes the key search operation.
	- Starts searching for the key.
	- finishes the key search operation.
*/
bool CryptoWrapper::getKey(string keyName, CK_OBJECT_CLASS keyClass, CK_OBJECT_HANDLE &key) {
	CK_RV	returnValue;	//holds the return value
	//set up the template
	CK_KEY_TYPE keyType = CKK_RSA;
	CK_BBOOL True = TRUE;
	CK_ULONG count;

	CK_ATTRIBUTE keyTemplate[] = {
		{CKA_CLASS,		&keyClass,		sizeof(CK_OBJECT_CLASS)},
		{CKA_KEY_TYPE,	&keyType,		sizeof(CK_KEY_TYPE)},
		{CKA_TOKEN,		&True,			sizeof (True) },
		{CKA_ID,		NULL,			0 },
	};

	//cert stuff
	CK_OBJECT_CLASS certClass =		CKO_CERTIFICATE;
	CK_CERTIFICATE_TYPE certType =	CKC_X_509;
	CK_OBJECT_HANDLE *tempCert =	new CK_OBJECT_HANDLE;

	CK_ATTRIBUTE certTemplate[] = {
		{CKA_CLASS,				&certClass,	sizeof(CK_OBJECT_CLASS)},
		{CKA_CERTIFICATE_TYPE,	&certType,	sizeof(CK_CERTIFICATE_TYPE)},
		{CKA_TOKEN,				&True,		sizeof (True) },
	};

	//init the search
	returnValue = (funcList->C_FindObjectsInit)(hSession, certTemplate, 3);
	if (returnValue != CKR_OK) {
		return false;
	}
	//while more results keep searching	
	while(true) {
		CK_BYTE_PTR subject;
		CK_ATTRIBUTE subjectTemplate[] = {
			{CKA_SUBJECT, NULL_PTR, 0}
		};
		returnValue = (funcList->C_FindObjects)(hSession, tempCert, 1, &count);		
		if (returnValue != CKR_OK || count == 0) {
			setError(KEY_NOT_FOUND);
			return false;
		}
		//get the subject of each key
		returnValue = (funcList->C_GetAttributeValue)(hSession, *tempCert,
			subjectTemplate, 1);
		if (returnValue != CKR_OK) {
			return false;
		}
		//ok now we have to allocate space for the subject
		subject = (CK_BYTE_PTR)malloc(subjectTemplate[0].ulValueLen);
		subjectTemplate[0].pValue = subject;
		//get the actual value now that we have space for it
		returnValue = (funcList->C_GetAttributeValue)(hSession, *tempCert,
			subjectTemplate, 1);
		if (returnValue != CKR_OK) {
			return false;
		}
		//decrypt the subject line and get the information that we want
		Subject sub(subject);
		//free the objects used
		free(subject);
		//is this the key we want?
		if(sub.getCertName().compare(keyName) == 0) {
			//CK_BYTE *id = 0;
			CK_ULONG size = 0;
			returnValue = (funcList->C_FindObjectsFinal)(hSession);
			CK_ATTRIBUTE idTemplate[] = {
				{CKA_ID,		NULL,			0},		
			};
			returnValue = (funcList->C_GetAttributeValue)(hSession, *tempCert, idTemplate, 1);
			if (returnValue != CKR_OK) {
				return false;
			}
			CK_BYTE *id = (CK_BYTE*)malloc(sizeof(CK_BYTE)*idTemplate[0].ulValueLen);
			idTemplate[0].pValue = id;

			returnValue = (funcList->C_GetAttributeValue)(hSession, *tempCert, idTemplate, 1);
			if (returnValue != CKR_OK) { 
				return false;
			}
			size = idTemplate[0].ulValueLen;
			keyTemplate[3].pValue = id;
			keyTemplate[3].ulValueLen = idTemplate[0].ulValueLen;
			returnValue = (funcList->C_FindObjectsInit)(hSession, keyTemplate, 4);
			if (returnValue != CKR_OK) {
				return false;
			}

			returnValue = (funcList->C_FindObjects)(hSession, &key, 1, &count);		
			if (returnValue != CKR_OK) {
				return false;
			}
			
			returnValue = (funcList->C_FindObjectsFinal)(hSession);
			if (returnValue != CKR_OK || count == 0) {
				return false;
			}
			return true;
		}
	}
	//finalize the seach
	returnValue = (funcList->C_FindObjectsFinal)(hSession);
	if (returnValue != CKR_OK) {
		return NULL;
	}
	//no key was found
	setError(KEY_NOT_FOUND);
	return false;
}

/*
sign() do the following:
	- gets private keys for signing, verify using public key.
	- Initializes a message-digesting operation.
	- Digests once to get the size.
	- Starts digesting the data.
	- Initializes a signature operation
	- Signs once to get the size.
	- Starts signing the data.
*/
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

/*
signFile() do the following:
	- Opens the file that wanted to be signed, then reads the data, saves it, and then close the file.
	- Signs the saved data by calling the sign function.
	- Creates a new file to write the signed data with a header and a footer.
	- Closes the signed file.
*/
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
		fileWrite << "<type>signed</type>";
		fileWrite << "<cert>" << strKeyLabel.c_str() << "</cert>";
		fileWrite << "<data>" << strToDigestSign << "</data>";
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

/*
listKeys() do the following:
	- Sets up the template for the certificate.
	- Initializes the certificate search operation.
	- Starts and keeps searching for certificate.
	- Gets the subject field for each certificate it finds.
	- Decrypts the subject line and get the information that we want.
	- Adds the key to the list.
	- Finishes the certificate search operation.
	- Turns vector into an array.
*/
string* CryptoWrapper::listKeys() {
	CK_RV	returnValue;	//holds the return value
	//set up the template
	CK_OBJECT_CLASS certClass = CKO_CERTIFICATE;
	CK_CERTIFICATE_TYPE certType = CKC_X_509;
	CK_BBOOL True = TRUE;
	CK_ULONG count;
	CK_OBJECT_HANDLE *tempCert = new CK_OBJECT_HANDLE;

	vector<string> keyList;

	CK_ATTRIBUTE certTemplate[] = {
		{CKA_CLASS,				&certClass,	sizeof(CK_OBJECT_CLASS)},
		{CKA_CERTIFICATE_TYPE,	&certType,	sizeof(CK_CERTIFICATE_TYPE)},
		{CKA_TOKEN,				&True,		sizeof (True) },
	};
	//init the search
	returnValue = (funcList->C_FindObjectsInit)(hSession, certTemplate, 3);
	if (returnValue != CKR_OK) {
		return NULL;
	}
	//while more results keep searching	
	while(true) {
		CK_BYTE_PTR subject;
		CK_ATTRIBUTE subjectTemplate[] = {
			{CKA_SUBJECT, NULL_PTR, 0}
		};
		returnValue = (funcList->C_FindObjects)(hSession, tempCert, 1, &count);		
		if (returnValue != CKR_OK) {
			return NULL;
		}
		if(count == 0) {
			break;
		}
		//get the subject of each cert
		returnValue = (funcList->C_GetAttributeValue)(hSession, *tempCert,
			subjectTemplate, 1);
		if (returnValue != CKR_OK) {
			return NULL;
		}
		//ok now we have to allocate space for the subject
		subject = (CK_BYTE_PTR)malloc(subjectTemplate[0].ulValueLen);
		subjectTemplate[0].pValue = subject;
		//get the actual value now that we have space for it
		returnValue = (funcList->C_GetAttributeValue)(hSession, *tempCert,
			subjectTemplate, 1);
		if (returnValue != CKR_OK) {
			return NULL;
		}
		//decrypt the subject line and get the information that we want
		//insert into vector
		Subject sub(subject);
		string temp = sub.getCertName();
		keyList.insert(keyList.end(), temp);
		//free the objects used
		free(subject);
	}
	//finalize the seach
	returnValue = (funcList->C_FindObjectsFinal)(hSession);
	if (returnValue != CKR_OK) {
		return NULL;
	}
	//turn vector into an array
	string *stringArray = new string[keyList.size()];
	for(int i = 0; i < keyList.size(); i++) {
		stringArray[i] = keyList[i];
	}
	//return the array
	return stringArray;
}