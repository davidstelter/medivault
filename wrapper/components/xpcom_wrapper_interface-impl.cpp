#include "xpcom_wrapper_interface-impl.h"

using namespace std;

NS_IMPL_ISUPPORTS1(nsSPRS_PKCS11_Wrapper, nsISPRS_PKCS11_Wrapper)

nsSPRS_PKCS11_Wrapper::nsSPRS_PKCS11_Wrapper()
{
  /* member initializers and constructor code */
}

nsSPRS_PKCS11_Wrapper::~nsSPRS_PKCS11_Wrapper()
{
  /* destructor code */
}

/* long SPRS_getLastError (); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_getLastError(PRInt32 *_retval)
{
	*_retval = wrapper.getLastError();
	return NS_OK;
}

/* boolean SPRS_initCrypto (); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_initCrypto(PRBool *_retval)
{

	if (wrapper.initCrypto()){
		*_retval = PR_TRUE;
		return NS_OK;
	}
	else{
		*_retval = PR_FALSE;
		return NS_OK; //not the sort of failure we'd handle with a non-ok return code here, would make JS sad
	}
}

/* void SPRS_finalizeCrypto (); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_finalizeCrypto()
{
	wrapper.finalizeCrypto();
    return NS_OK; //that probably worked! 
}

/* nsIArray SPRS_enumerateCards (); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_enumerateCards(PRUint32 *count, char ***cards)
{
	string* strings = wrapper.enumerateCards();
	const static PRUint32 scount = wrapper.getTokenCount();

    char** out = (char**) nsMemory::Alloc(scount * sizeof(char*));
    if(!out)
        return NS_ERROR_OUT_OF_MEMORY;

     for(PRUint32 i = 0; i < scount; ++i)
	 {
         out[i] = (char*) nsMemory::Clone(strings[i].c_str(), strlen(strings[i].c_str())+1);
         // failure unlikely, leakage foolishly tolerated in this test case
         if(!out[i])
             return NS_ERROR_OUT_OF_MEMORY;
     }
 
     *count = scount;
     *cards = out;
     return NS_OK;
}

/* void getArray (out unsigned long count, [array, size_is (count), retval] out long retv); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::GetArray(PRUint32 *count, PRInt32 **retv)
{
	*count = 10;
	*retv = (PRInt32*)nsMemory::Alloc(*count * sizeof(PRInt32));
	for (int i = 0; i < 10; ++i) (*retv)[i] = i;

    return NS_OK;
}

/* boolean SPRS_selectCard (in nsAString card); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_selectCard(PRInt32 card, const nsAString & pin, PRBool *_retval)
{
	CK_UTF8CHAR* UserPIN = (CK_UTF8CHAR*)malloc(sizeof(CK_UTF8CHAR) * (pin.Length()+1));
	const PRUnichar* cur = pin.BeginReading();
	const PRUnichar* end = pin.EndReading();
	int i;
	for(i=0; cur < end; ++cur, ++i){
		UserPIN[i] = (CK_UTF8CHAR)*cur;
	}
	UserPIN[pin.Length()] = 0;

	
	if(wrapper.selectCard(card, UserPIN, pin.Length()))
		*_retval = PR_TRUE;
	else
		*_retval = PR_FALSE;
	
	
    return NS_OK;
}

/* void SPRS_listCerts (out PRUint32 count, [array, size_is (count), retval] out string certs); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_listCerts(PRUint32 *count, char ***certs)
{
	vector<string> keyVec = wrapper.listKeys();
	static PRUint32 scount = (PRUint32) keyVec.size();

    char** out = (char**) nsMemory::Alloc(scount * sizeof(char*));
    if(!out)
        return NS_ERROR_OUT_OF_MEMORY;

     for(PRUint32 i = 0; i < scount; ++i)
	 {
         out[i] = (char*) nsMemory::Clone(keyVec[i].c_str(), strlen(keyVec[i].c_str())+1);
         // failure unlikely, leakage foolishly tolerated in this test case
         if(!out[i])
             return NS_ERROR_OUT_OF_MEMORY;
     }
 
     *count = scount;
     *certs = out;
     return NS_OK;
}

/* boolean SPRS_createCert (in nsAString cert); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_createCert(const nsAString & cert, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean SPRS_encryptFile (in nsAString input, in nsAString output_file, in nsAString cert); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_encryptFile(const nsAString & input, const nsAString & output, const nsAString & cert, PRBool *_retval)
{

	nsCString inCString;
	nsCString outCString;
	nsCString certCString;
	NS_UTF16ToCString(input,  NS_CSTRING_ENCODING_UTF8, inCString);
	NS_UTF16ToCString(output, NS_CSTRING_ENCODING_UTF8, outCString);
	NS_UTF16ToCString(cert,   NS_CSTRING_ENCODING_UTF8, certCString);
	
	string sInfile;
	string sOutfile;
	string sCert;
	sInfile.assign(inCString.get());
	sOutfile.assign(outCString.get());
	sCert.assign(certCString.get());
	
	if(wrapper.encryptFile(sInfile, sOutfile, sCert))
		*_retval = PR_TRUE;
	else
		*_retval = PR_FALSE;
	
	
    return NS_OK;
}

/* boolean signFile (in nsAString input_file, in nsAString output_file, in nsAString cert); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_signFile(const nsAString & input_file, const nsAString & output_file, const nsAString & cert, PRBool *_retval)
{
	nsCString inCString;
	nsCString outCString;
	nsCString certCString;
	NS_UTF16ToCString(input_file,  NS_CSTRING_ENCODING_UTF8, inCString);
	NS_UTF16ToCString(output_file, NS_CSTRING_ENCODING_UTF8, outCString);
	NS_UTF16ToCString(cert,   NS_CSTRING_ENCODING_UTF8, certCString);
	
	string sInfile;
	string sOutfile;
	string sCert;
	sInfile.assign(inCString.get());
	sOutfile.assign(outCString.get());
	sCert.assign(certCString.get());
	
	if(wrapper.signFile(sInfile, sOutfile, sCert))
		*_retval = PR_TRUE;
	else
		*_retval = PR_FALSE;
	
    return NS_OK;
}

/* nsAString SPRS_decrypt (in nsAString input_file); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_decrypt(const nsAString & input_file, nsAString **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean SPRS_loadFile (in AString input_file, out AString output); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_loadFile(const nsAString & input_file, nsAString & output, PRBool *_retval)
{
	//LoadFile uses empty string to indicate error, so we need to check
	//the error code in the event of an empty string
	wrapper.clearError();

	nsCString inCString;
	nsCString outCString;
	NS_UTF16ToCString(input_file,  NS_CSTRING_ENCODING_UTF8, inCString);
	
	string sInfile;
	string sClear;
	sInfile.assign(inCString.get());
	
	sClear = wrapper.LoadFile(sInfile);
	if(sClear.length()>0){
		outCString.Assign(sClear.c_str());
		NS_CStringToUTF16(outCString, NS_CSTRING_ENCODING_ASCII, output);
		*_retval = PR_TRUE;
	}
	else{
		if(wrapper.getLastError()==OK)
			//no error, plaintext must really be empty string...
			*_retval = PR_TRUE;
		else
			*_retval = PR_FALSE;
	}
  return NS_OK;
}

/* nsAString SPRS_verify (in nsAString input_file); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_verify(const nsAString & input_file, nsAString **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* long SPRS_getTokenCount (); */
/* returns number of tokens recognized in the system*/
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_getTokenCount(PRInt32 *_retval)
{
	
	//CK_ULONG ulSlotCount;			//Number of connected readers
		//returnValue = (funcList->C_GetSlotList)(TRUE,NULL_PTR,&ulSlotCount);
		*_retval = wrapper.getTokenCount();//ulSlotCount;
	
	//*_retval = 9;
	return NS_OK;
}

