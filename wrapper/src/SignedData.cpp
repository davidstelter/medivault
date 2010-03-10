#include "SignedData.h"

using namespace std;

/*!
*	\class SignedData
*	@brief
*	This class is to interface with Signature Data
*/

/*!
*	@brief
*	SignedData Constructor
*	@param void
*	@retval
*	@remarks
* 	initialize signature class
*/
SignedData::SignedData(void)
{
	this->signature = NULL;
	this->sigSize = 0;
}

/*!
*	@brief
*	SignedData deconstructor
*	@param void
*	@retval
*	@remarks
* 	free signature memory
*/
SignedData::~SignedData(void)
{
	if(signature) {
		free(signature);
	}
}

/*!
*	@brief
*	Functions for write signature data to file
*	writeToFile() do the following:
*		- write certificate used in signing to file
* 		- write plaintext to be used in verify of signature
* 		- write signature data to file
*	@param [in] file Filename to write data out
*	@retval void
*	@remarks
* 	write signature data and plaintext data to file in order to be used 
* 	for future verify action.
*/
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

/*!
*	@brief
*	Functions for reading data from file to sign 
*	readFromFile() do the following:
*		- open input file
* 		- get certificate name to use for signing
* 		- read plaintext to be sign
*	@param [in] file Filename to read data in for signing
*	@retval void
*	@remarks
* 	read plaintext data from file and get certificate in order retrive keys
* 	for signing of data
*/
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

/*!
*	@brief
*	Functions for allocating buffer memory for signing data
*	setSignature() do the following:
*		- allocate space aside in memory for signature buffer
* 		- insert signature into memory buffer
*	@param [in] sig Signature data
* 	@param [in] size Size of signature
*	@retval void
*	@remarks
* 	allocated memory and place set signature
*/
void SignedData::setSignature(CK_BYTE *sig, CK_ULONG size) {
	this->sigSize = size;
	if(this->signature) {
		free(this->signature);
	}
	this->signature = (CK_BYTE *)malloc(size * sizeof(CK_BYTE));
	memcpy((void*)this->signature, (void *)sig, (size * sizeof(CK_BYTE)));
}
