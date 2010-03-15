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
 * SignedData.h
 * Contains the definition of the SignedData class which is used for loading
 * and writing signed data files.
 * */

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
