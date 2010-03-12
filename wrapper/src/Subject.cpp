/*
 * Copyright © 2010 Dylan Enloe, Vincent Cao, Muath Alissa
 * ALL RIGHTS RESERVED
 *
 * Subject.cpp
 * Contains the implementation of the constructor and destructor for the 
 * subject class.
 * */

#include <string>
#include "cryptoki.h"
#include "Subject.h"

using namespace std;

/*!
*	\class Subject
*	@brief
*	A parser to traverse through and parse a given Certificate for subject fields
*/

/*!
*	@brief
*	Functions for parsing out subject fields from x509 Certificates
*	Subject() do the following:
*		- Parse the certificate
*	@param [in] *data Certificate bytes to parse through
*	@retval NULL
*	@remarks
* 	Parse though data and match up certian values to guess whether we have valid subject data
*/

/*	This parses the DER encoded subject.  In the DER encoded string the value 
	0x04 followed by any value followed by 0x13 means that you have a string
	literal.  the next value is the length of that string literal and following
	the length there is a string proper of that length.
	
	We know that there are exactly 6 of these in a proper subject field, no more
	no less.  So this function searchs through the data stream for 6 of these 
	strings.  Knowing that they are always stored in a certain order we can then
	load them into the class.
	
	This is by no means a good solution, but given the time constrants of the 
	project and the shear arcaneness of the specs for this, it is the best that
	we could come up with.*/

Subject::Subject(CK_BYTE *data) : fields(6)
{
	int i=0;
	int j=0;
	while(i < 6) {
		//if the value is 0x04 then we might have a field
		if(data[j] == 0x04) {
			//Skip the length field and if we find a 0x13 we have a string
			if(data[j+2] == 0x13) {
				for(int k = 0; k < data[j+3]; k++) {
					char temp = (char)(data[j+4+k]);
					fields[i] = fields[i] + temp;
				}
				i++;
			}
		}
		j++;
	}
}

/*!
*	@brief
*	Subject deconstructor
*	@param void
*	@retval
*	@remarks Subject class deconstructor
*/
Subject::~Subject(void)
{
}
