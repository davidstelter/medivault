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
	void readFromFile(ifstream &file);
	void writeToFile(ofstream &file);
	void setCipherText(CK_BYTE *cipherText, CK_ULONG size);
	void setCert(string cert){this->cert = cert;};
	CK_BYTE *getCipherText(){return cipherText;};
	CK_LONG	getCipherSize(){return cipherSize;};
	string	getCert(){return cert;};
private:
	CK_BYTE *cipherText;
	CK_ULONG cipherSize;
	string cert;
};
