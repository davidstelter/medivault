#include "nsIGenericFactory.h"
#include "comp-impl.h"

NS_GENERIC_FACTORY_CONSTRUCTOR(CSpecialThing)

static nsModuleComponentInfo components[] =
{
    {
       SPECIALTHING_CLASSNAME, 
       SPECIALTHING_CID,
       SPECIALTHING_CONTRACTID,
       CSpecialThingConstructor,
    }
};

NS_IMPL_NSGETMODULE("SpecialThingsModule", components) 

