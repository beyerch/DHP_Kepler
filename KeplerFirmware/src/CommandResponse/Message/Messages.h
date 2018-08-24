/*
 * KeplerCommands.h
 *
 * Created: 2/24/2017 11:51:23 PM
 *  Author: adeck
 */ 


#ifndef KEPLERCOMMANDS_H_
#define KEPLERCOMMANDS_H_


#define START_BYTE 0x02

//See command and response document for more information

/*		NAME					  |			VALUE	  |			SECURE_MODE_REQUIRED			|			IMPLEMENTED		*/
/*________________________________|___________________|_________________________________________|___________________________*/
#define STATUS_REQUEST			/*|*/		0x00	/*|						N					|				Y			*/
#define SET_INTERFACE_MODE		/*|*/		0xA0 	/*|						N					|				Y			*/
#define SEND_MESSAGE			/*|*/		0xA1	/*|						N					|				Y			*/
#define ENTER_VPW_1X			/*|*/		0xB0	/*|						N					|				Y			*/
#define ENTER_VPW_4X			/*|*/		0xB1	/*|						N					|				Y			*/
#define SET_PERIODIC_MSG		/*|*/		0xB4	/*|						N					|				N			*/
#define SET_PERIODIC_TMR		/*|*/		0xB5	/*|						N					|				N			*/
#define READ_ADC_VALUE			/*|*/		0xBD	/*|						N					|				Y			*/
#define ENABLE_PERIODIC_ADC		/*|*/		0xBE	/*|						N					|				Y			*/
#define DISABLE_PERIODIC_ADC	/*|*/		0xBF	/*|						N					|				Y			*/
#define DELETE_ALL_VPW_FILTERS	/*|*/		0xC0	/*|						N					|				N			*/
#define DELETE_VPW_FILTER		/*|*/		0xC1	/*|						N					|				Y			*/
#define CREATE_VPW_FILTER		/*|*/		0xC2	/*|						N					|				Y			*/
#define DELETE_ALL_CAN_FILTERS	/*|*/		0xC3	/*|						N					|				N			*/
#define DELETE_CAN_FILTER		/*|*/		0xC4	/*|						N					|				Y			*/
#define CREATE_CAN_FILTER		/*|*/		0xC5	/*|						N					|				Y			*/
#define INIT_CAN_MAILBOX		/*|*/		0xC6	/*|						N					|				Y			*/
#define SET_CAN_BAUD			/*|*/		0xC7	/*|						N					|				N			*/
#define VERSION_REQUEST			/*|*/		0xE0	/*|						N					|				Y			*/
#define READ_UNIQUE_ID			/*|*/		0xE1	/*|						N					|				Y			*/
#define ENTER_SECURE_MODE		/*|*/		0xE2	/*|						N					|				Y			*/
#define FIRMWARE_UPDATE			/*|*/		0xE3	/*|						Y					|				Y			*/
#define RESET_DEVICE			/*|*/		0xE4	/*|						N					|				Y			*/
#define EXIT_SECURE_MODE		/*|*/		0xE6	/*|						N					|				Y			*/
#define SET_TX_MODE				/*|*/		0xFD	/*|						Y					|				y			*/
#define PRODUCTION_SELF_TEST	/*|*/		0xFF	/*|						Y					|				N			*/
#define UNLOCK_BLUETOOTH		/*|*/		0xE7	/*|						Y					|				N
/*________________________________|___________________|_________________________________________|___________________________*/


#define NETWORK_MESSAGE	0xAA



#endif /* KEPLERCOMMANDS_H_ */