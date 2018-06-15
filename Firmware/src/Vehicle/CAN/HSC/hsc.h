/*
 * hsc.h
 *
 * Created: 2/25/2017 1:56:47 PM
 *  Author: Aaron
 */ 


#ifndef HSC_H_
#define HSC_H_

#include <asf.h>
#include "console.h"
#include "runtimer.h"
#include "KeplerConfiguration.h"
#include "isotp_types.h"

#define MAX_NUMBER_OF_MAILBOXES 8
#define SYSTEM_CAN CAN0
#define SYSTEM_CAN_ID ID_CAN0
#define SYSTEM_CAN_IRQ CAN0_IRQn

#define CAN_COMM_RXMB_ID 0
#define CAN_COMM_TXMB_ID 3


static can_mb_conf_t hsc_tx_mailbox;
static can_mb_conf_t hsc_rx_mailbox;



void HSCEnable(uint32_t CANBaud, uint32_t ul_sysclk);
void HSCDisable(void);
void HSCSetRxSettings(uint32_t MessageID, uint32_t MessageIDMask);
uint32_t HSCSend(uint32_t arbitration_id, uint8_t *data, const uint8_t size);
void HscIsoTpSend(Message_t *message);
static void reset_mailbox_conf(can_mb_conf_t *p_mailbox);
void DisableAllTransceivers(void);
void ReceiveNormalMessage(can_mb_conf_t *mb);
void message_received(const IsoTpMessage* message);
bool RunTimer(uint16_t time_ms,bool stop, TimeoutCallback cb);
#endif /* HSC_H_ */