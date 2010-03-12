/*
 * Copyright © 2010 Dylan Enloe, Vincent Cao, Muath Alissa
 * ALL RIGHTS RESERVED
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
