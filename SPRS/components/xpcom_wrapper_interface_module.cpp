#include "nsIGenericFactory.h"
#include "xpcom_wrapper_interface-impl.h"

NS_GENERIC_FACTORY_CONSTRUCTOR(nsSPRS_PKCS11_Wrapper)

static nsModuleComponentInfo components[] = 
{
	{
		SPRS_PKCS11_WRAPPER_CLASSNAME,
		SPRS_PKCS11_WRAPPER_CID,
		SPRS_PKCS11_WRAPPER_CONTRACTID,
		nsSPRS_PKCS11_WrapperConstructor,
	}
};

NS_IMPL_NSGETMODULE("nsSPRS_PKCS11_WrapperModule", components)
