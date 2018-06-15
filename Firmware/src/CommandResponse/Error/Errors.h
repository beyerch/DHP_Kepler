/*
 * Errors.h
 *
 * Created: 2/25/2017 12:01:12 AM
 *  Author: adeck
 */ 


#ifndef ERRORS_H_
#define ERRORS_H_

#define ERROR_RESPONSE											0xEF
#define ERROR_NO_MINOR_CODE										0x00

//Exceptions
#define INVALID_START_BYTE_EXCEPTION							0x01
#define INVALID_LENGTH_BYTES									0x02
#define VPW_RETURN_CODE_SOF_TOO_LONG_EXCEPTION					0x03
#define VPW_RETURN_CODE_NO_DATA_EXCEPTION						0x04
#define UNREACHABLE_STATE_REACHED_EXCPETION						0x05
#define INVALID_PC_COM_MODE_EXCEPTION							0x06
#define VPW_RETURN_CODE_PULSE_TIMING_UNKOWN						0x07
#define INVALID_INTERFACE_MODE_EXCEPTION						0x08
#define TP_NO_FIRST_FC_RESP_RCVD								0x09
#define TP_EXCEEDED_MAX_ALLOW_WAIT_FRAMES						0x0A
#define FLASH_INIT_FAILED										0x0B
#define FLASH_ID_READ_FAILED									0x0C
#define TP_RECEIVER_SIGNALED_OVERFLOW							0x0D
#define TP_RECEIVED_UNEXPECTED_FC_TYPE							0x0E
#define TP_TIMEOUT												0x0F
#define MESSAGE_FILTER_INDEX_OUT_OF_BOUNDS						0x10
#define INVALID_FILTER_TYPE_BYTE								0X11
#define INVALID_KEY												0x14

//Thrower IDs, used if message byte not applicable
#define THROWER_ID_COMMAND_RESPONSE_SYSTEM						0x01
#define THROWER_ID_FLASH_SERVICE								0x02
#define THROWER_ID_ISO_TP										0x03

#endif /* ERRORS_H_ */