/* protected wrapper methods *//*
bool nsSPRS_PKCS11_Wrapper::initCrypto() {
	if(PKCSLibraryModule) {
		setError(ALREADY_LOADED);
		return false;
	}
	//Load the DLL file.
	if (!(PKCSLibraryModule = LoadLibrary(L".\\acospkcs11.dll"))) {
		setError(DLL_LOAD_ERROR);
		return false;
	}
	//Get the function list
	CK_C_GetFunctionList pC_GetFunctionList = (CK_C_GetFunctionList)
		GetProcAddress(PKCSLibraryModule, "C_GetFunctionList");

	returnValue = (*pC_GetFunctionList)(&funcList);
	if (returnValue != CKR_OK) {
		setError(NO_FUNC_LIST);
		FreeLibrary(PKCSLibraryModule);
		PKCSLibraryModule = 0;  //set library to zero to be safe
		return false;
	}
	//initialize the cryptoEngine
	returnValue = (funcList->C_Initialize)(NULL_PTR);
	if (returnValue != CKR_OK) {
		setError(INIT_FAILED);
		FreeLibrary(PKCSLibraryModule);
		PKCSLibraryModule = 0;  //set library to zero to be safe
		return false;
	}
	return true;
}

void nsSPRS_PKCS11_Wrapper::finalizeCrypto() {
	if(!PKCSLibraryModule) {
		return;  //if there is nothing loaded we are done
	}
	funcList->C_Finalize(NULL_PTR);
	FreeLibrary(PKCSLibraryModule);
	PKCSLibraryModule = 0;  //set library to zero to be safe
}

int nsSPRS_PKCS11_Wrapper::getTokenCount(){
	CK_ULONG ulSlotCount;			//Number of connected readers
	//Search readers to get count
		returnValue = (funcList->C_GetSlotList)(TRUE,NULL_PTR,&ulSlotCount);
	return ulSlotCount;
}

int nsSPRS_PKCS11_Wrapper::getLastError(void){
	return m_lastError;
}

void nsSPRS_PKCS11_Wrapper::setError(int errorcode){
	m_lastError = errorcode;
}
*/
/*
//Get list of all slots with a token present//Get list of all slots with a token present
string *nsSPRS_PKCS11_Wrapper::enumerateCards(void)
{
	CK_SLOT_INFO_PTR pSlotInfo;				//Pointer to slot info
	CK_ULONG ulSlotWithTokenCount;			//Number of connected readers

	pSlotWithTokenList = (CK_SLOT_ID_PTR) malloc(0);		
	ulSlotWithTokenCount = 0;
	string	*SlotsArray= new string[TokenCount];			// array that contains slots info (return value)



	while (1)
		{
			//Search readers and store result in pSlotWithTokenList;
			returnValue = (funcList->C_GetSlotList)(CK_TRUE, pSlotWithTokenList, &ulSlotWithTokenCount);

			if (returnValue != CKR_BUFFER_TOO_SMALL)
				break;

			pSlotWithTokenList = (unsigned long *)malloc(sizeof(CK_SLOT_ID)*ulSlotWithTokenCount);
		}

	if (returnValue == CKR_OK && ulSlotWithTokenCount > 0) 
	{
			
			pSlotInfo = (CK_SLOT_INFO_PTR)malloc(sizeof(CK_SLOT_INFO));

			// Get all the slots info
			for(unsigned int i = 0; i < ulSlotWithTokenCount; i++)
			{
				returnValue = (funcList->C_GetSlotInfo)(pSlotWithTokenList[i],pSlotInfo);
				if(returnValue == CKR_OK)
				{
					SlotsArray[i] = ((char*)pSlotInfo->slotDescription);
					//printf("Slot %d: ",i);
					//cout << SlotsArray[i] << endl; // somehow it doesn't print with printf()!
				}
			}
	}
	else
		setError(NO_DEVICES_FOUND);

		//returns list of all cards on the system. An empty list indicates an error;
		return SlotsArray;
}
*/
/*
//Selects a card to use for subsequent operations.  Returns false on failure and sets 
bool nsSPRS_PKCS11_Wrapper::selectCard(long SlotID, const nsAString & pin )
{
	//Open session for selected card
		returnValue = (funcList->C_OpenSession)(pSlotWithTokenList[SlotID],
						CKF_SERIAL_SESSION| CKF_RW_SESSION, NULL, NULL, &hSession);

		if (returnValue == CKR_OK) 
		{
			//CK_UTF8CHAR UserPIN[] = "12345678";
			CK_UTF8CHAR* UserPIN = (CK_UTF8CHAR*)malloc(sizeof(CK_UTF8CHAR) * (pin.Length()+1));
			const PRUnichar* cur = pin.BeginReading();
			const PRUnichar* end = pin.EndReading();
			int i;
			for(i=0; cur < end; ++cur, ++i){
				UserPIN[i] = (CK_UTF8CHAR)*cur;
			}
			UserPIN[pin.Length()] = 0;
			
			returnValue = (funcList->C_Login)(hSession, CKU_USER, UserPIN, pin.Length());

			if (returnValue != CKR_OK) 
			{
				setError(WRONG_PASSWORD);
				return false;
			}
			else				
				return true;
		}
		else
		{
			setError(OPEN_SESSION_FAILED);
			return false;
		}
}
*/
