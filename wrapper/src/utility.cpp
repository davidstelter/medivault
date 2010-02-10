#include <cryptoki.h>
#include <pkcs11.h>
#include <stdio.h>
#include <windows.h>
#include <string>
#include <iostream>

#include "error.h"

using namespace std;

HMODULE PKCSLibraryModule = 0;
CK_RV	returnValue;
CK_FUNCTION_LIST_PTR funcList;

CK_SLOT_ID_PTR pSlotWithTokenList;
CK_SESSION_HANDLE hSession;
int TokenCount = 0;

bool initCrypto() {
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

void finalizeCrypto() {
	if(!PKCSLibraryModule) {
		return;  //if there is nothing loaded we are done
	}
	funcList->C_Finalize(NULL_PTR);
	FreeLibrary(PKCSLibraryModule);
	PKCSLibraryModule = 0;  //set library to zero to be safe
}

int getTokenCount(){
	CK_ULONG ulSlotCount;			//Number of connected readers
	//Search readers to get count
		returnValue = (funcList->C_GetSlotList)(TRUE,NULL_PTR,&ulSlotCount);
	return ulSlotCount;
}

//Get list of all slots with a token present
string *enumerateCards(void)
{
	CK_SLOT_INFO_PTR pSlotInfo;				//Pointer to slot info
	CK_ULONG ulSlotWithTokenCount;			//Number of connected readers

	pSlotWithTokenList = (CK_SLOT_ID_PTR) malloc(0);		
	ulSlotWithTokenCount = 0;
	string	*SlotsArray= new string[TokenCount];			// array that contains slots info (return value)



	while (1)
		{
			//Search readers and store result in pSlotWithTokenList;
			returnValue = (funcList->C_GetSlotList)(CK_TRUE, pSlotWithTokenList, &ulSlotWithTokenCount);

			if (returnValue != CKR_BUFFER_TOO_SMALL)
				break;

			pSlotWithTokenList = (unsigned long *)malloc(sizeof(CK_SLOT_ID)*ulSlotWithTokenCount);
		}

	if (returnValue == CKR_OK && ulSlotWithTokenCount > 0) 
	{
			
			pSlotInfo = (CK_SLOT_INFO_PTR)malloc(sizeof(CK_SLOT_INFO));

			// Get all the slots info
			for(unsigned int i = 0; i < ulSlotWithTokenCount; i++)
			{
				returnValue = (funcList->C_GetSlotInfo)(pSlotWithTokenList[i],pSlotInfo);
				if(returnValue == CKR_OK)
				{
					SlotsArray[i] = ((char*)pSlotInfo->slotDescription);
					printf("Slot %d: ",i);
					cout << SlotsArray[i] << endl; // somehow it doesn't print with printf()!
				}
			}
	}
	else
		setError(NO_DEVICES_FOUND);

		//returns list of all cards on the system. An empty list indicates an error;
		return SlotsArray;
}


//Selects a card to use for subsequent operations.  Returns false on failure and sets 
bool selectCard(int SlotID)
{
		//Open session for selected card
		returnValue = (funcList->C_OpenSession)(pSlotWithTokenList[SlotID],
						CKF_SERIAL_SESSION| CKF_RW_SESSION, NULL, NULL, &hSession);

		if (returnValue == CKR_OK) 
			return true;
		else
		{
			setError(OPEN_SESSION_FAILED);
			return false;
		}
}

int main(){
	if(initCrypto()){
		TokenCount = getTokenCount();
		printf("Token count: %d\n", TokenCount);

		if (TokenCount != 0){
			string *slots;
			slots = enumerateCards();
			

			if (!selectCard(0))
				printf("Opening session error: %d", getLastError());

			delete [] slots;
		}
	}
	else{
		printf("Initialization error: %d", getLastError());
	}
	getchar();
	return 0;
}