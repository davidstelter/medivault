#include "cryptoki.h"
#include <fstream>
#include <string>

#pragma once

using namespace std;

class SignedData
{
public:
	SignedData(void);
	~SignedData(void);
	void readFromFile(ifstream &file);
	void writeToFile(ofstream &file);
	void setSignature(CK_BYTE *sig, CK_ULONG size);
	void setCert(string cert){this->cert = cert;};
	void setPlainText(string text){this->plainText = text;};
	CK_BYTE *getSignature(){return signature;};
	CK_LONG	getSigSize(){return sigSize;};
	string getPlainText(){return plainText;};
	string getCert(){return cert;};
private:
	CK_BYTE *signature;
	CK_ULONG sigSize;
	string plainText;
	string cert;
};
