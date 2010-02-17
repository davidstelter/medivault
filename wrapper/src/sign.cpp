string sign( string plainText, string keyLabel )
{
	CK_OBJECT_HANDLE key;
	CK_ULONG numKeyFound;
	CK_ULONG ulPlainTextLength;
	CK_ULONG ulDigestLen;			
	CK_ULONG ulSignatureLen;								
	CK_BYTE* inBuffer;
	CK_BYTE* signBuffer;
	CK_BYTE* digestBuffer;
	CK_MECHANISM mechanism_digest = {CKN_SHA_1, NULL_PTR, 0};
	CK_MECHANISM mechanism_sign = {CKN_RSA_PKCS, NULL_PTR, 0};
	CK_UFT8CHAR* label = {CK_UTF8CHAR*}keyLabel.c_str();
	CK_ATTRIBUTE searchKey[] = {CKA_LABEL, label, sizeof(label) - 1};

	/* get private keys for signing, verify using public key */
	getPrivateKey(keyLabel, key);
	
	/* init digest */
	returnValue = (funcList->C_DigestInit)(hSession, &mechanism_digest);
	if(returnValue != CKR_OK)
	{
		setError(COULD_NOT_INIT_DIGEST);
		return"";
	}

	ulPlainTextLength = sizeof(plainText.c_str());
	inBuffer = plainText.c_str();
	
	/* do to get length for digest buffer */
	returnValue = (pFnList->C_Digest)(hSession, inBuffer, ulPlainTextLength, NULL_PTR, &ulDigestLen);
	if(returnValue != CKR_OK)
	{ 
		setError(FAILED_TO_DIGEST);
		return "";
	}
	
	/* digest data */
	digestBuffer = (CK_BYTE*)malloc(sizeof(CK_BYTE) * ulDigestLen);
	returnValue = (pFnList->C_Digest)(hSession, inBuffer, ulPlainTextLength, digestBuffer, &ulDigestLen);
	if(returnValue != CKR_OK)
	{
		setError(FAILED_TO_DIGEST);
		return ""; 
	}
	
	/* init sign function */
	returnValue = (funcList->C_SignInit)(hSession, &mechanism_sign, key);
	if(returnValue != CKR_OK)
	{
		setError(COULD_NOT_INIT_SIGN);
		return ""; 
	}
	
	/* get length for sign buffer */
	returnValue = (pFnList->C_Sign)(hSession, digestBuffer, ulDigestLen, NULL_PTR, &ulSignatureLen);
	if(returnValue != CKR_OK)
	{
		setError(FAILED_TO_SIGN);
		return ""; 
	}
	
	/* sign */
	signBuffer = (CK_BYTE*)malloc(sizeof(CK_BYTE) * ulSignatureLen);
	returnValue = (pFnList->C_Sign)(hSession, digestBuffer, ulDigestLen, signBuffer, &ulSignatureLen);
	if(returnValue != CKR_OK)
	{
		setError(FAILED_TO_SIGN);
		return ""; 
	}
	
	string output = (char*)signBuffer;
	return output;
}

bool signFile( string fileToSign, string signedFile, string strKeyLabel )
{
	ifstream fileRead;
	ofstream fileWrite;
	string strSigned;
	string strToDigestSign;
	stringstream inputStream;
	
	fileRead.exceptions ( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
	fileWrite.exceptions ( ofstream::eofbit | ofstream::failbit | ofstream::badbit );

	try
	{
		fileRead.open( fileToSign.c_str() );
		if (failRead.fail()) 
		{ 
#ifdef DEBUG_STDOUT_MSGS
			cout << "Failed to open file " << plainTextFile << endl;
#endif
			return false;
		}
		inputStream << fileRead.rdbuf();
		strToDigestSign = inputStream.str();

		fileRead.close();
	}
	catch (ifstream::failure e)
	{
#ifdef DEBUG_STDOUT_MSGS
		cout << "Exception opening/reading file";
#endif
		setError(FAILED_TO_OPEN_READ_FILE);
#ifndef SURVIVE_READ_FAIL
	       	return	false;
#endif
	}
#ifdef DEBUG_STDOUT_MSGS
	cout << "Read file: " << strToDigestSign << endl;
#endif

	strSigned = sign(strToDigestSign, strKeyLabel);
	
	if (strSigned == "")
		return false;

	try 
	{
		fileWrite.open( signedFile.c_str(), fstream::in | fstream::out | fstream::app );
		
		/* write stream to file, APPEND MODE */
		fileWrite << strToDigestSign;
		fileWrite << "<type>signature</type>";
		fileWrite << "<cert>" << strKeyLabel.c_str() << "</cert>";
		fileWrite << "<sign>" << strSigned.c_str() << "</sign>";
		
		fileWrite.close();
	}
	catch (ofstream:: failure e) 
	{
#ifdef DEBUG_STDOUT_MSGS
		cout << "Exception opening/writing to file";
#endif
		setError(FAILED_TO_OPEN_WRITE_FILE);
		return false;
	}

	return true;
}



