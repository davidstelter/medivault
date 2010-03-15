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
 * EncryptedData.cpp
 * This file contains the implementation of the EncryptedData class.
 * */

#include "EncryptedData.h"

using namespace std;

/*!
*	\class EncryptedData
*	@brief
*	This class is to interface with Encrypting Data
*/

/*!
*	@brief
*	EncryptedData Constructor
*	@param void
*	@retval
*	@remarks
* 	initialize encrypteddata class
*/
EncryptedData::EncryptedData(void)
{
	this->cipherText = NULL;
	this->cipherSize = 0;
}

/*!
*	@brief
*	EncryptedData deconstructor
*	@param void
*	@retval
*	@remarks
* 	free encrypted memory
*/
EncryptedData::~EncryptedData(void)
{
	if(this->cipherText) {
		free(this->cipherText);
	}
}

/*!
*	@brief
*	Functions for reading data from file to sign 
*	readFromFile() do the following:
*		- open input file
* 		- get certificate name to use for encryption
* 		- read plaintext to be encrypt
*	@param [in] file Filename to read data in for signing
*	@retval void
*	@remarks
* 	read plaintext data from file and get certificate in order retrive keys
* 	for encrypting of data
*/
void EncryptedData::readFromFile(ifstream &file) {
	//the first X bytes contain the length of the cert
	int certSize;
	file.read((char*)&certSize, sizeof(int));
	//allocate memory to hold the cert name and read the name in
	char *temp = (char *)malloc((certSize + 1) * sizeof(char));
	file.read(temp, certSize);
	//terminate the string as otherwise there may be garbage at the end
	temp[certSize] = '\0';
	cert = string(temp);
	free(temp);
	//read in the next X bytes which is the size of the cipher text
	file.read((char*)&cipherSize, sizeof(CK_ULONG));
	//Allocate space for and read the cipher text
	cipherText = (CK_BYTE *)malloc(cipherSize * sizeof(CK_BYTE));
	file.read((char *)cipherText, cipherSize);
}

/*!
*	@brief
*	Functions for write encrypted data to file
*	writeToFile() do the following:
*		- write certificate used in decrypt to file
* 		- write encrypted data to file
*	@param [in] file Filename to write data out
*	@retval void
*	@remarks
* 	write encrypted data to file in order to be used 
* 	for future decrypting action.
*/
void EncryptedData::writeToFile(ofstream &file) {
	//write out the header
	file << (char)9 <<"encrypted";
	int size = cert.size();
	//write out the size
	file.write((char*)&size,sizeof(int));
	file << cert.c_str();
	//write out the cipher text
	file.write((char*)&cipherSize, sizeof(CK_ULONG));
	file.write((char*)cipherText, cipherSize * sizeof(CK_BYTE));
}

/*!
*	@brief
*	Functions for allocating buffer memory for encrypting data
*	setSignature() do the following:
*		- allocate space aside in memory for encryption buffer
* 		- insert encrypted data into memory buffer
*	@param [in] sig Signature data
* 	@param [in] size Size of signature
*	@retval void
*	@remarks
* 	allocated memory and place set signature
*/
void EncryptedData::setCipherText(CK_BYTE *cipherText, CK_ULONG size) {
	//This is a deep copy so that we don't have to worry about allocation issues
	this->cipherSize = size;
	if(this->cipherText) {
		free(this->cipherText);
	}
	this->cipherText = (CK_BYTE *)malloc(size * sizeof(CK_BYTE));
	memcpy((void*)this->cipherText, (void *)cipherText, (size * sizeof(CK_BYTE)));
}
