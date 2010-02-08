#include "xpcom_wrapper_interface-impl.h"
#include "wrapper.h"

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
	*_retval = getLastError();
	return NS_OK;
}

/* boolean SPRS_initCrypto (); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_initCrypto(PRBool *_retval)
{
	if (initCrypto()){
		*_retval = true;
		return NS_OK;
	}
	else{
		*_retval = false;
		return NS_OK; //not the sort of failure we'd handle with a non-ok return code here, would make JS sad
	}
}

/* void SPRS_finalizeCrypto (); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_finalizeCrypto()
{
	finalizeCrypto();
    return NS_OK; //that probably worked! 
}

/* nsIArray SPRS_enumerateCards (); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_enumerateCards(nsIArray **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean SPRS_selectCard (in nsAString card); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_selectCard(nsAString *card, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIArray SPRS_listCerts (); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_listCerts(nsIArray **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean SPRS_createCert (in nsAString cert); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_createCert(nsAString *cert, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean SPRS_encryptFile (in nsAString input, in nsAString output_file, in nsAString cert); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_encryptFile(nsAString *input, nsAString *output_file, nsAString *cert, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean signFile (in nsAString input_file, in nsAString output_file, in nsAString cert); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SignFile(nsAString *input_file, nsAString *output_file, nsAString *cert, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsAString SPRS_decrypt (in nsAString input_file); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_decrypt(nsAString *input_file, nsAString **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsAString SPRS_verify (in nsAString input_file); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_verify(nsAString *input_file, nsAString **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}