#ifndef __SPECIALTHING_IMPL_H__
#define __SPECIALTHING_IMPL_H__

#include "comp.h"
#include "nsStringAPI.h"

#define SPECIALTHING_CONTRACTID "@starkravingfinkle.org/specialthing;1"
#define SPECIALTHING_CLASSNAME "SpecialThing"
#define SPECIALTHING_CID { 0x245626, 0x5cc1, 0x11db, { 0x96, 0x73, 0x0, 0xe0, 0x81, 0x61, 0x16, 0x5f } }

class CSpecialThing : public ISpecialThing
{
public:
	NS_DECL_ISUPPORTS
	NS_DECL_ISPECIALTHING

	CSpecialThing();

private:
	~CSpecialThing();

protected:
	/* additional members */
	nsString mName;
	nsString mFileContents;
	nsString mFileName;
};

#endif