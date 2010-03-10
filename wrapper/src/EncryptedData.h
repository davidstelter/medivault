#include "cryptoki.h"
#include <fstream>
#include <string>

#pragma once

using namespace std;

class EncryptedData
{
public:
	EncryptedData(void);
	~EncryptedData(void);
	//helper function to read data from file
	void readFromFile(ifstream &file);
	//helper function to write data to file
	void writeToFile(ofstream &file);
	//helper function to set cipher text into buffer 
	void setCipherText(CK_BYTE *cipherText, CK_ULONG size);
	void setCert(string cert){this->cert = cert;};
	// Function get cipher text
	CK_BYTE *getCipherText(){return cipherText;};
	// Function get cipher data size
	CK_LONG	getCipherSize(){return cipherSize;};
	// get certs
	string	getCert(){return cert;};
private:
	CK_BYTE *cipherText;	/**< cipherText pointer */
	CK_ULONG cipherSize;	/**< cipherText size */
	string cert;			/**< certificate name */
};
