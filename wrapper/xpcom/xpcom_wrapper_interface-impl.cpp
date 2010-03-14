#include "xpcom_wrapper_interface-impl.h"

using namespace std;

NS_IMPL_ISUPPORTS1(nsSPRS_PKCS11_Wrapper, nsISPRS_PKCS11_Wrapper)

/*!
 * \class nsPSRS_PKCS11_Wrapper
 * @brief
 * XPCOM component exposing CryptoWrapper to JavaScript
 */

/*!
 * @brief Constructor for nsSPRS_PKCS11_Warpper
 * @remarks does nothing interesting
 */
nsSPRS_PKCS11_Wrapper::nsSPRS_PKCS11_Wrapper()
{
  /* member initializers and constructor code */
}

/*!
 * @brief Destructor for nsSPRS_PKCS11_Warpper
 * @remarks does nothing interesting
 */
nsSPRS_PKCS11_Wrapper::~nsSPRS_PKCS11_Wrapper()
{
  /* destructor code */
}

/*!
 * @brief calls CryptoWrapper.getLastError()
 * @param [out] _retval error code 
 * @retval NS_IMETHODIMP NS_OK
 * @remarks JavaScript signature: long SPRS_getLastError();
 */
/* long SPRS_getLastError (); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_getLastError(PRInt32 *_retval)
{
	*_retval = wrapper.getLastError();
	return NS_OK;
}

/*!
 * @brief calls CryptoWrapper.initCrypto()
 * @param [out] _retval success/failure of CryptoWrapper.initCrypto()
 * @retval NS_IMETHODIMP NS_OK
 * @remarks JavaScript signature: boolean SPRS_initCrypto();
 */
/* boolean SPRS_initCrypto (); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_initCrypto(PRBool *_retval)
{
    if (wrapper.initCrypto())
        *_retval = PR_TRUE;
    else
        *_retval = PR_FALSE;

    return NS_OK;
}

/*!
 * \brief calls CryptoWrapper.finalizeCrypto()
 *
 * There is no indication of success/failure.
 * 
 * JavaScript signature: void SPRS_finalizeCrypto();
 * @retval NS_IMETHODIMP NS_OK
 * @remarks calls CryptoWrapper.finalizeCrypto()
 */
/* void SPRS_finalizeCrypto (); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_finalizeCrypto()
{
    wrapper.finalizeCrypto();
    return NS_OK; //that probably worked! 
}

/*!
 * \brief calls CryptoWrapper.enumerateCards()
 *
 * JavaScript signature: nsIArray SPRS_enumerateCards();
 *
 * @param [out] count number of cards detected
 * @param [out] cards array of strings of card names
 * @retval NS_IMETHODIMP NS_OK or NS_ERROR_OUT_OF_MEMORY
 * @remarks calls CryptoWrapper.enumerateCards()
 */
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
         if(!out[i])
             return NS_ERROR_OUT_OF_MEMORY;
     }
 
     *count = scount;
     *cards = out;
     return NS_OK;
}

/*!
 * \brief calls CryptoWrapper.selectCard()
 *
 * JavaScript signature: boolean SPRS_selectCard(in nsAString card);
 *
 * @param [in] card zero-based index of card
 * @param [in] pin user-level PIN for card
 * @param [out] _retval success/failure of CryptoWrapper.selectCard()
 * @retval NS_IMETHODIMP NS_OK
 * @remarks calls CryptoWrapper.selectCard()
 */
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

/*!
 * \brief calls CryptoWrapper.listCerts()
 *
 * JavaScript signature: void SPRS_listCerts (out PRUint32 count, [array, size_is (count), retval]out string certs);
 * @param [out] count number of certs detected
 * @param [out] certs array of strings of cert names
 * @retval NS_IMETHODIMP NS_OK or NS_ERROR_OUT_OF_MEMORY
 * @remarks calls CryptoWrapper.listCerts()
 */
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
         if(!out[i])
             return NS_ERROR_OUT_OF_MEMORY;
     }
 
     *count = scount;
     *certs = out;
     return NS_OK;
}


