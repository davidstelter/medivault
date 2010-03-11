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
