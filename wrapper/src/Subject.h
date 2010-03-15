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
 * Subject.h
 * This file contains the defition of the subject class.  This class is for
 * decoding and storing data having to do with the subject of a certificate.
 * */

#include <string>
#include <vector>
#include "cryptoki.h"

#pragma once

using namespace std;

class Subject
{
public:
	// Function parses through certificate to find subject fields
	Subject(CK_BYTE *data);
	~Subject(void);
	// Function check is subject field valid
	bool isValid(){return valid;};
	// Function get Certificate Name
	string getCertName(){return fields[5];};
	// Function get UO Field
	string getOU(){return fields[4];};
	// Function get Certificate Organization
	string getOrganization(){return fields[3];};
	// Function get Certificate Location
	string getLocation(){return fields[2];};
	// Function get Certificate state field
	string getState(){return fields[1];};
	// Function get Certificate country field
	string getCountry(){return fields[0];};
private:
	bool valid;
	/*	
	0 = country
	1 = state
	2 = location
	3 = organization
	4 = OU
	5 = certName
	*/
	vector<string> fields;
};
