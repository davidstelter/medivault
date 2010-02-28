/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM xpcom_wrapper_interface.idl
 */

#ifndef __gen_xpcom_wrapper_interface_h__
#define __gen_xpcom_wrapper_interface_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif
class nsIArray; /* forward declaration */

class nsAString; /* forward declaration */


/* starting interface:    nsISPRS_PKCS11_Wrapper */
#define NS_ISPRS_PKCS11_WRAPPER_IID_STR "b23e4e97-4e78-4da5-bf18-1accf19361db"

#define NS_ISPRS_PKCS11_WRAPPER_IID \
  {0xb23e4e97, 0x4e78, 0x4da5, \
    { 0xbf, 0x18, 0x1a, 0xcc, 0xf1, 0x93, 0x61, 0xdb }}

class NS_NO_VTABLE NS_SCRIPTABLE nsISPRS_PKCS11_Wrapper : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(NS_ISPRS_PKCS11_WRAPPER_IID)

  /* long SPRS_getLastError (); */
  NS_SCRIPTABLE NS_IMETHOD SPRS_getLastError(PRInt32 *_retval) = 0;

  /* boolean SPRS_initCrypto (); */
  NS_SCRIPTABLE NS_IMETHOD SPRS_initCrypto(PRBool *_retval) = 0;

  /* void SPRS_finalizeCrypto (); */
  NS_SCRIPTABLE NS_IMETHOD SPRS_finalizeCrypto(void) = 0;

  /* nsIArray SPRS_enumerateCards (); */
  NS_SCRIPTABLE NS_IMETHOD SPRS_enumerateCards(nsIArray **_retval) = 0;

  /* boolean SPRS_selectCard (in long card, in AString pin); */
  NS_SCRIPTABLE NS_IMETHOD SPRS_selectCard(PRInt32 card, const nsAString & pin, PRBool *_retval) = 0;

  /* nsIArray SPRS_listCerts (); */
  NS_SCRIPTABLE NS_IMETHOD SPRS_listCerts(nsIArray **_retval) = 0;

  /* boolean SPRS_createCert (in AString cert); */
  NS_SCRIPTABLE NS_IMETHOD SPRS_createCert(const nsAString & cert, PRBool *_retval) = 0;

  /* boolean SPRS_encryptFile (in AString input, in AString output_file, in AString cert); */
  NS_SCRIPTABLE NS_IMETHOD SPRS_encryptFile(const nsAString & input, const nsAString & output_file, const nsAString & cert, PRBool *_retval) = 0;

  /* boolean signFile (in AString input_file, in AString output_file, in AString cert); */
  NS_SCRIPTABLE NS_IMETHOD SignFile(const nsAString & input_file, const nsAString & output_file, const nsAString & cert, PRBool *_retval) = 0;

  /* nsAString SPRS_decrypt (in AString input_file); */
  NS_SCRIPTABLE NS_IMETHOD SPRS_decrypt(const nsAString & input_file, nsAString **_retval) = 0;

  /* nsAString SPRS_verify (in AString input_file); */
  NS_SCRIPTABLE NS_IMETHOD SPRS_verify(const nsAString & input_file, nsAString **_retval) = 0;

  /* long SPRS_getTokenCount (); */
  NS_SCRIPTABLE NS_IMETHOD SPRS_getTokenCount(PRInt32 *_retval) = 0;

};

  NS_DEFINE_STATIC_IID_ACCESSOR(nsISPRS_PKCS11_Wrapper, NS_ISPRS_PKCS11_WRAPPER_IID)

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISPRS_PKCS11_WRAPPER \
  NS_SCRIPTABLE NS_IMETHOD SPRS_getLastError(PRInt32 *_retval); \
  NS_SCRIPTABLE NS_IMETHOD SPRS_initCrypto(PRBool *_retval); \
  NS_SCRIPTABLE NS_IMETHOD SPRS_finalizeCrypto(void); \
  NS_SCRIPTABLE NS_IMETHOD SPRS_enumerateCards(nsIArray **_retval); \
  NS_SCRIPTABLE NS_IMETHOD SPRS_selectCard(PRInt32 card, const nsAString & pin, PRBool *_retval); \
  NS_SCRIPTABLE NS_IMETHOD SPRS_listCerts(nsIArray **_retval); \
  NS_SCRIPTABLE NS_IMETHOD SPRS_createCert(const nsAString & cert, PRBool *_retval); \
  NS_SCRIPTABLE NS_IMETHOD SPRS_encryptFile(const nsAString & input, const nsAString & output_file, const nsAString & cert, PRBool *_retval); \
  NS_SCRIPTABLE NS_IMETHOD SignFile(const nsAString & input_file, const nsAString & output_file, const nsAString & cert, PRBool *_retval); \
  NS_SCRIPTABLE NS_IMETHOD SPRS_decrypt(const nsAString & input_file, nsAString **_retval); \
  NS_SCRIPTABLE NS_IMETHOD SPRS_verify(const nsAString & input_file, nsAString **_retval); \
  NS_SCRIPTABLE NS_IMETHOD SPRS_getTokenCount(PRInt32 *_retval); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISPRS_PKCS11_WRAPPER(_to) \
  NS_SCRIPTABLE NS_IMETHOD SPRS_getLastError(PRInt32 *_retval) { return _to SPRS_getLastError(_retval); } \
  NS_SCRIPTABLE NS_IMETHOD SPRS_initCrypto(PRBool *_retval) { return _to SPRS_initCrypto(_retval); } \
  NS_SCRIPTABLE NS_IMETHOD SPRS_finalizeCrypto(void) { return _to SPRS_finalizeCrypto(); } \
  NS_SCRIPTABLE NS_IMETHOD SPRS_enumerateCards(nsIArray **_retval) { return _to SPRS_enumerateCards(_retval); } \
  NS_SCRIPTABLE NS_IMETHOD SPRS_selectCard(PRInt32 card, const nsAString & pin, PRBool *_retval) { return _to SPRS_selectCard(card, pin, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD SPRS_listCerts(nsIArray **_retval) { return _to SPRS_listCerts(_retval); } \
  NS_SCRIPTABLE NS_IMETHOD SPRS_createCert(const nsAString & cert, PRBool *_retval) { return _to SPRS_createCert(cert, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD SPRS_encryptFile(const nsAString & input, const nsAString & output_file, const nsAString & cert, PRBool *_retval) { return _to SPRS_encryptFile(input, output_file, cert, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD SignFile(const nsAString & input_file, const nsAString & output_file, const nsAString & cert, PRBool *_retval) { return _to SignFile(input_file, output_file, cert, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD SPRS_decrypt(const nsAString & input_file, nsAString **_retval) { return _to SPRS_decrypt(input_file, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD SPRS_verify(const nsAString & input_file, nsAString **_retval) { return _to SPRS_verify(input_file, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD SPRS_getTokenCount(PRInt32 *_retval) { return _to SPRS_getTokenCount(_retval); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISPRS_PKCS11_WRAPPER(_to) \
  NS_SCRIPTABLE NS_IMETHOD SPRS_getLastError(PRInt32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->SPRS_getLastError(_retval); } \
  NS_SCRIPTABLE NS_IMETHOD SPRS_initCrypto(PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->SPRS_initCrypto(_retval); } \
  NS_SCRIPTABLE NS_IMETHOD SPRS_finalizeCrypto(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->SPRS_finalizeCrypto(); } \
  NS_SCRIPTABLE NS_IMETHOD SPRS_enumerateCards(nsIArray **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->SPRS_enumerateCards(_retval); } \
  NS_SCRIPTABLE NS_IMETHOD SPRS_selectCard(PRInt32 card, const nsAString & pin, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->SPRS_selectCard(card, pin, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD SPRS_listCerts(nsIArray **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->SPRS_listCerts(_retval); } \
  NS_SCRIPTABLE NS_IMETHOD SPRS_createCert(const nsAString & cert, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->SPRS_createCert(cert, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD SPRS_encryptFile(const nsAString & input, const nsAString & output_file, const nsAString & cert, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->SPRS_encryptFile(input, output_file, cert, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD SignFile(const nsAString & input_file, const nsAString & output_file, const nsAString & cert, PRBool *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->SignFile(input_file, output_file, cert, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD SPRS_decrypt(const nsAString & input_file, nsAString **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->SPRS_decrypt(input_file, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD SPRS_verify(const nsAString & input_file, nsAString **_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->SPRS_verify(input_file, _retval); } \
  NS_SCRIPTABLE NS_IMETHOD SPRS_getTokenCount(PRInt32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->SPRS_getTokenCount(_retval); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsSPRS_PKCS11_Wrapper : public nsISPRS_PKCS11_Wrapper
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISPRS_PKCS11_WRAPPER

  nsSPRS_PKCS11_Wrapper();

private:
  ~nsSPRS_PKCS11_Wrapper();

protected:
  /* additional members */
};

/* Implementation file */
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
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean SPRS_initCrypto (); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_initCrypto(PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void SPRS_finalizeCrypto (); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_finalizeCrypto()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIArray SPRS_enumerateCards (); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_enumerateCards(nsIArray **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean SPRS_selectCard (in long card, in AString pin); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_selectCard(PRInt32 card, const nsAString & pin, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIArray SPRS_listCerts (); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_listCerts(nsIArray **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean SPRS_createCert (in AString cert); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_createCert(const nsAString & cert, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean SPRS_encryptFile (in AString input, in AString output_file, in AString cert); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_encryptFile(const nsAString & input, const nsAString & output_file, const nsAString & cert, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean signFile (in AString input_file, in AString output_file, in AString cert); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SignFile(const nsAString & input_file, const nsAString & output_file, const nsAString & cert, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsAString SPRS_decrypt (in AString input_file); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_decrypt(const nsAString & input_file, nsAString **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsAString SPRS_verify (in AString input_file); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_verify(const nsAString & input_file, nsAString **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* long SPRS_getTokenCount (); */
NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_getTokenCount(PRInt32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_xpcom_wrapper_interface_h__ */
