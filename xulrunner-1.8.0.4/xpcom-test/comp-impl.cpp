#include "comp-impl.h"

NS_IMPL_ISUPPORTS1(CSpecialThing, ISpecialThing)

CSpecialThing::CSpecialThing()
{
	/* member initializers and constructor code */
	mName.Assign(L"Default Name");
}

CSpecialThing::~CSpecialThing()
{
	/* destructor code */
}

/* long add (in long a, in long b); */
NS_IMETHODIMP CSpecialThing::Add(PRInt32 a, PRInt32 b, PRInt32 *_retval)
{
	*_retval = a + b + 1;
	return NS_OK;
}

/* attribute AString name; */
NS_IMETHODIMP CSpecialThing::GetName(nsAString & aName)
{
	aName.Assign(mName);
	return NS_OK;
}
NS_IMETHODIMP CSpecialThing::SetName(const nsAString & aName)
{
	mName.Assign(aName);
	return NS_OK;
}

/* long readFile (); */
NS_IMETHODIMP CSpecialThing::ReadFile(PRInt32 *_retval)
{
/*	char* buf[2048];

	FILE* fd = fopen(filename, "r");
	while (fread(buf, 1, 1, fd)){
		printf("%c", buf);
	}
	*/
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute AString fileName; */
NS_IMETHODIMP CSpecialThing::GetFileName(nsAString & aFileName)
{
	aFileName.Assign(mFileName);
    return NS_OK;
}
NS_IMETHODIMP CSpecialThing::SetFileName(const nsAString & aFileName)
{
	mFileName.Assign(aFileName);
    return NS_OK;
}

/* attribute AString fileContents; */
NS_IMETHODIMP CSpecialThing::GetFileContents(nsAString & aFileContents)
{
	aFileContents.Assign(mFileContents);
    return NS_OK;
}
NS_IMETHODIMP CSpecialThing::SetFileContents(const nsAString & aFileContents)
{
	mFileContents.Assign(aFileContents);
    return NS_OK;
}