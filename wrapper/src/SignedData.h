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
	//helper function to read from file
	void readFromFile(ifstream &file);
	//helper funciton to write from file
	void writeToFile(ofstream &file);
	//helper function to set given signature to buffer memory
	void setSignature(CK_BYTE *sig, CK_ULONG size);
	//helper function to set given certificate
	void setCert(string cert){this->cert = cert;};
	//helper function to set given plaintext
	void setPlainText(string text){this->plainText = text;};
	//helper function to get signature
	CK_BYTE *getSignature(){return signature;};
	CK_LONG	getSigSize(){return sigSize;};
	//helper function to get plaintext
	string getPlainText(){return plainText;};
	//helper function to get certificates
	string getCert(){return cert;};
private:
	CK_BYTE *signature; /**< signature pointer */
	CK_ULONG sigSize;	/**< signature size */
	string plainText;	/**< plaintext */
	string cert;		/**< certificate name */
};
