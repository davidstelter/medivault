#include <pkcs11.h>
#include <stdio.h>
#include <windows.h> 

#include "error.h"



CK_RV	returnValue;						//Cryptoki return value
extern CK_SLOT_ID_PTR pSlotWithTokenList;	//Pointer to slot list or reader list with a token
extern CK_FUNCTION_LIST_PTR funcList;

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
