/*
 * security.c
 *
 * Created: 2/25/2017 10:14:41 AM
 *  Author: adeck
 */ 
#include "security.h"

//Reads the flash ID as per the datasheet/ASF documentation and then sends it out to the user
uint32_t* GetUniqueID()
{
	uint32_t unique_id[4];
	
	uint32_t ul_rc;
	
	/* Initialize Flash service */
	ul_rc = flash_init(FLASH_ACCESS_MODE_128, 4);
	if (ul_rc != FLASH_RC_OK)
	{
		Error_T FlashInitFailedError;
		FlashInitFailedError.ThrowerID = THROWER_ID_FLASH_SERVICE;
		FlashInitFailedError.ErrorMajor = FLASH_INIT_FAILED;
		FlashInitFailedError.ErrorMinor = ERROR_NO_MINOR_CODE;
		ThrowError(&FlashInitFailedError);
		return NULL;
	}

	/* Read the unique ID */
	ul_rc = flash_read_unique_id(unique_id, 4);
	
	if (ul_rc != FLASH_RC_OK)
	{
		Error_T FlashInitFailedError;
		FlashInitFailedError.ThrowerID = THROWER_ID_FLASH_SERVICE;
		FlashInitFailedError.ErrorMajor = FLASH_ID_READ_FAILED;
		FlashInitFailedError.ErrorMinor = ERROR_NO_MINOR_CODE;
		ThrowError(&FlashInitFailedError);
		return NULL;
	}
	
	return unique_id;
}
//Calculates the secure key and verifies it against the received key. If they match, secure mode is entered
void EnterSecureMode(long key)
{
	if(key != 0 && GetKey() == key )
	{
		SystemConfiguration.in_secure_mode = true;
		SendStatusReport(ENTER_SECURE_MODE);
		//Start Timer ?? Define behavior
	}else
	{
		//ErrorResponse[4] = ENTER_SECURE_MODE;
		//ErrorResponse[6] = INVALID_KEY;
		//WriteBufferOut(ErrorResponse, 7);
		Error_T FlashInitFailedError;
		FlashInitFailedError.ThrowerID = ENTER_SECURE_MODE;
		FlashInitFailedError.ErrorMajor = INVALID_KEY;
		FlashInitFailedError.ErrorMinor = ERROR_NO_MINOR_CODE;
		ThrowError(&FlashInitFailedError);
		//Error - Key Verification Failed
	}
}
//Exits secure mode
void ExitSecureMode()
{
	SystemConfiguration.in_secure_mode = false;
	SendStatusReport(EXIT_SECURE_MODE);
} 
//Calculates the key for the system
unsigned long GetKey()
{
	uint32_t* unique_id  = GetUniqueID();
	if(unique_id != NULL)
	{
		return (unique_id[0] | unique_id[2] ) ^ (unique_id[1] | unique_id[3]);	
	}else
	{
		return 0;		
	}
}




//Discussed possibly requiring a key to "unlock" bluetooth communication (????)
unsigned long GetBluetoothKey()
{
	uint32_t* unique_id  = GetUniqueID();
	if(unique_id != NULL)
	{
		return (unique_id[0] ^ unique_id[2] ) | (unique_id[1] ^ unique_id[3]);
	}else
	{
		return 0;
	}
}

void UnlockBluetoothCommunications(long key)
{
	if(key != 0 && GetBluetoothKey() == key )
	{
		SystemConfiguration.bluetooth_unlocked = true;
		SendStatusReport(UNLOCK_BLUETOOTH);
		//Start Timer ?? Define behavior
	}else
	{
		//ErrorResponse[4] = ENTER_SECURE_MODE;
		//ErrorResponse[6] = INVALID_KEY;
		//WriteBufferOut(ErrorResponse, 7);
		Error_T FlashInitFailedError;
		FlashInitFailedError.ThrowerID = UNLOCK_BLUETOOTH;
		FlashInitFailedError.ErrorMajor = INVALID_KEY;
		FlashInitFailedError.ErrorMinor = ERROR_NO_MINOR_CODE;
		ThrowError(&FlashInitFailedError);
		//Error - Key Verification Failed
	}
}
/////