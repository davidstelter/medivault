#include "EncryptedData.h"

using namespace std;

EncryptedData::EncryptedData(void)
{
	this->cipherText = NULL;
	this->cipherSize = 0;
}

EncryptedData::~EncryptedData(void)
{
	if(this->cipherText) {
		free(this->cipherText);
	}
}

void EncryptedData::readFromFile(ifstream &file) {
	int certSize;
	file.read((char*)&certSize, sizeof(int));
	char *temp = (char *)malloc((certSize + 1) * sizeof(char));
	file.read(temp, certSize);
	temp[certSize] = '\0';
	cert = string(temp);
	free(temp);
	file.read((char*)&cipherSize, sizeof(CK_ULONG));
	cipherText = (CK_BYTE *)malloc(cipherSize * sizeof(CK_BYTE));
	file.read((char *)cipherText, cipherSize);
}

void EncryptedData::writeToFile(ofstream &file) {
	file << (char)9 <<"encrypted";
	int size = cert.size();
	file.write((char*)&size,sizeof(int));
	file << cert.c_str();
	file.write((char*)&cipherSize, sizeof(CK_ULONG));
	file.write((char*)cipherText, cipherSize * sizeof(CK_BYTE));
}

void EncryptedData::setCipherText(CK_BYTE *cipherText, CK_ULONG size) {
	this->cipherSize = size;
	if(this->cipherText) {
		free(this->cipherText);
	}
	this->cipherText = (CK_BYTE *)malloc(size * sizeof(CK_BYTE));
	memcpy((void*)this->cipherText, (void *)cipherText, (size * sizeof(CK_BYTE)));
}
