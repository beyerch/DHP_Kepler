/*
 * j1850vpw.h
 *
 * Created: 2/25/2017 1:06:12 AM
 *  Author: adeck
 */ 


#ifndef J1850VPW_H_
#define J1850VPW_H_

#include <asf.h>
#include "compiler.h"
#include "Timers.h"
#define  VPW_BUF_SIZE 5000

#define J1850_VPW_PASSIVE_ONE 128
#define J1850_VPW_PASSIVE_ZERO 64

#define J1850_VPW_ACTIVE_ONE 64
#define J1850_VPW_ACTIVE_ZERO 128

// Allow easy swap of logic


// define levels inverted by hardware
#ifdef VPW_BIT_NEG
#define VPW_ACTIVE 0
#define VPW_PASSIVE 1
#else
#define VPW_ACTIVE 1
#define VPW_PASSIVE 0

#define VPW_RX_ACTIVE 0
#define VPW_RX_PASSIVE 1
#endif /* !VPW_BIT_NEG */

//Convert a microsecond value to a timer value
#define us2cnt(us) (unsigned int)(12 * us)

#define GO_ACTIVE ioport_set_pin_level(J1850_P_TX, VPW_ACTIVE);
#define GO_PASSIVE ioport_set_pin_level(J1850_P_TX, VPW_PASSIVE);

#define IS_VPW_ACTIVE_LEVEL	(ioport_get_pin_level(J1850_P_TX) == VPW_ACTIVE) ? VPW_ACTIVE : VPW_PASSIVE
#define CURRENT_BUS_STATE ioport_get_pin_level(J1850_P_TX)
#define CURRENT_BUS_RX_STATE ioport_get_pin_level(J1850_VPW_RX_IDX)

#define WAIT_100us	us2cnt(100)		// 100us, used to count 100ms

// define J1850 VPW timing requirements in accordance with SAE J1850 standard
// all pulse width times in us
// transmitting pulse width
#define TX_SHORT	us2cnt(64)		// Short pulse nominal time
#define TX_LONG		us2cnt(128)		// Long pulse nominal time
#define TX_SOF		us2cnt(200)		// Start Of Frame nominal time
#define TX_EOD		us2cnt(175)		// End Of Data nominal time
#define TX_EOF		us2cnt(225)		// End Of Frame nominal time
#define TX_BRK		us2cnt(300)		// Break nominal time
#define TX_IFS		us2cnt(300)		// Inter Frame Separation nominal time

//See SAE J1850 chapter 6.6.2.5 for preferred use of In Frame Respond/Normalization pulse
#define TX_IFR_SHORT_CRC	us2cnt(64)	// short In Frame Respond, IFR contain CRC
#define TX_IFR_LONG_NOCRC 	us2cnt(128)	// long In Frame Respond, IFR contain no CRC

// receiving pulse width

#define RX_SHORT_MIN	us2cnt(0)	// minimum short pulse time
#define RX_SHORT_MAX	us2cnt(100)	// maximum short pulse time
#define RX_LONG_MIN		us2cnt(101)	// minimum long pulse time
#define RX_LONG_MAX		us2cnt(500)	// maximum long pulse time
#define RX_SOF_MIN		us2cnt(140)	// minimum start of frame time
#define RX_SOF_MAX		us2cnt(238)	// maximum start of frame time
#define RX_EOD_MIN		us2cnt(172)	// minimum end of data time
#define RX_EOD_MAX		us2cnt(228)	// maximum end of data time
#define RX_EOF_MIN		us2cnt(261)	// minimum end of frame time, ends at minimum IFS
#define RX_BRK_MIN		us2cnt(280)	// minimum break time
#define RX_IFS_MIN		us2cnt(280)	// minimum inter frame separation time, ends at next SOF
#define RX_TIMEOUT		us2cnt(2000);
//See SAE J1850 chapter 6.6.2.5 for preferred use of In Frame Respond/Normalization pulse
#define RX_IFR_SHORT_MIN	us2cnt(34)		// minimum short in frame respond pulse time
#define RX_IFR_SHORT_MAX	us2cnt(96)		// maximum short in frame respond pulse time
#define RX_IFR_LONG_MIN		us2cnt(96)		// minimum long in frame respond pulse time
#define RX_IFR_LONG_MAX		us2cnt(163)		// maximum long in frame respond pulse time

//-------------------------------------------------------------------------
// define error return codes
#define VPW_RETURN_CODE_UNKNOWN    0
#define VPW_RETURN_CODE_OK         1
#define VPW_RETURN_CODE_BUS_BUSY   2
#define VPW_RETURN_CODE_BUS_ERROR  3
#define VPW_RETURN_CODE_DATA_ERROR 4
#define VPW_RETURN_CODE_DATA       6

#define VPW_RETURN_CODE_NO_DATA       -5
#define VPW_RETURN_CODE_SOF_TOO_LONG  -7
#define VPW_RETURN_CODE_SOF_TOO_SHORT -8
#define VPW_RETURN_CODE_PULSE_TOO_SHORT -9
#define VPW_RETURN_CODE_PULSE_TOO_LONG -10
#define VPW_RETURN_CODE_HEADER_MISMATCH -11

#define  FILTER_TYPE_PASS true
#define  FILTER_TYPE_BLOCK false

#define VPW_PIO_CHANNEL PIOA
#define VPW_PIO_CHANNEL_ID ID_PIOA

#define WIDTH  (8 * sizeof(uint8_t))
#define TOPBIT (1 << (WIDTH - 1))
#define POLYNOMIAL 0x1D


#define CRC_TABLE_SIZE 256

static uint8_t crcTable[CRC_TABLE_SIZE];



static uint32_t PulseWidthDivisor = 1;
static bool VPWFilterEnable = false;
static Bool Is4xMode = false;

void VPWEnable(void);
void VPWDisable(void);
void VPWEnter4xMode(void);
void VPWEnter1xMode(void);
void VPWReceiveNetworkMessage(unsigned char *mbuf, uint32_t MaxBytes);
uint8_t VPWSendNetworkMessage(unsigned char *mbuf, unsigned short n);
void VPWInitalizeCRCLUT(void);
uint8_t VPWFastCRC(uint8_t const message[], int nBytes);



#endif /* J1850VPW_H_ */