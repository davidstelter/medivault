#include "cryptoki.h"
#include <windows.h>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

#include "subject.h"
#include "CryptoWrapper.h"

/*!
*	\class CryptoWrapper
*	@brief
*	Interface with acospkcs11.dll to provide cryptographic functionality.
*/
CryptoWrapper::CryptoWrapper(void)
{
	this->PKCSLibraryModule = 0;
	lastError = OK;
}


/*!
*	@brief
*	Interface wrapper constructor
*	@param void
*	@retval
*	@remarks Constructor for the wrapper API calls
*/
CryptoWrapper::~CryptoWrapper(void)
{
	//destroy the crypto environment
	finalizeCrypto();
}


/*!
*	@brief
*	Functions for setting up the crypto environment
*	initCrypto() do the following:
*		- Makes sure that the DLL was not loaded before.
*		- Loads the DLL file.
*		- Gets the functions list from the DLL file.
*		- Initializes the Cryptoki library
*	@param void
*	@retval bool Returns false on failure and sets
*	@remarks
* 	Initializes the crypto enviroment by loading the 
* 	acospkcs11.dll into memory and retrieve it's functionlist.
*/
bool CryptoWrapper::initCrypto() {
	CK_RV	returnValue;	//holds the return value
	if(PKCSLibraryModule) {
		setError(ALREADY_LOADED);
		return false;
	}
	//Load the DLL file.
	if (!(PKCSLibraryModule = LoadLibrary(L".\\acospkcs11.dll"))) {
		setError(DLL_LOAD_ERROR);
		return false;
	}
	//Get the function list
	CK_C_GetFunctionList pC_GetFunctionList = (CK_C_GetFunctionList)
		GetProcAddress(PKCSLibraryModule, "C_GetFunctionList");

	returnValue = (*pC_GetFunctionList)(&funcList);
	if (returnValue != CKR_OK) {
		setError(NO_FUNC_LIST);
		FreeLibrary(PKCSLibraryModule);
		PKCSLibraryModule = 0;  //set library to zero to be safe
		return false;
	}
	//initialize the cryptoEngine
	returnValue = (funcList->C_Initialize)(NULL_PTR);
	if (returnValue != CKR_OK) {
		setError(INIT_FAILED);
		FreeLibrary(PKCSLibraryModule);
		PKCSLibraryModule = 0;  //set library to zero to be safe
		return false;
	}
	return true;
}


/*!
*	@brief
*	Functions for taking down the crypto environment
*	finalizeCrypto() do the following:
*		- Will finalize the Crypto library if it was loaded.
*		- Frees the the library.
*	@param void
*	@retval
*	@remarks Unload acospkcs11 DLL and free memory allocated, destroy enviroment
*/
void CryptoWrapper::finalizeCrypto() {
	if(!PKCSLibraryModule) {
		return;  //if there is nothing loaded we are done
	}
	if(!funcList) {
		return;
	}
	funcList->C_Finalize(NULL_PTR);
	FreeLibrary(PKCSLibraryModule);
	PKCSLibraryModule = 0;  //set library to zero to be safe
}



/*!
*	@brief
*	Utility function
*	getTokenCount() do the following:
*		- Returns the number of cards which have tokens
*	@param void
*	@retval int
*	@remarks Count the number of cards which have tokens
*/
int CryptoWrapper::getTokenCount(){
	CK_RV	returnValue;	//holds the return value
	CK_ULONG ulSlotCount;	//Number of connected readers with tokens
	//Search readers to get count
	returnValue = (funcList->C_GetSlotList)(TRUE,NULL_PTR,&ulSlotCount);
	return ulSlotCount;
}


/*!
*	@brief
*	Function return list of token present
*	enumerateCards() do the following:
*		- Gets the number of cards which have tokens.
*		- Creats a pointer to an array that contains slots info (return value).
*		- Search readers and store result in SlotWithTokenList.
*		- Gets all the slots info
*		- Returns list of all slots with a token present
*	@param void
*	@retval *string Returns pointer to string of tokens
*	@remarks
* 	Get list of all slots with a token present when there is a 
* 	acos5 detected within the system
*/
string* CryptoWrapper::enumerateCards(void)
{
	CK_RV	returnValue;			//holds the return value
	CK_SLOT_INFO_PTR SlotInfo;		//Pointer to slot info
	CK_ULONG SlotWithTokenCount;	//Number of connected readers

	int TokenCount = getTokenCount();

	SlotWithTokenList = (CK_SLOT_ID_PTR) malloc(0);		
	SlotWithTokenCount = 0;
	//array that contains slots info (return value)
	string	*SlotsArray = new string[TokenCount]; 

	while (1) {
		//Search readers and store result in SlotWithTokenList;
		returnValue = (funcList->C_GetSlotList)(TRUE, SlotWithTokenList, &SlotWithTokenCount);

		if (returnValue != CKR_BUFFER_TOO_SMALL) {
			break;
		}		

		SlotWithTokenList = (unsigned long *)malloc(sizeof(CK_SLOT_ID)*SlotWithTokenCount);
	}

	if (returnValue == CKR_OK && SlotWithTokenCount > 0) {
		SlotInfo = (CK_SLOT_INFO_PTR)malloc(sizeof(CK_SLOT_INFO));

		// Get all the slots info
		for(unsigned int i = 0; i < SlotWithTokenCount; i++) {
			returnValue = (funcList->C_GetSlotInfo)(SlotWithTokenList[i],SlotInfo);
			if(returnValue == CKR_OK) {
				SlotsArray[i] = ((char*)SlotInfo->slotDescription);
			}
		}
	}
	else {
		setError(NO_DEVICES_FOUND);
		return NULL;
	}

	//returns list of all cards on the system. An empty list indicates an error;
	return SlotsArray;
}


/*!
*	@brief
*	Function to select inserted smartcard
*	selectCard() do the following:
*		- lists all slots with a token present.
*		- Opens a session for the selected card to use for subsequent operations.
*		- Logs into the selected token. 
*	@param [in] SlotID The slotid token to be used
*   @param [in] pin	Pointer to pin authentication
*   @param [in] pinlen Length of given pin input
*	@retval	bool Returns false on failure and sets 
*	@remarks Selects a card to use for subsequent operations.  
*/
bool CryptoWrapper::selectCard(int SlotID, CK_UTF8CHAR* UserPIN, int pinlen)
{
	enumerateCards();		//this does a little housekeeping for us...
	CK_RV	returnValue;	//holds the return value

	//Open session for selected card
	returnValue = (funcList->C_OpenSession)(SlotWithTokenList[SlotID],
					CKF_SERIAL_SESSION| CKF_RW_SESSION, NULL, NULL, &hSession);

	if (returnValue == CKR_OK) {
		//CK_UTF8CHAR UserPIN[] = SC_PIN_CODE;
		
		//returnValue = (funcList->C_Login)(hSession, CKU_USER, UserPIN, sizeof(UserPIN)-1);
		returnValue = (funcList->C_Login)(hSession, CKU_USER, UserPIN, pinlen);

		if (returnValue != CKR_OK) {
			setError(WRONG_PASSWORD);
			return false;
		}
		else {			
			return true;
		}
	}
	else {
		setError(OPEN_SESSION_FAILED);
		return false;
	}
}
