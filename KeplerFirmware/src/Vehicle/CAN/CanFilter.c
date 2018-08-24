/*
 * Filter.c
 *
 * Created: 7/12/2017 11:14:51 AM
 *  Author: adeck
 */ 
#include "CanFilter.h"

//Creates a filer and adds it to the filter list
bool CreateFilter(uint8_t FilterType, uint8_t * Mask, uint8_t* Pattern, uint8_t* FlowControlMessage ,uint16_t Length)
{
	//Make sure we have room
	if(FilterCounter + 1 > MAX_FILTERS)
	{
		Error_T OutOfBoundsError;
		OutOfBoundsError.ThrowerID = THROWER_ID_COMMAND_RESPONSE_SYSTEM;
		OutOfBoundsError.ErrorMajor = MESSAGE_FILTER_INDEX_OUT_OF_BOUNDS;
		OutOfBoundsError.ErrorMinor = ERROR_NO_MINOR_CODE;
		ThrowError(&OutOfBoundsError);
		return;
	}
	//Create and initialize the filter
	Filter_t tmpFilter;
	tmpFilter.Type = FilterType;
	tmpFilter.FilterMessageLength = Length;
	tmpFilter.MaskMessage = (uint8_t*) malloc(Length);
	tmpFilter.PatternMessage = (uint8_t*) malloc(Length);
	tmpFilter.FlowControlMessage = (uint8_t*) malloc(Length);
	//Out of memory error
	if(tmpFilter.MaskMessage == NULL || tmpFilter.PatternMessage == NULL || tmpFilter.FlowControlMessage == NULL)
	{
		//TODO: ERROR OOM
		return false;
	}
	//Copy over filter data
	tmpFilter.FilterID = FilterCounter;
	memcpy(tmpFilter.MaskMessage, Mask, Length);
	memcpy(tmpFilter.PatternMessage, Pattern, Length);
	memcpy(tmpFilter.FlowControlMessage, FlowControlMessage, Length);
//Add it to the list
	Filters[FilterCounter++] = tmpFilter;

	return true;
}

//Removes a filter from the list based on ID
bool RemoveFilter(uint8_t FilterID)
{
	int i;
 	if (FilterID > FilterCounter)
	{
		return false;
	}

	for(i = 0; i < FilterCounter - 1; i++) 
	{
		if(Filters[i].FilterID == FilterID)
		{
			free(Filters[i].MaskMessage);
			free(Filters[i].PatternMessage);
			FilterCounter--;
			for(i; i < FilterCounter - 1; i++) Filters[i] = Filters[i + 1];
			return true;
		}
	}
	
}
//Removes all the filters
void DeleteAllFilters()
{
	int i;
	//Free all of the filter memory
	for(i = 0; i < FilterCounter; i++)
	{
		Filters[i].MaskMessage = NULL;
		Filters[i].PatternMessage = NULL;
		Filters[i].FlowControlMessage = NULL;
		free(Filters[i].MaskMessage);
		free(Filters[i].PatternMessage);
		free(Filters[i].FlowControlMessage);	
	}
	
	FilterCounter = 0;
	rx_mailbox_num = 1;
	can_disable_interrupt(SYSTEM_CAN, CAN_DISABLE_ALL_INTERRUPT_MASK);
	can_enable_interrupt(SYSTEM_CAN, CAN_IER_ERRA | CAN_IER_WARN | CAN_IER_ERRP | CAN_IER_BOFF | CAN_IER_BERR);
	can_reset_all_mailbox(SYSTEM_CAN);
	
	uint8_t DeleteFiltersSuccess[] = {START_BYTE, 0x00,0x02,DELETE_CAN_FILTER, 0x01};
	Message_t tmpMsg;
	tmpMsg.buf = DeleteFiltersSuccess;
	tmpMsg.Size = 5;
	WriteMessage(&tmpMsg);
}
//Starts Filtering messages
bool StartFiltering()
{
	if(FilterCounter > 0)
	{
		IsFilteringEnabled = true;
		return true;
	}	
	IsFilteringEnabled = false;
	return false;
}
//Stops filtering messages
bool StopFiltering()
{
	IsFilteringEnabled = false;
	return true;	
}
//Runs the filters, returns the filter type. By doing this we simply the logic. A block filter type is 0 and if it matches it will return 0 which will
//cause the caller to ignore the message as intended.
int RunFilters(uint8_t * Message, uint16_t length, uint8_t** FlowControlMessage, uint8_t* FlowControlMessageLength)
{
	int i, j;	
	if(IsFilteringEnabled == false)
	{
		WriteLine("Filtering Disabled!");
		return true;
	}
	
	WriteLine("Running Filter!");
	
	for(i = 0; i < FilterCounter; i++)
	{

		if(Filters[i].FilterMessageLength > length)
		{
			//WriteLine("Filter length greater than message length.");
			continue;	
		}
		for(j = 0; j < Filters[i].FilterMessageLength; j++)
		{
			//WriteLine("Running Filter!");
			//
			//WriteString("Mask Message: ");
			//WriteCharArr(Filters[i].MaskMessage, Filters[i].FilterMessageLength);
			//WriteChar(0x0A);
			//
			//WriteString("Pattern Message: ");
			//WriteCharArr(Filters[i].PatternMessage, Filters[i].FilterMessageLength);
			//WriteChar(0x0A);
			char t1 = Message[j];
			char t2 = Filters[i].MaskMessage[j];
			char t3 = Filters[i].PatternMessage[j];
			char t4 = t1 & t2;
			
			//if(Message[j] & Filters[i].MaskMessage[j] != Filters[i].PatternMessage[j])
			if(t4 != t3)
			{
				//WriteLine("Skip");
				goto Skip;
			}	
		}
		//WriteString("Display Message: ");
		//WriteChar(Filters[i].Type);
		//WriteChar(0x0A);
		*FlowControlMessage = Filters[i].FlowControlMessage;
		*FlowControlMessageLength = length;
		return Filters[i].Type;
		
		Skip:
		continue;
	}
	//WriteLine("Dropping Message");
	return false;
}
