/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM comp.idl
 */

#ifndef __gen_comp_h__
#define __gen_comp_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    ISpecialThing */
#define ISPECIALTHING_IID_STR "263ed1ba-5cc1-11db-9673-00e08161165f"

#define ISPECIALTHING_IID \
  {0x263ed1ba, 0x5cc1, 0x11db, \
    { 0x96, 0x73, 0x00, 0xe0, 0x81, 0x61, 0x16, 0x5f }}

class NS_NO_VTABLE ISpecialThing : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(ISPECIALTHING_IID)

  /* long add (in long a, in long b); */
  NS_IMETHOD Add(PRInt32 a, PRInt32 b, PRInt32 *_retval) = 0;

  /* long readFile (); */
  NS_IMETHOD ReadFile(PRInt32 *_retval) = 0;

  /* attribute AString name; */
  NS_IMETHOD GetName(nsAString & aName) = 0;
  NS_IMETHOD SetName(const nsAString & aName) = 0;

  /* attribute AString fileName; */
  NS_IMETHOD GetFileName(nsAString & aFileName) = 0;
  NS_IMETHOD SetFileName(const nsAString & aFileName) = 0;

  /* attribute AString fileContents; */
  NS_IMETHOD GetFileContents(nsAString & aFileContents) = 0;
  NS_IMETHOD SetFileContents(const nsAString & aFileContents) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_ISPECIALTHING \
  NS_IMETHOD Add(PRInt32 a, PRInt32 b, PRInt32 *_retval); \
  NS_IMETHOD ReadFile(PRInt32 *_retval); \
  NS_IMETHOD GetName(nsAString & aName); \
  NS_IMETHOD SetName(const nsAString & aName); \
  NS_IMETHOD GetFileName(nsAString & aFileName); \
  NS_IMETHOD SetFileName(const nsAString & aFileName); \
  NS_IMETHOD GetFileContents(nsAString & aFileContents); \
  NS_IMETHOD SetFileContents(const nsAString & aFileContents); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_ISPECIALTHING(_to) \
  NS_IMETHOD Add(PRInt32 a, PRInt32 b, PRInt32 *_retval) { return _to Add(a, b, _retval); } \
  NS_IMETHOD ReadFile(PRInt32 *_retval) { return _to ReadFile(_retval); } \
  NS_IMETHOD GetName(nsAString & aName) { return _to GetName(aName); } \
  NS_IMETHOD SetName(const nsAString & aName) { return _to SetName(aName); } \
  NS_IMETHOD GetFileName(nsAString & aFileName) { return _to GetFileName(aFileName); } \
  NS_IMETHOD SetFileName(const nsAString & aFileName) { return _to SetFileName(aFileName); } \
  NS_IMETHOD GetFileContents(nsAString & aFileContents) { return _to GetFileContents(aFileContents); } \
  NS_IMETHOD SetFileContents(const nsAString & aFileContents) { return _to SetFileContents(aFileContents); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_ISPECIALTHING(_to) \
  NS_IMETHOD Add(PRInt32 a, PRInt32 b, PRInt32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->Add(a, b, _retval); } \
  NS_IMETHOD ReadFile(PRInt32 *_retval) { return !_to ? NS_ERROR_NULL_POINTER : _to->ReadFile(_retval); } \
  NS_IMETHOD GetName(nsAString & aName) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetName(aName); } \
  NS_IMETHOD SetName(const nsAString & aName) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetName(aName); } \
  NS_IMETHOD GetFileName(nsAString & aFileName) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFileName(aFileName); } \
  NS_IMETHOD SetFileName(const nsAString & aFileName) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFileName(aFileName); } \
  NS_IMETHOD GetFileContents(nsAString & aFileContents) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetFileContents(aFileContents); } \
  NS_IMETHOD SetFileContents(const nsAString & aFileContents) { return !_to ? NS_ERROR_NULL_POINTER : _to->SetFileContents(aFileContents); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class _MYCLASS_ : public ISpecialThing
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_ISPECIALTHING

  _MYCLASS_();

private:
  ~_MYCLASS_();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(_MYCLASS_, ISpecialThing)

_MYCLASS_::_MYCLASS_()
{
  /* member initializers and constructor code */
}

_MYCLASS_::~_MYCLASS_()
{
  /* destructor code */
}

/* long add (in long a, in long b); */
NS_IMETHODIMP _MYCLASS_::Add(PRInt32 a, PRInt32 b, PRInt32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* long readFile (); */
NS_IMETHODIMP _MYCLASS_::ReadFile(PRInt32 *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute AString name; */
NS_IMETHODIMP _MYCLASS_::GetName(nsAString & aName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP _MYCLASS_::SetName(const nsAString & aName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute AString fileName; */
NS_IMETHODIMP _MYCLASS_::GetFileName(nsAString & aFileName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP _MYCLASS_::SetFileName(const nsAString & aFileName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute AString fileContents; */
NS_IMETHODIMP _MYCLASS_::GetFileContents(nsAString & aFileContents)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP _MYCLASS_::SetFileContents(const nsAString & aFileContents)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_comp_h__ */
