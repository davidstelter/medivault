/*
 * Copyright © 2010 Dylan Enloe, Vincent Cao, Muath Alissa
 * ALL RIGHTS RESERVED
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * KeyFunctions.cpp
 * This file contains functions for finding and listing keys on the ACOS5 card
 * as part of the CryptoWrapper class.
 *
 * Functions
 *		getPublicKey
 *		getPrivateKey
 *		getKey
 *		listKeys
 * */

#include "cryptoki.h"
#include <string>
#include <vector>

#include "subject.h"
#include "CryptoWrapper.h"


/*!
*	@brief
*	Functions for getting the public key from given x509 certificates
*	getPublicKey() do the following:
*		- Gets the public key.
*	@param [in] keyName Name of certificate key
* 	@param [in] pubKey Handler to save the public key once found
*	@retval bool True for succesful transaction, False if not
*	@remarks
* 	Call getKey function to retrieve public key for given x509 Certificates
*/
bool CryptoWrapper::getPublicKey(string keyName, CK_OBJECT_HANDLE &pubKey) {
	CK_OBJECT_CLASS pubKeyClass  = CKO_PUBLIC_KEY;
	return getKey(keyName, pubKeyClass, pubKey);
}


/*!
*	@brief
*	Functions for getting the public key from given x509 certificates
*	getPrivateKey() do the following:
*		- Gets the private key.
*	@param [in] keyName Name of certificate key
* 	@param [in] privKey Handler to save the private key once found
*	@retval bool True for succesful transaction, False if not
*	@remarks
* 	Call getKey function to retrieve private key for given x509 Certificates
*/
bool CryptoWrapper::getPrivateKey(string keyName, CK_OBJECT_HANDLE &privKey) {
	CK_OBJECT_CLASS keyClass  = CKO_PRIVATE_KEY;
	return getKey(keyName,keyClass, privKey);
}

/*!
*	@brief
*	Functions for getting the public key from given x509 certificates
*	getKey() do the following:
*		- Sets up the template for the key and the certificate.
*		- Initializes the certificate search operation.
*		- Starts searching for certificate.
*		- Gets the subject field for each certificate it finds.
*		- Compares if it is the desired key.
*		- If no, Keep searching for another certificate.
*		- If yes, it finishes the certificate search operation.
*		- Gets the Key ID.
*		- Initializes the key search operation.
*		- Starts searching for the key.
*		- finishes the key search operation.
*	@param [in] keyName Name of certificate key
* 	@param [in] keyClass Class of keys, whether public or private
* 	@param [in] key	Handle to save key
*	@retval bool True for succesful transaction, False if not
*	@remarks
* 	Given the type of key to retrieve and the certificate where the keys
* 	are embedded, this class will parse through the certificate to find 
* 	subjects fields and search for the keys.
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
		//Get the next cert
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
			CK_ULONG size = 0;
			//finalize the certificate search as we have found the one we want
			returnValue = (funcList->C_FindObjectsFinal)(hSession);
			CK_ATTRIBUTE idTemplate[] = {
				{CKA_ID,		NULL,			0},		
			};
			//Get the id number of the cert that we just found
			returnValue = (funcList->C_GetAttributeValue)(hSession, *tempCert, idTemplate, 1);
			if (returnValue != CKR_OK) {
				return false;
			}
			//allocate space for the id and link the template and the id
			CK_BYTE *id = (CK_BYTE*)malloc(sizeof(CK_BYTE)*idTemplate[0].ulValueLen);
			idTemplate[0].pValue = id;
			
			//get the id for real this time.
			returnValue = (funcList->C_GetAttributeValue)(hSession, *tempCert, idTemplate, 1);
			if (returnValue != CKR_OK) { 
				return false;
			}
			size = idTemplate[0].ulValueLen;
			keyTemplate[3].pValue = id;
			keyTemplate[3].ulValueLen = idTemplate[0].ulValueLen;
			//start a new search.  This time we are looking for keys
			returnValue = (funcList->C_FindObjectsInit)(hSession, keyTemplate, 4);
			if (returnValue != CKR_OK) {
				return false;
			}
			//find the key with the id that is the same as the cert.
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

/*!
*	@brief
*	Functions for getting the public key from given x509 certificates
*	listKeys() do the following:
*		- Sets up the template for the certificate.
*		- Initializes the certificate search operation.
*		- Starts and keeps searching for certificate.
*		- Gets the subject field for each certificate it finds.
*		- Decrypts the subject line and get the information that we want.
*		- Adds the key to the list.
*		- Finishes the certificate search operation.
*	@retval vector<string> return a vector string of keys available
*	@remarks
*	Search for certificate and retrieve all subject field within certificate
* 	and decrypt to get information we need before allow keys to be use in
* 	transactions.
*/
vector<string> CryptoWrapper::listKeys() {
	CK_RV	returnValue;	//holds the return value
	//set up the template
	CK_OBJECT_CLASS certClass = CKO_CERTIFICATE;
	CK_CERTIFICATE_TYPE certType = CKC_X_509;
	CK_BBOOL True = TRUE;
	CK_ULONG count;
	CK_OBJECT_HANDLE *tempCert = new CK_OBJECT_HANDLE;

	vector<string> keyList;
	vector<string> nullVec;

	CK_ATTRIBUTE certTemplate[] = {
		{CKA_CLASS,				&certClass,	sizeof(CK_OBJECT_CLASS)},
		{CKA_CERTIFICATE_TYPE,	&certType,	sizeof(CK_CERTIFICATE_TYPE)},
		{CKA_TOKEN,				&True,		sizeof (True) },
	};
	//init the search
	returnValue = (funcList->C_FindObjectsInit)(hSession, certTemplate, 3);
	if (returnValue != CKR_OK) {
		return nullVec;
	}
	//while more results keep searching	
	while(true) {
		CK_BYTE_PTR subject;
		CK_ATTRIBUTE subjectTemplate[] = {
			{CKA_SUBJECT, NULL_PTR, 0}
		};
		returnValue = (funcList->C_FindObjects)(hSession, tempCert, 1, &count);		
		if (returnValue != CKR_OK) {
			return nullVec;
		}
		if(count == 0) {
			break;
		}
		//get the subject of each cert
		returnValue = (funcList->C_GetAttributeValue)(hSession, *tempCert,
			subjectTemplate, 1);
		if (returnValue != CKR_OK) {
			return nullVec;
		}
		//ok now we have to allocate space for the subject
		subject = (CK_BYTE_PTR)malloc(subjectTemplate[0].ulValueLen);
		subjectTemplate[0].pValue = subject;
		//get the actual value now that we have space for it
		returnValue = (funcList->C_GetAttributeValue)(hSession, *tempCert,
			subjectTemplate, 1);
		if (returnValue != CKR_OK) {
			return nullVec;
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
		return nullVec;
	}
	return keyList;
}
