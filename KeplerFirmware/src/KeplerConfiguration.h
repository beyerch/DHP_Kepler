/*
 * KeplerConfiguration.h
 *
 * Created: 2/25/2017 12:03:49 AM
 *  Author: adeck
 */ 


#ifndef KEPLERCONFIGURATION_H_
#define KEPLERCONFIGURATION_H_

#include "compiler.h"

#define cpu_ms_2_cy(ms, f_cpu) (((uint64_t)(ms) * (f_cpu) / 6 + 999) / 1e3)

#define PC_COM_MODE_USB 0
#define PC_COM_MODE_BLUETOOTH 1
#define PC_COM_MODE_INTERNAL_DUAL 2

#define ENGINEERING_FIRMWARE		    0xEF	
#define MAJOR							0x01	
#define MINOR							0x1A
#define DATE_MM							0x0B
#define DATE_DD							0x12
#define DATE_YY							0x11	
#define VERSION_LENGTH					0x0A

#define MESSAGE_BUFFER_SIZE 5000

#define LED_USB_FRAME_COUNTER 

#define VPW_MODE						0x00
#define PWM_MODE						0x01
#define CAN_MODE						0x02
#define K_LINE							0x07
#define NONE							0xFF

#define ISO_TP_DISABLE  0
#define ISO_TP_ENABLE   1

typedef struct {
	
	char pc_com_mode;
	char vehicle_com_mode;
	bool in_secure_mode;
	bool bluetooth_unlocked;
	char isotp_mode;
	
} KeplerConfiguration_t;

static KeplerConfiguration_t SystemConfiguration;

#endif /* KEPLERCONFIGURATION_H_ */