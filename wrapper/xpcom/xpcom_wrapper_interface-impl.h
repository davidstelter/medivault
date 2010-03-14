/*
 * Copyright © 2010 David Stelter
 * ALL RIGHTS RESERVED
 */

#ifndef __SPRS_PKCS11_WRAPPER_IMPL_H__
#define __SPRS_PKCS11_WRAPPER_IMPL_H__

#include "xpcom_wrapper_interface.h"
#include "nsStringAPI.h"
#include "nsMemory.h"
#include <stdio.h>
#include <windows.h>
#include <string>
#include <iostream>

#include "..\..\wrapper\src\CryptoWrapper.h"

using namespace std;

#define SPRS_PKCS11_WRAPPER_CONTRACTID "@capstone.pdx.edu/sprs;1"
#define SPRS_PKCS11_WRAPPER_CLASSNAME "SPRS_PKCS11_Wrapper"
#define SPRS_PKCS11_WRAPPER_CID {0xc0a65ae3, 0x43d1, 0x4917, \
    { 0xb5, 0x05, 0x28, 0xd8, 0xc3, 0x29, 0xc8, 0x26 }}

class nsSPRS_PKCS11_Wrapper : public nsISPRS_PKCS11_Wrapper
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISPRS_PKCS11_WRAPPER

  nsSPRS_PKCS11_Wrapper();

  //these public members are handled by the above IDL wizardry, 
  //they only need to be listed here to make Doxygen happy.
  ~nsSPRS_PKCS11_Wrapper();
  NS_IMETHODIMP  SPRS_getLastError(PRInt32 *_retVal);
  NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_initCrypto(PRBool *_retval);
  NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_finalizeCrypto();
  NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_enumerateCards(PRUint32 *count, char ***cards);
  NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_selectCard(PRInt32 card, const nsAString & pin, PRBool *_retval);
  NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_listCerts(PRUint32 *count, char ***certs);
  NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_encryptFile(const nsAString & input, const nsAString & output, const nsAString & cert, PRBool *_retval);
  NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_signFile(const nsAString & input_file, const nsAString & output_file, const nsAString & cert, PRBool *_retval);
  NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_loadFile(const nsAString & input_file, nsAString & output, PRBool *_retval);
  NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_getTokenCount(PRInt32 *_retval);
  NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_verify(const nsAString & input_file, nsAString **_retval);
  NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_createCert(const nsAString & cert, PRBool *_retval);
  NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::SPRS_decrypt(const nsAString & input_file, nsAString **_retval);
  NS_IMETHODIMP nsSPRS_PKCS11_Wrapper::GetArray(PRUint32 *count, PRInt32 **retv);

private:
	CryptoWrapper wrapper;
	
  ~nsSPRS_PKCS11_Wrapper();

protected:
	void setError(int error);
	bool initCrypto();
	void finalizeCrypto();
	int getLastError(void);

	int getTokenCount();
	bool selectCard(long slotID, const nsAString & pin);
	string *enumerateCards(void);
};

#endif
