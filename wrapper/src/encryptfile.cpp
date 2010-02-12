#include <cryptoki.h>
#include <stdio.h>
#include <windows.h>
#include <iostream.h>
#include <fstream.h>
#include "error.h"	/* parse error class */

#define BUFFER_SIZE 256;

using namespace std;

CK_RV retValue;	
HMODULE PKCSLibMod = 0;         		/* pkcs library module handle */
CK_FUNCTION_LIST_PTR pFnList;			/* ptr to function list */
CK_SLOT_ID_PTR pSlotList;			/* ptr to slot list or reader list */
CK_SESSION_HANDLE hSession;			/* session handle */
CK_BBOOL bTrue = TRUE;		    
CK_BBOOL bFalse = FALSE;	    
CFile DataFile;						/* data file for encrypt */
CString DataFileName;

string encrypt( string plainText, string keyLabel )
{
	CK_ULONG *encrypted = 0;
	CK_OBJECT_HANDLE key;
	CK_ULONG numKeysFound;
	CK_ULONG plainTextLength;
	CK_BYTE* inBuffer;
	CK_BYTE* outBuffer; 
	CK_MECHANISM mechanism = {CKM_RSA_PKCS, NULL_PTR, 0}; /* encrypt using RSA PCKS */	
	CK_UTF8CHAR* label = (CK_UTF8CHAR*)keyLabel.c_str();
	CK_ATTRIBUTE searchKey[] = {CKA_LABEL, label, sizeof(label) - 1};
	
	/* search for label which matches our key */
	retValue = funcList->C_FindObjectsInit(hSession, searchKey,1); 
	if(retValue == CKR_OK) 
	{
		setError(INVALID_ATTRIBUTE);
		return "";
	}
	
	retValue = funcList->C_FindObjects(hSession, &key, 1, &numFound);
	if(retValue != CKR_OK || numFound == 0) 
	{
		/* no result, return error */
		setError(KEY_NOT_FOUND);
		return "";
	}

	funcList->C_FindObjectsFinal(hSession);
	/* begin encryption */
	if(funcList->C_EncryptInit(hSession, &mechanism, key) != CKR_OK) 
	{
		setError(COULD_NOT_INIT_DECRYPTION);
		return "";
	}
	
	plainTextLength = sizeof(plainText.c_str());
	inBuffer = (CK_BYTE*)plainText.c_str();
	outBuffer = (CK_BYTE*)malloc(plainTextLength);
	
	if(funcList->C_Encrypt(hSession, inBuffer, length, outBuffer, encrypted) != CKR_OK) 
	{
			setError(FAILED_TO_ENCRYPT);
			return "";
	}
	
	string output = (char*)outBuffer;
	return output;
}

int encryptFile( string fileToEncrypt, string encryptedFile )
{
	int retval = 0;
	ifstream fileRead;
	ofstream fileWrite;
	string strToEncrypt;
	string strEncrypted;
	string strKeyLabel;
	
	fileRead.exceptions ( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
	fileWrite.exceptions ( ofstream::eofbit | ofstream::failbit | ofstream::badbit );
	
	try 
	{
		fileRead.open( filename.c_str() );
		getline(fileRead,strToEncrypt); 
		
        	while(fileRead) 
		{  	 				
            		getline(file,strToEncrypt);
        	}

        	fileRead.close();
	}
	catch (ifstream::failure e) 
	{
		cout << "Exception opening/reading file";
		setError(FAILED_TO_OPEN_READ_FILE);
		return "";
	}
	
	/* encrypt sucka */
	strEncrypted = encrypt(strToEncrypt, strKeyLabel);

	/* writing header, data, footer to file */
	try
	{
		fileWrite.open( filename.c_str(), fstream::in | fstream::out | fstream::app );
		fileWrite << "<type>encrypted</type>";
		fileWrite.<< "<cert>" << strKeyLabel << "</cert>";
		fileWrite << "<enc>" << strEncrypted << "</enc>";
		fileWrite.close();
	}
	catch (ofstream:: failure e) 
	{
		cout << "Exception opening/writing to file";
		setError(FAIL_TO_OPEN_WRITE_FILE);
		return "";
	}

	return 0;
}
