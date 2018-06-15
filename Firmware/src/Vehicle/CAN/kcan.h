/*
 * can.h
 *
 * Created: 8/18/2017 11:08:07 AM
 *  Author: adeck
 */ 


#ifndef KCAN_H_
#define KCAN_H_

#include <asf.h>
#include "Message.h"
#include "KeplerConfiguration.h"
#include "isotp_types.h"

#define CHANNEL_HSC		0x00
#define CHANNEL_MSC		0x01
#define CHANNEL_SWC		0x02

#define MODE_STANDARD	0x00
#define MODE_ISOTP		0x01

#define MAX_NUMBER_OF_MAILBOXES 8
#define SYSTEM_CAN CAN0
#define SYSTEM_CAN_ID ID_CAN0
#define SYSTEM_CAN_IRQ CAN0_IRQn

#define CAN_COMM_RXMB_ID 1
#define CAN_COMM_TXMB_ID 0

static can_mb_conf_t tx_mailbox;
static can_mb_conf_t rx_mailbox;
extern uint32_t rx_mailbox_num;

void InitalizeCanSystem(uint8_t DataRate, uint32_t ul_sysclk);
void HandleSendCanRequest(Message_t *OutgoingMessage);
uint32_t SendStandardCanMessage(uint8_t arbitration_type, uint8_t *data, const uint8_t size);
static void reset_mailbox_conf(can_mb_conf_t *p_mailbox);
void DisableAllTransceivers();
/*
* Type 0 is an 11 bit mailbox 
* Type 1 is a 29 bit mailbox
* Mask is the message mask
* Pattern is the message pattern
* Length is the length of the mask and pattern. These must be the same length.
*/
void InitalizeReceiverMailbox(uint8_t type, uint8_t * Mask, uint8_t* Pattern);
bool RunTimer(uint16_t time_ms,bool stop, TimeoutCallback cb);
void ReceiveNormalMessage(can_mb_conf_t *mb);
void message_received(const IsoTpMessage* message);
void delayms(uint32_t delay);
void RemoveMailbox(uint8_t MailboxID);
#endif /* CAN_H_ */