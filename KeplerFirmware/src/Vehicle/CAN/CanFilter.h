/*
 * Filter.h
 *
 * Created: 7/12/2017 10:55:08 AM
 *  Author: adeck
 */ 


#ifndef CAN_FILTER_H_
#define CAN_FILTER_H_

#include "asf.h"
#include "kcan.h"
#define MAX_FILTERS 254

static uint8_t FilterCounter = 1;
static uint32_t FilterID = 1;

typedef struct {
	uint8_t Type;
	uint16_t FilterMessageLength;
	uint8_t* MaskMessage;
	uint8_t* PatternMessage;
	uint8_t* FlowControlMessage;
	uint8_t* FilterID;
} Filter_t;

static Filter_t Filters[MAX_FILTERS];
static bool IsFilteringEnabled = false;

bool CreateFilter(uint8_t FilterType, uint8_t * Mask, uint8_t* Pattern, uint8_t* FlowControlMessage ,uint16_t Length);
bool RemoveFilter(uint8_t FilterID);
bool StartFiltering(void);
bool StopFiltering(void);
/*
*Runs a message through the current list of filters. 
*Returns 2 if the message should be passed on and ran through the ISO15765 processor.
*Returns 1 if the message should be passed on. 
*Returns 0 if the message should be dropped.
*/
int RunFilters(uint8_t * Message, uint16_t length, uint8_t** FlowControlMessage, uint8_t* FlowControlMessageLength);
void DeleteAllFilters(void);




#endif /* CAN_FILTER_H_ */