#include "SignedData.h"

using namespace std;

SignedData::SignedData(void)
{
	this->signature = NULL;
	this->sigSize = 0;
}

SignedData::~SignedData(void)
{
	if(signature) {
		free(signature);
	}
}


void SignedData::writeToFile(ofstream &file) {
	file << (char)9 <<"signature";
	int size = cert.size();
	file.write((char*)&size,sizeof(int));
	file << cert.c_str();
	size = plainText.size();
	file.write((char*)&size,sizeof(int));
	file << plainText.c_str();
	file.write((char*)&sigSize,sizeof(CK_ULONG));
	file.write((char*)signature, sigSize * sizeof(CK_BYTE));
}

void SignedData::readFromFile(ifstream &file) {
	int size;
	file.read((char*)&size, sizeof(int));
	char *temp = (char *)malloc((size + 1) * sizeof(char));
	file.read(temp, size);
	temp[size] = '\0';
	this->cert = string(temp);
	free(temp);
	file.read((char*)&size, sizeof(int));
	temp = (char *)malloc((size + 1) * sizeof(char));
	file.read((char *)temp, size);
	temp[size] = '\0';
	this->plainText = string(temp);
	file.read((char*)&sigSize, sizeof(CK_ULONG));
	signature = (CK_BYTE *)malloc(sigSize * sizeof(CK_BYTE));
	file.read((char *)signature,sigSize);
}

void SignedData::setSignature(CK_BYTE *sig, CK_ULONG size) {
	this->sigSize = size;
	if(this->signature) {
		free(this->signature);
	}
	this->signature = (CK_BYTE *)malloc(size * sizeof(CK_BYTE));
	memcpy((void*)this->signature, (void *)sig, (size * sizeof(CK_BYTE)));
}