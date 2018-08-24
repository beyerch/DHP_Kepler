/*
 * can.c
 *
 * Created: 8/18/2017 11:10:04 AM
 *  Author: adeck
 */ 

//See the ASF documentation for the CAN controller.

#include "kcan.h"
 #include "isotp.h"

 IsoTpShims shims;
 IsoTpReceiveHandle rx_handle;
 TimeoutCallback TimeoutCB;
 
 
uint32_t rx_mailbox_num = CAN_COMM_RXMB_ID;
  uint32_t ErrorCount = 0;
  
void InitalizeCanSystem(uint8_t DataRate, uint32_t ul_sysclk)
{
	uint32_t _DataRate = 0;
	/*
	#define CAN_BPS_1000K                 1000
	#define CAN_BPS_800K                  800
	#define CAN_BPS_500K                  500
	#define CAN_BPS_250K                  250
	#define CAN_BPS_125K                  125
	#define CAN_BPS_50K                   50
	#define CAN_BPS_25K                   25
	#define CAN_BPS_10K                   10
	#define CAN_BPS_5K                    5
	*/
	switch(DataRate)
	{
		case 0:
		_DataRate = CAN_BPS_1000K;
		break;
		
		case 1:
		_DataRate = CAN_BPS_800K;
		break;
		
		case 2:
		_DataRate = CAN_BPS_500K;
		break;
		
		case 3:
		_DataRate = CAN_BPS_250K;
		break;
		
		case 4:
		_DataRate = CAN_BPS_125K;
		break;
		
		case 5: 
		_DataRate = CAN_BPS_50K;
		break;
		
		case 6:
		_DataRate = CAN_BPS_25K;
		break;
		
		case 7:
		_DataRate = CAN_BPS_10K;
		break;
		
		case 8:
		_DataRate = CAN_BPS_5K;
		break;
		
		default:
		//TODO: ERROR
		return;
	}
	//Enable the CAN system
	pmc_enable_periph_clk(SYSTEM_CAN_ID);
	//Initalize the can system
	can_init(SYSTEM_CAN, ul_sysclk, _DataRate);
	SYSTEM_CAN->CAN_BR |= CAN_BR_SMP_THREE;
	//Disable all the trasceivers and then enable the HSC transeiver 
	DisableAllTransceivers();
	ioport_set_pin_level(HSC_NEN, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(HSC_NRM, IOPORT_PIN_LEVEL_HIGH);
	
	//Init Timer 
	sysclk_enable_peripheral_clock(TP_TIMEOUT_TIMER_ID);
	
	tc_stop(TP_TIMEOUT_TIMER, TP_TIMEOUT_TIMER_CHANNEL);
	
	tc_init(
	TP_TIMEOUT_TIMER,
	TP_TIMEOUT_TIMER_CHANNEL,
	TC_CMR_TCCLKS_TIMER_CLOCK4|
	TC_CMR_BURST_NONE|TC_CMR_CPCTRG
	);
	//Enable can interrupts
	can_enable_interrupt(SYSTEM_CAN, CAN_IER_ERRA | CAN_IER_WARN | CAN_IER_ERRP | CAN_IER_BOFF | CAN_IER_BERR);
	//Set the ISO15765 Shims
	shims = isotp_init_shims( NULL, SendStandardCanMessage, RunTimer, delayms);
	//Set the rx handle for iso15765
	rx_handle = isotp_receive(&shims,0x00,message_received);
}
//Delay shim
void delayms(uint32_t delay)
{
	delay_ms(delay);
}
//Send a can message to the network
void HandleSendCanRequest(Message_t *OutgoingMessage)
{
	switch(OutgoingMessage->buf[1])
	{
		case MODE_STANDARD:	
		//Just send the message
			SendStandardCanMessage(OutgoingMessage->buf[2], OutgoingMessage->buf+3, OutgoingMessage->Size-3);
		break;
		//Send the message with ISOTP processing
		case MODE_ISOTP:
		{
			
			if(OutgoingMessage->buf[2] == 0)
			{
				isotp_send(&shims, (OutgoingMessage->buf[5] << 8) | OutgoingMessage->buf[6], OutgoingMessage->buf+5, OutgoingMessage->Size-5, NULL);
			} else
			{
				isotp_send_ext(&shims, (OutgoingMessage->buf[6] << 8) | OutgoingMessage->buf[7], OutgoingMessage->buf+6, OutgoingMessage->Size-6, NULL, OutgoingMessage->buf[3]);	
			}
			
		}
		break;
	}
}
//Set the active transceiver
void SetActiveTransceiver(uint8_t Transveiver)
{
	DisableAllTransceivers();
	switch(Transveiver)
	{
		case CHANNEL_HSC:
			ioport_set_pin_level(HSC_NEN, IOPORT_PIN_LEVEL_LOW);
			ioport_set_pin_level(HSC_NRM, IOPORT_PIN_LEVEL_HIGH);
		break;
		
		case CHANNEL_MSC:
		
		break;
		
		case CHANNEL_SWC: 
		break;
		
		default:
		//TODO: ERROR
		return;
		
	}
	
}
//Disables all the transceivers
void DisableAllTransceivers()
{
	ioport_set_pin_level(SWC_RX_EN, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(SWC_MODE_1, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(SWC_MODE_2, IOPORT_PIN_LEVEL_HIGH);

	ioport_set_pin_level(MSC_NEN, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(MSC_NRM, IOPORT_PIN_LEVEL_HIGH);

	ioport_set_pin_level(HSC_NEN, IOPORT_PIN_LEVEL_HIGH);
	ioport_set_pin_level(HSC_NRM, IOPORT_PIN_LEVEL_HIGH);
}
/*
This function is called when a standard can message must be sent to the network. 
It waits for the transmit mailbox to become available, sets it up, and sends the message. 
Any response is handled by the CAN receiver logic.
*/
uint32_t SendStandardCanMessage(uint8_t arbitration_type, uint8_t *data, const uint8_t size)
{
	uint32_t retval = 0;
	uint32_t arbitrationID = 0;
	uint32_t CanStatusReg = CAN0->CAN_SR;
	
	if(size - 4 > 8)
	{
		//TODO:ERROR
		//Too much data for a standard frame!
		return;
	}
	
	//switch(channel)
	//{
		//case CHANNEL_HSC:
			//DisableAllTransceivers();
			//ioport_set_pin_level(HSC_NEN, IOPORT_PIN_LEVEL_LOW);
			//ioport_set_pin_level(HSC_NRM, IOPORT_PIN_LEVEL_HIGH);
		//break;
		//
		//default:
		////Unsupported Error	
		//return;
		//break;
	//}
	
	arbitrationID = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | (data[3] << 0);

	reset_mailbox_conf(&tx_mailbox);
	tx_mailbox.ul_mb_idx = CAN_COMM_TXMB_ID;
	tx_mailbox.uc_obj_type = CAN_MB_TX_MODE;

	tx_mailbox.ul_id = CAN_MID_MIDvA(arbitrationID) | CAN_MID_MIDvB((arbitrationID >>16));
	if(arbitration_type)
	{
		tx_mailbox.ul_id |= CAN_MID_MIDE;
	}
	
	tx_mailbox.uc_tx_prio = 2;
	tx_mailbox.uc_id_ver = 0;
	tx_mailbox.ul_id_msk = 0;
	tx_mailbox.uc_length = 8;
	can_mailbox_init(SYSTEM_CAN, &tx_mailbox);
	
	memcpy(&tx_mailbox.ul_datal,data+4, 4*sizeof(Byte));
	memcpy(&tx_mailbox.ul_datah,data+8, 4*sizeof(Byte));
	//memset(&tx_mailbox.ul_datal+(size-3),0x00,8-(size - 4));
	retval = can_mailbox_write(SYSTEM_CAN, &tx_mailbox);
	
	int i =0;
	if(data[4] == 0x00)
	{
		i++;
	}
	
	//TODO: Report TX error here
	if(retval)
	{
		return;
	}
	//can_mailbox_send_transfer_cmd(SYSTEM_CAN, &hsc_tx_mailbox);
	can_global_send_transfer_cmd(CAN0,  (0x1u << CAN_COMM_TXMB_ID));
}

//Initalizes a receiver mailbox, see the datasheet/asf documentation for information on mailboxs
void InitalizeReceiverMailbox(uint8_t type, uint8_t * Mask, uint8_t* Pattern)
{
	
	//TODO: HANDLE MORE THAN 7 RX BOX
	reset_mailbox_conf(&rx_mailbox);
	
	rx_mailbox.ul_mb_idx = rx_mailbox_num;
	rx_mailbox.uc_obj_type = CAN_MB_RX_MODE;

	rx_mailbox.ul_id = CAN_MID_MIDvA(Pattern[2] << 8 | Pattern[3]) | CAN_MID_MIDvB(Pattern[0] << 8 | Pattern[1]);
	rx_mailbox.ul_id_msk = CAN_MID_MIDvA(Mask[2] << 8 | Mask[3]) | CAN_MID_MIDvB(Mask[0] << 8 | Mask[1]);
	
	if(type)
	{
		rx_mailbox.ul_id |= CAN_MID_MIDE;
		rx_mailbox.ul_id_msk |= CAN_MAM_MIDE;
	}
	
	can_mailbox_init(SYSTEM_CAN, &rx_mailbox);
	
	can_enable_interrupt(SYSTEM_CAN, (0x1u << rx_mailbox_num++));
	NVIC_EnableIRQ(SYSTEM_CAN_IRQ);
	NVIC_SetPriority(SYSTEM_CAN_IRQ, 7);
	reset_mailbox_conf(&rx_mailbox);
	
	
}
//Removes a mailbox
void RemoveMailbox(uint8_t MailboxID)
{
	rx_mailbox.ul_mb_idx = MailboxID;
	rx_mailbox.uc_obj_type = CAN_MB_DISABLE_MODE;
	can_mailbox_init(SYSTEM_CAN, &rx_mailbox);	
	can_disable_interrupt(SYSTEM_CAN, (0x1u << rx_mailbox_num));	
	reset_mailbox_conf(&rx_mailbox);
	rx_mailbox_num--;
	if(rx_mailbox_num <= 0)
	{
		rx_mailbox_num = 1;	
	}
}
//Resets the mailbox configuration vairables
static void reset_mailbox_conf(can_mb_conf_t *p_mailbox)
{
	p_mailbox->ul_mb_idx = 0;
	p_mailbox->uc_obj_type = 0;
	p_mailbox->uc_id_ver = 0;
	p_mailbox->uc_length = 0;
	p_mailbox->uc_tx_prio = 0;
	p_mailbox->ul_status = 0;
	p_mailbox->ul_id_msk = 0;
	p_mailbox->ul_id = 0;
	p_mailbox->ul_fid = 0;
	p_mailbox->ul_datal = 0;
	p_mailbox->ul_datah = 0;
}
//Timer interuppty
TC3_Handler()
{
	if ((tc_get_status(TP_TIMEOUT_TIMER, TP_TIMEOUT_TIMER_CHANNEL) & TC_SR_CPCS) == TC_SR_CPCS)
	{
		NVIC_ClearPendingIRQ(TC3_IRQn);
		TimeoutCB();
		
		Error_T TPTImeoutError;
		TPTImeoutError.ThrowerID = THROWER_ID_ISO_TP;
		TPTImeoutError.ErrorMajor = TP_TIMEOUT;
		TPTImeoutError.ErrorMinor = ERROR_NO_MINOR_CODE;
		ThrowError(&TPTImeoutError);
	}
}
//CAN interrupt
CAN0_Handler()
 {
	 uint8_t* FlowControlMessage;
	 uint8_t FlowControlMessageSize;
	 uint8_t retval;
	 uint8_t Mailbox;
	 uint32_t CanStatusReg;
	 
	  iram_size_t error = SYSTEM_CAN->CAN_SR & (CAN_IMR_ERRA | CAN_IMR_WARN | CAN_IMR_ERRP | CAN_IMR_BOFF | CAN_IMR_BERR);
	  if(error)
	  {
		  CanStatusReg = CAN0->CAN_SR;
		  SYSTEM_CAN->CAN_MR = CAN_MR_CANEN;  
	  }
//Check each mailbox
	 for(Mailbox = CAN_COMM_RXMB_ID; Mailbox < MAX_NUMBER_OF_MAILBOXES; Mailbox++)
	 {
		//IF one is ready handle it
		 iram_size_t ready = SYSTEM_CAN->CAN_MB[Mailbox].CAN_MSR & CAN_MSR_MRDY;
		 if (  ready == CAN_MSR_MRDY)
		 {
			 rx_mailbox.ul_mb_idx = Mailbox;
			 //read the mailbox
			 can_mailbox_read(SYSTEM_CAN, &rx_mailbox);			 		
			 
			 
			 /*This code will attempt to process the isoTP messages in the driver software*/
			 
			 uint32_t MessageID = (( rx_mailbox.ul_id & 0x1FFC0000) >> 18) |  rx_mailbox.ul_fid;
			 uint8_t MessageIDArr[5];
			 
			 MessageIDArr[0] = (MessageID >> 24) & 0xFF;
			 MessageIDArr[1] = (MessageID >> 16) & 0xFF;
			 MessageIDArr[2] = (MessageID >> 8) & 0xFF;
			 MessageIDArr[3] = MessageID & 0xFF;
			 
			 uint8_t CANMessageReceived[17] = {START_BYTE, 0x00, 0x0E, NETWORK_MESSAGE, 0x02 ,MessageIDArr[0], MessageIDArr[1], MessageIDArr[2], MessageIDArr[3], 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
			 //if(MessageIDArr[2] == 0x03)
			 //{
			 //return;
			 //}
			 memcpy(CANMessageReceived+9,&rx_mailbox.ul_datal,8);
			 
			 Message_t CanSfMessage;
			 CanSfMessage.buf = CANMessageReceived;
			 CanSfMessage.Size = 17;
			 WriteMessage(&CanSfMessage);
			 
			 /*This code will filter it and run it against the firmware isotp processor. Uncomment it for that.*/
			 /*Neither option works and this was the major hangup of this project. */
			 
			 //retval = RunFilters(MessageIDArr,4,&FlowControlMessage, &FlowControlMessageSize);
			 //
			 //if(retval == 0)
			 //{
				 ////No matching filters
				 //return;
			 //}
			 //else if(retval == 1)
			 //{
				//uint8_t CANMessageReceived[17] = {START_BYTE, 0x00, 0x0E, NETWORK_MESSAGE, 0x02 ,MessageIDArr[0], MessageIDArr[1], MessageIDArr[2], MessageIDArr[3], 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
					////if(MessageIDArr[2] == 0x03)
					////{
						////return;
					////}
				//memcpy(CANMessageReceived+9,&rx_mailbox.ul_datal,8);	
				//
				//Message_t CanSfMessage;
				//CanSfMessage.buf = CANMessageReceived;
				//CanSfMessage.Size = 17;
				//WriteMessage(&CanSfMessage);
				 //
			 //}
			 //else if(retval == 3)
			 //{
				 //
				 ////uint8_t CANMessageReceived[17] = {START_BYTE, 0x00, 0x0E, NETWORK_MESSAGE, 0x03, MessageIDArr[0], MessageIDArr[1], MessageIDArr[2], MessageIDArr[3], 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
				 //////if(MessageIDArr[2] == 0x03)
				 //////{
				 //////return;
				 //////}
				 ////memcpy(CANMessageReceived+9,&rx_mailbox.ul_datal,8);
				 ////Message_t CanSfMessage;
				 ////CanSfMessage.buf = CANMessageReceived;
				 ////CanSfMessage.Size = 17;
				 ////WriteMessage(&CanSfMessage);
				 //
				 ////Flow control filter passed, process with ISO15765
				 //isotp_continue_receive(
				 //&shims,
				 //&rx_handle,
				 //MessageID,
				 //(FlowControlMessage[0] << 24) | (FlowControlMessage[1] << 16) | (FlowControlMessage[2] << 8) | FlowControlMessage[3],
				 //FlowControlMessage[4],
				 //&rx_mailbox.ul_datal,
				 //7,
				 //9
				 //);
			 //}
		 } 
	 }
 }
 
 /* ISO-TP SHIMS*/
 bool RunTimer(uint16_t time_ms,bool stop, TimeoutCallback cb)
 {
	 if(stop)
	 {
		 tc_stop(TP_TIMEOUT_TIMER, TP_TIMEOUT_TIMER_CHANNEL);
		 NVIC_DisableIRQ(TC3_IRQn);
		 return true;
	 }
	 //Run the timer for X milliseconds
	 tc_write_rc(TP_TIMEOUT_TIMER, TP_TIMEOUT_TIMER_CHANNEL, cpu_ms_2_cy(time_ms,96000000));
	 tc_enable_interrupt(TP_TIMEOUT_TIMER, TP_TIMEOUT_TIMER_CHANNEL,  TC_IER_CPCS);
	 NVIC_EnableIRQ(TC3_IRQn);
	 NVIC_SetPriority(TC3_IRQn,0);
	 tc_start(TP_TIMEOUT_TIMER, TP_TIMEOUT_TIMER_CHANNEL);
	 TimeoutCB = cb;
	 return true;
 }

 //Called when a complete can message is received
 //TODO: Proper Formatting here
 void message_received( const IsoTpMessage* message) {
	 Message_t CanMessageReceived;
	  uint8_t MessageIDArr[5];
	  
	MessageIDArr[0] = (message->arbitration_id >> 24) & 0xFF;
	MessageIDArr[1] = (message->arbitration_id >> 16) & 0xFF;
	MessageIDArr[2] = (message->arbitration_id >> 8) & 0xFF;
	MessageIDArr[3] = message->arbitration_id & 0xFF;
	message->payload[0] = START_BYTE;
	 
	message->payload[1] = ( ((message->size) - 3) & 0xFF00) >> 8;
	message->payload[2] = (message->size) - 3 & 0x00FF;
	message->payload[3] = NETWORK_MESSAGE;
	message->payload[4] = 3;
	message->payload[5] = MessageIDArr[0];
	message->payload[6] = MessageIDArr[1];
	message->payload[7] = MessageIDArr[2]; 
	message->payload[8] = MessageIDArr[3];
	 
	CanMessageReceived.buf = message->payload;
	CanMessageReceived.Size = message->size;
	WriteMessage(&CanMessageReceived);
 }
 

 
 /* ISO-TP SHIMS*/