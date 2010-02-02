
#include <cryptoki.h>
#include <stdio.h>
#include <windows.h>   




HMODULE PKCSLibMod = 0;         //PKCS library module handle
	  
CK_RV rv;						//Cryptoki return value
CK_FUNCTION_LIST_PTR pFnList;	//Pointer to function list



int main(void)
{
		CK_SLOT_INFO_PTR pInfo;			//Pointer to slot info
		CK_ULONG ulSlotCount;			//Number of connected readers
		CK_SLOT_ID_PTR pSlotList;		//Pointer to slot list or reader list




		if ( !(PKCSLibMod = LoadLibrary("acospkcs11.dll")))
		{
			printf("Couldn't found/n");
			return 0;
		} 
		else
			printf("acospkcs11 was found...\n");		


		// 2. Load PKCS functions
		CK_C_GetFunctionList pC_GetFunctionList = (CK_C_GetFunctionList)GetProcAddress(PKCSLibMod, "C_GetFunctionList");
		

		
		rv = (*pC_GetFunctionList)(&pFnList);
		
		if (rv != CKR_OK) 
			printf("Couldn't get Function List\n");


		//3. Initialize

			rv = pFnList->C_Initialize(NULL_PTR);
			if (rv != CKR_OK) 
			{
				printf("Couldn't Initilized\n");
				printf("%d\n",rv);
			}


		//4. Load readers

		//Search readers to get count
		rv = (pFnList->C_GetSlotList)(FALSE,NULL_PTR,&ulSlotCount);

		if ((rv == CKR_OK) && (ulSlotCount > 0)) 
		{
		 
			pSlotList = (unsigned long *)malloc(sizeof(CK_SLOT_ID)*ulSlotCount);
			
			//Search readers and store result in pSlotList
			rv = (pFnList->C_GetSlotList)(FALSE, pSlotList, &ulSlotCount);
			if (rv != CKR_OK)
			{
				//Add detected readers to list	
				pInfo = (CK_SLOT_INFO_PTR)malloc(sizeof(CK_SLOT_INFO));

				/* Get slot information for each slot */
				for(unsigned int i=0;i<ulSlotCount;i++)
				{
					rv = (pFnList->C_GetSlotInfo)(pSlotList[i],pInfo);
					if(rv != CKR_OK)
					{
					  printf("Couldn't get SlotInfo\n"); 
					  printf("%d\n",rv);
					}		

					//mcb_readerlist.AddString((char*)pInfo->slotDescription);
					getchar();
				}	
				
				//if (rv != CKR_OK) return;
				//	cout << rv << endl;
			}
		}
		else
		{
			printf("Couldn't get Slots List\n");
		}

		//Finalizing
		rv=(pFnList->C_Finalize)(NULL_PTR);		
		if (rv != CKR_OK) 
			{
				printf("Couldn't Finalize\n");
				printf("%d\n",rv);
			}
		
		
		getchar();

return 0;
}