/*!
 * JavaScript signature: boolean signFile(in nsAString input_file, in nsAString output_file, in nsAString cert);
 * @param [in] input name of input file
 * @param [in] output name of output file
 * @param [in] cert name of certificate to use for encryption
 * @param [out] _retval success/failure of CryptoWrapper.encryptFile()
 * @retval NS_IMETHODIMP NS_OK
 * @remarks calls CryptoWrapper.encryptFile()
 */
/* boolean sprs_encryptfile (in nsastring input, in nsastring output_file, in nsastring cert); */
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

/*!
 *
 * JavaScript signature: boolean signFile (in nsAString input_file, in nsAString output_file, in nsAString cert);
 * @param [in] input_file name of input file
 * @param [in] output_file name of output file
 * @param [in] cert name of certificate to use for signing
 * @param [out] _retval success/failure of CryptoWrapper.signFile()
 * @retval NS_IMETHODIMP NS_OK
 * @remarks calls CryptoWrapper.signFile()
 */
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

/*!
 *
 * JavaScript signature: boolean SPRS_loadFile (in AString input_file, out AString output);
 * @param [in] input_file name of file to load
 * @param [out] output string containing cleartext of input file
 * @param [out] _retval success/failure of CryptoWrapper.loadFile()
 * @retval NS_IMETHODIMP NS_OK
 * @remarks calls CryptoWrapper.loadFile()
 */
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

/*!
 * JavaScript signature: long SPRS_getTokenCount();
 * @param PRInt32 *_retval number of cards/tokens detected
 * @retval NS_IMETHODIMP NS_OK
 * @remarks calls CryptoWrapper.getTokenCount()
 */
/* long SPRS_getTokenCount (); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_getTokenCount(PRInt32 *_retval)
{
    *_retval = wrapper.getTokenCount();
    return NS_OK;
}

/*!
 * \brief not implemented
 *
 * @retval NS_IMETHODIMP NS_ERROR_NOT_IMPLEMENTED
 * @remarks Calling this will result in a not implemented exception!
 */
/* nsAString SPRS_verify (in nsAString input_file); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_verify(const nsAString & input_file, nsAString **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}


/*!
 * \brief not implemented
 *
 * @retval NS_IMETHODIMP NS_ERROR_NOT_IMPLEMENTED
 * @remarks Calling this will result in a not implemented exception!
 */
/* boolean SPRS_createCert (in nsAString cert); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_createCert(const nsAString & cert, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/*!
 * \brief not implemented
 *
 * @retval NS_IMETHODIMP NS_ERROR_NOT_IMPLEMENTED
 * @remarks Calling this will result in a not implemented exception!
 */
/* nsAString SPRS_decrypt (in nsAString input_file); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_decrypt(const nsAString & input_file, nsAString **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/*!
 * \brief Deemonstrates XPIDL array support is in fact working with SDK
 *
 * This is not part of the wrapper API, it's just a sanity check.
 * Returns an array with 10 elements.
 * JavaScript signature: 
 * void getArray (out unsigned long count, [array, size_is (count), retval] out long retv); 
 *
 * This is called from JavaScript like this:
 * 			var count = {};
 * 			var arr = obj.getArray(count);
 * 			alert("Count.value: " + count.value);
 * 			alert("Cards: " + arr);
 * 
 * @retval NS_IMETHODIMP NS_OK
 * @remarks This is not part of the wrapper API, it's just a sanity check.
 */
/* void getArray (out unsigned long count, [array, size_is (count), retval] out long retv); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::GetArray(PRUint32 *count, PRInt32 **retv)
{
	*count = 10;
	*retv = (PRInt32*)nsMemory::Alloc(*count * sizeof(PRInt32));
	for (int i = 0; i < 10; ++i) (*retv)[i] = i;

    return NS_OK;
}
