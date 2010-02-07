#include <cryptoki.h>
#include <stdio.h>
#include <windows.h> 



CK_RV	returnValue;					//Cryptoki return value
CK_SLOT_ID_PTR pSlotWithTokenList;;		//Pointer to slot list or reader list with a token

//Get list of all slots with a token present
void enumerateCards()
{
	pSlotWithTokenList = (CK_SLOT_ID_PTR) malloc(0);		
	ulSlotWithTokenCount = 0;
	Slots	SlotsArray[] = NULL;			// Slot is a struct that has fields: Info, ID

	CK_SLOT_INFO_PTR pInfo;					//Pointer to slot info
	CK_ULONG ulSlotWithTokenCount;			//Number of connected readers

	while (1)
		{
			//Search readers and store result in pSlotWithTokenList;
			returnValue = (pFnList->C_GetSlotList)(CK_TRUE, pSlotWithTokenList, &ulSlotWithTokenCount);

			if (returnValue != CKR_BUFFER_TOO_SMALL)
				break;

			pSlotWithTokenList = (unsigned long *)malloc(sizeof(CK_SLOT_ID)*ulSlotWithTokenCount);
		}

	if (returnValue == CKR_OK && ulSlotWithTokenCount > 0) 
	{
			
			pInfo = (CK_SLOT_INFO_PTR)malloc(sizeof(CK_SLOT_INFO));

			// Get all the slots info and assign an ID to them
			for(unsigned int i = 0; i < ulSlotWithTokenCount; i++)
			{
				returnValue = (pFnList->C_GetSlotInfo)(pSlotWithTokenList[i],pInfo);
				if(returnValue == CKR_OK)
				{
					SlotsArray.info = (char*)pInfo->slotDescription);
					SlotsArray.ID = i;

				}
			}
	}
	else
		setError(NO_DEVICES_FOUND);


		//returns list of all cards on the system. A empty list indicates an error;
		return SlotsArray[];
}
