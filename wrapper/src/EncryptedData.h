/*
 * Copyright � 2010 Dylan Enloe, Vincent Cao, Muath Alissa
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
 * EncryptedData.h
 * This is the class definition for the EncryptedData class. This class
 * Handles the reading and writing of encrypted files.  It does not actually
 * do the encrypting.
 * */

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
