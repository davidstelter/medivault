#include <pkcs11.h>
#include <string>
#include <iostream>

#include "error.h"

#define BUFFER_SIZE 256;

using namespace std;

CK_RV	returnValue;
extern CK_FUNCTION_LIST_PTR funcList;
extern CK_SESSION_HANDLE hSession;

string decrypt(string cipherText, string keyLabel)
{
	CK_OBJECT_HANDLE key;
	CK_ULONG numFound;
	//we will be decrypting using RSA PKCS
	CK_MECHANISM mechanism = {CKM_RSA_PKCS, NULL_PTR, 0};
	//load key
	CK_UTF8CHAR* label = (CK_UTF8CHAR*)keyLabel.c_str();
	CK_ATTRIBUTE searchKey[] = {CKA_LABEL, label, sizeof(label) - 1};
	//initialize our search for the label which matches our key
	returnValue = funcList->C_FindObjectsInit(hSession, searchKey,1); 
	if(returnValue == CKR_OK) {
		setError(INVALID_ATTRIBUTE);
		return "";
	}
	returnValue = funcList->C_FindObjects(hSession, &key, 1, &numFound);
	//if no result is found then return an error
	if(returnValue != CKR_OK || numFound == 0) {
		setError(KEY_NOT_FOUND);
		return "";
	}
	//finalize the search
	funcList->C_FindObjectsFinal(hSession);
	//begin decryption
	if(funcList->C_DecryptInit(hSession, &mechanism, key) != CKR_OK) {
		setError(COULD_NOT_INIT_DECRYPTION);
		return "";
	}
	CK_ULONG length = sizeof(cipherText.c_str());
	CK_ULONG *decrypted = 0;
	CK_BYTE* inBuffer = (CK_BYTE*)cipherText.c_str();
	CK_BYTE* outBuffer = (CK_BYTE*)malloc(length);
	if(funcList->C_Decrypt(hSession, inBuffer, length, outBuffer, decrypted)
		!= CKR_OK) {
			setError(FAILED_TO_DECRYPT);
			return "";
	}
	string output = (char*)outBuffer;
	return output;
}