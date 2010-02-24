void Verify(string plainText, string keyLabel)

{

	CK_OBJECT_HANDLE key;
	CK_ULONG ulPlainTextLength;
	CK_ULONG ulDigestLen;			
	CK_ULONG ulSignatureLen;								
	CK_BYTE* inBuffer;
	CK_BYTE* verifyBuffer;
	CK_BYTE* digestBuffer;
	CK_MECHANISM mechanism_digest = {CKM_SHA_1, NULL_PTR, 0};
	CK_MECHANISM mechanism_verify = {CKM_RSA_PKCS, NULL_PTR, 0};
	CK_UTF8CHAR* label = (CK_UTF8CHAR*)keyLabel.c_str();
	CK_ATTRIBUTE searchKey[] = {CKA_LABEL, label, sizeof(label) - 1};

	
	//get private keys for signing, verify using public key 
	getPrivateKey(keyLabel, key);



	/* init digest */
	returnValue = (funcList->C_DigestInit)(hSession, &mechanism_digest);
	if(returnValue != CKR_OK)
	{
		setError(COULD_NOT_INIT_DIGEST);
		return " ";
	}

	ulPlainTextLength = sizeof(plainText.c_str());
	inBuffer = (CK_BYTE*)plainText.c_str();
	
	/* do to get length for digest buffer */
	returnValue = (funcList->C_Digest)(hSession, inBuffer, ulPlainTextLength, NULL_PTR, &ulDigestLen);
	if(returnValue != CKR_OK)
	{ 
		setError(FAILED_TO_DIGEST);
		return "";
	}
	
	/* digest data */
	digestBuffer = (CK_BYTE*)malloc(sizeof(CK_BYTE) * ulDigestLen);
	returnValue = (funcList->C_Digest)(hSession, inBuffer, ulPlainTextLength, digestBuffer, &ulDigestLen);
	if(returnValue != CKR_OK)
	{
		setError(FAILED_TO_DIGEST);
		return ""; 
	}


	//1. Initialize verification
	returnValue = (funcList->C_VerifyInit)(hSession, &mechanism_verify, key);
	
	if (returnValue != CKR_OK)
	{
		setError(FAILED_TO_VERIFY);
		return "";
	}	

	//Verifying
	returnValue = (funcList->C_Verify)(hSession, digestBuffer, ulDigestLen, NULL_PTR, ulSignatureLen);
	if(returnValue != CKR_OK)
	{
		setError(FAILED_TO_VERIFY);
		return ""; 
	}


	verifyBuffer = (CK_BYTE*)malloc(sizeof(CK_BYTE) * ulSignatureLen);
	returnValue = (funcList->C_Verify)(hSession, digestBuffer, ulDigestLen, verifyBuffer, ulSignatureLen);
	if(returnValue != CKR_OK)
	{
		setError(FAILED_TO_VERIFY);
		return ""; 
	}

	string output = (char*)verifyBuffer;
	return output;

}