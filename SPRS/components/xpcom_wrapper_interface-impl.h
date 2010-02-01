#ifndef __SPRS_PKCS11_WRAPPER_IMPL_H__
#define __SPRS_PKCS11_WRAPPER_IMPL_H__

#include "xpcom_wrapper_interface.h"
#include "nsStringAPI.h"

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

private:
  ~nsSPRS_PKCS11_Wrapper();

protected:
  /* additional members */
};

#endif
