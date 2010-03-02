#include <string>
#include <vector>
#include "cryptoki.h"

#pragma once

using namespace std;

class Subject
{
public:
	Subject(CK_BYTE *data);
	~Subject(void);
	bool isValid(){return valid;};
	string getCertName(){return fields[5];};
	string getOU(){return fields[4];};
	string getOrganization(){return fields[3];};
	string getLocation(){return fields[2];};
	string getState(){return fields[1];};
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
