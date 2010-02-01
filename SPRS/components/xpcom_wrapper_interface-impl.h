#ifndef __SPRS_PKCS11_WRAPPER_IMPL_H__
#define __SPRS_PKCS11_WRAPPER_IMPL_H__

#include "xpcom_pkcs11_wrapper.h"
#include "nsStringAPI.h"

#define SPRS_PKCS11_WRAPPER_CONTRACTID "@capstone.pdx.edu/sprs;1"
#define SPRS_PKCS11_WRAPPER_CLASSNAME "SPRS_PKCS11_Wrapper"

/* no idea what this next define does (if anything)...
#define SPRS_PKCS11_WRAPPER_CID \
  {0xb23e4e97, 0x4e78, 0x4da5, \
    { 0xbf, 0x18, 0x1a, 0xcc, 0xf1, 0x93, 0x61, 0xdb }}
*/

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
