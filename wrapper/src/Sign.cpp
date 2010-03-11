#include "cryptoki.h"
#include <string>
#include <sstream>
#include <fstream>

#include "CryptoWrapper.h"


/*!
*	@brief
*	Functions for digesting
*	Digest() do the following:
*		- Initializes a message-digesting operation.
*		- Digests once to get the size.
*		- Starts digesting the data.
*	@param [in] plainText Text string to be digest and sign
* 	@param [in] &size name of certificate to be used 
*	@retval CK_BYTE Returns bytes of digest data
*	@remarks
* 	Passes the inputed buffer string and tries to digest data and pass back
*	the digested data in byte format.
*/
CK_BYTE* CryptoWrapper::Digest(string plainText, CK_ULONG &size) {
	CK_RV returnValue;
	CK_ULONG plainTextLength;
	CK_BYTE *buffer;
	CK_BYTE *digested;
	CK_MECHANISM mechanism = {CKM_SHA_1, NULL_PTR, 0};

	/* initialize digest function call */
	returnValue = (funcList->C_DigestInit)(hSession, &mechanism);
	if(returnValue != CKR_OK) {
		setError(COULD_NOT_INIT_DIGEST);
		return NULL;
	}

	plainTextLength = plainText.size();
	buffer = (CK_BYTE*)plainText.c_str();
	
	/* do to get length for digest buffer */
	returnValue = (funcList->C_Digest)(hSession, buffer, plainTextLength, NULL_PTR, &size);
	if(returnValue != CKR_OK) { 
		setError(FAILED_TO_DIGEST);
		return NULL;
	}
	
	/* digest data */
	digested = (CK_BYTE*)malloc(sizeof(CK_BYTE) * size);
	returnValue = (funcList->C_Digest)(hSession, buffer, plainTextLength, digested, &size);
	if(returnValue != CKR_OK) {
		setError(FAILED_TO_DIGEST);
		return NULL; 
	}
	return digested;
}


/*!
*	@brief
*	Functions for dealing with signature
*	sign() do the following:
*		- gets private keys for signing, verify using public key.
*		- Initializes a message-digesting operation.
*		- Digests once to get the size.
*		- Starts digesting the data.
*		- Initializes a signature operation
*		- Signs once to get the size.
*		- Starts signing the data.
*	@param [in] plainText Text string to be digest and sign
*	@param [in] &size Size of plaintext
* 	@param [in] keyLabel name of certificate to be used
*	@retval string Returns signed string
*	@remarks
* 	Passes the inputed buffer string and tries to digest and sign given with 
* 	private key using acospkcs functional calls.
*/
CK_BYTE*  CryptoWrapper::sign(string plainText, string keyLabel, int &size)
{
	CK_RV returnValue;	//holds the return value
	CK_OBJECT_HANDLE key;
	CK_ULONG digestLen;			
	CK_ULONG signatureLen;								
	CK_BYTE* signBuffer;
	CK_BYTE* digestBuffer;
	CK_MECHANISM mechanism_sign = {CKM_RSA_PKCS, NULL_PTR, 0};

	/* get private keys for signing, verify using public key */
	getPrivateKey(keyLabel, key);

	digestBuffer = Digest(plainText, digestLen);
	if(digestBuffer == NULL) {
		setError(FAILED_TO_DIGEST);
		return NULL;
	}
		
	/* init sign function */
	returnValue = (funcList->C_SignInit)(hSession, &mechanism_sign, key);
	if(returnValue != CKR_OK)
	{
		setError(COULD_NOT_INIT_SIGN);
		return NULL; 
	}
	
	/* get length for sign buffer */
	returnValue = (funcList->C_Sign)(hSession, digestBuffer, digestLen, NULL_PTR, &signatureLen);
	if(returnValue != CKR_OK)
	{
		setError(FAILED_TO_SIGN);
		return NULL; 
	}
	
	/* sign */
	signBuffer = (CK_BYTE*)malloc(sizeof(CK_BYTE) * signatureLen);
	returnValue = (funcList->C_Sign)(hSession, digestBuffer, digestLen, signBuffer, &signatureLen);
	if(returnValue != CKR_OK)
	{
		setError(FAILED_TO_SIGN);
		return NULL; 
	}
	size = signatureLen;
	return signBuffer;
}


/*!
*	@brief
*	Functions for signing files
*	signFile() do the following:
*		- Opens the file that wanted to be signed, then reads the data, saves it, and then close the file.
*		- Signs the saved data by calling the sign function.
*		- Creates a new file to write the signed data with a header and a footer.
*		- Closes the signed file.
*	@param [in] fileToSign Name of file to be signed
* 	@param [out] signedFile Name of file to write signed data
* 	@param [in] strKeyLabel Certificate Name to use
*	@retval	bool Returns false on failure and sets
*	@remarks digest data with the key given by keyLabel and sign data
*/
bool CryptoWrapper::signFile( string fileToSign, string signedFile, string strKeyLabel )
{
	ifstream fileRead;
	ofstream fileWrite;
	CK_BYTE* signature;
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
	int signedSize;
	signature = sign(strToDigestSign, strKeyLabel,signedSize);
	
	if (signature == NULL)
		return false;

	try 
	{
		fileWrite.open( signedFile.c_str(), fstream::binary | fstream::out | fstream::trunc );
		
		fileWrite << (char)9 <<"signature";
		int size = strKeyLabel.size();
		fileWrite.write((char*)&size,4);
		fileWrite << strKeyLabel.c_str();
		size = strToDigestSign.size();
		fileWrite.write((char*)&size,4);
		fileWrite << strToDigestSign.c_str();
		fileWrite.write((char*)&signedSize,4);
		fileWrite.write((char*)signature, signedSize * sizeof(CK_BYTE));
		
		fileWrite.close();
	}
	catch (ofstream:: failure e) 
	{
		setError(FAILED_TO_OPEN_WRITE_FILE);
		return false;
	}

	return true;
}

