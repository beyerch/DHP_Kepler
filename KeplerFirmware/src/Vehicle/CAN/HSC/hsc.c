/*
 * hsc.c
 *
 * Created: 2/25/2017 1:56:32 PM
 *  Author: Aaron
 */ 
 #include "hsc.h"
 #include "isotp.h"

IsoTpShims shims;
IsoTpReceiveHandle rx_handle;
TimeoutCallback TimeoutCB;

 void HSCEnable(uint32_t CANBaud, uint32_t ul_sysclk)
 {
	pmc_enable_periph_clk(SYSTEM_CAN_ID);
	
	DisableAllTransceivers();
	
	ioport_set_pin_level(HSC_NEN, IOPORT_PIN_LEVEL_LOW);
	ioport_set_pin_level(HSC_NRM, IOPORT_PIN_LEVEL_HIGH);
	
	can_init(SYSTEM_CAN, ul_sysclk, CAN_BPS_500K);
	can_disable_interrupt(SYSTEM_CAN, 1 << CAN_COMM_RXMB_ID);
	can_reset_all_mailbox(CAN0);
	NVIC_EnableIRQ(SYSTEM_CAN_IRQ);
	
	//Init Timer Here
	 sysclk_enable_peripheral_clock(TP_TIMEOUT_TIMER_ID);
	 
	 tc_stop(TP_TIMEOUT_TIMER, TP_TIMEOUT_TIMER_CHANNEL);
	 
	 tc_init(
	 TP_TIMEOUT_TIMER,
	 TP_TIMEOUT_TIMER_CHANNEL,
	 TC_CMR_TCCLKS_TIMER_CLOCK4|
	 TC_CMR_BURST_NONE|TC_CMR_CPCTRG
	 );
	
	
	shims = isotp_init_shims( WriteLine, HSCSend, RunTimer);
	rx_handle = isotp_receive(&shims,0x00,message_received);
 }
 
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


//Called when a complete can message is received
//TODO: Proper Formatting here

 void message_received( const IsoTpMessage* message) {
	 Message_t CanMessageReceived;
	 message->payload[0] = START_BYTE;
	 
	 message->payload[1] = ( ((message->size) - 3) & 0xFF00) >> 8;
	 message->payload[2] = (message->size) - 3 & 0x00FF;
	 message->payload[3] = NETWORK_MESSAGE;
	 message->payload[4] = (message->arbitration_id & 0xFF00) >> 8;
	 message->payload[5] = message->arbitration_id & 0x00FF;
	 CanMessageReceived.buf = message->payload;
	 CanMessageReceived.Size = message->size;
	 WriteMessage(&CanMessageReceived);
 }
 
 void HscIsoTpSend(Message_t *message)
 {
	 
	 
	if(message->buf[1] == 0) 
	{
		isotp_send(&shims, (message->buf[2] << 8) | message->buf[3], message->buf+4, message->Size-4, NULL);
	}
	else
	{
		isotp_send_ext(&shims, (message->buf[3] << 8) | message->buf[4], message->buf+5, message->Size-5, NULL, message->buf[2]);
	}
 }
 
 
 void HSCDisable()
 {
	reset_mailbox_conf(&hsc_rx_mailbox);
	reset_mailbox_conf(&hsc_tx_mailbox);
 }

 void HSCSetRxSettings(uint32_t MessageID, uint32_t MessageIDMask)
 {
	reset_mailbox_conf(&hsc_rx_mailbox);
	hsc_rx_mailbox.ul_mb_idx = CAN_COMM_RXMB_ID;
	hsc_rx_mailbox.uc_obj_type = CAN_MB_RX_MODE;
	hsc_rx_mailbox.ul_id = CAN_MID_MIDvA(MessageID);
	hsc_rx_mailbox.ul_id_msk = CAN_MID_MIDvA(MessageIDMask);
	can_mailbox_init(SYSTEM_CAN, &hsc_rx_mailbox);
	
	can_enable_interrupt(SYSTEM_CAN, (0x1u << CAN_COMM_RXMB_ID));
	NVIC_EnableIRQ(SYSTEM_CAN_IRQ);

	//NVIC_SetPriority(SYSTEM_CAN_IRQ, 1);
	char tmpRtn[] = {START_BYTE,0x00,0x02,INIT_STD_MBX_RX,0x01};
    Message_t CanRxSettingsAck;
	CanRxSettingsAck.buf = tmpRtn;
	CanRxSettingsAck.Size = 5;
	WriteMessage(&CanRxSettingsAck);
 }

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

uint32_t HSCSend(uint32_t arbitration_id, uint8_t *data, const uint8_t size)
 {
	
	//WriteString("Sending ISOTP Message to ");
	//WriteChar((arbitration_id & 0xFF00) >> 8);
	//WriteChar(arbitration_id & 0xFF);
	//WriteLine(":");
	//
	//for(int i = 0; i <= size; i++)
	//{
		//WriteChar(data[i]);
	//}
	//WriteLine("");
	//WriteLine("End Message.");
	
	uint32_t retval = 0;
	
	reset_mailbox_conf(&hsc_tx_mailbox);
	hsc_tx_mailbox.ul_mb_idx = CAN_COMM_TXMB_ID;
	hsc_tx_mailbox.uc_obj_type = CAN_MB_TX_MODE;
	hsc_tx_mailbox.ul_id = CAN_MID_MIDvA(arbitration_id);
	hsc_tx_mailbox.uc_tx_prio = 2;
	hsc_tx_mailbox.uc_id_ver = 0;
	hsc_tx_mailbox.ul_id_msk = 0;
	hsc_tx_mailbox.uc_length = 8;
	can_mailbox_init(SYSTEM_CAN, &hsc_tx_mailbox);
	
	memcpy(&hsc_tx_mailbox.ul_datal,data, 4*sizeof(Byte));
	memcpy(&hsc_tx_mailbox.ul_datah,data+4, 4*sizeof(Byte));

	retval = can_mailbox_write(SYSTEM_CAN, &hsc_tx_mailbox);
	
	if(retval)
	{
		while(true);
	}
	//TODO: CHECK READY HERE THEN REMOVE 1MS DELAY!
	//can_mailbox_send_transfer_cmd(SYSTEM_CAN, &hsc_tx_mailbox);
	can_global_send_transfer_cmd(CAN0,  (0x1u << CAN_COMM_TXMB_ID));
	
	
	//return retval;
}

 void CAN0_Handler()
 {
	 iram_size_t ready = SYSTEM_CAN->CAN_MB[CAN_COMM_RXMB_ID].CAN_MSR & CAN_MSR_MRDY;
	if ( ready  == CAN_MSR_MRDY)
	{
		//Free the mailbox
		can_mailbox_read(SYSTEM_CAN, &hsc_rx_mailbox);				//Handle the incomming message
		//isotp_continue_receive(
				//&shims, 
				//&rx_handle,
				//(( hsc_rx_mailbox.ul_id & 0x1FFC0000) >> 18) |  hsc_rx_mailbox.ul_fid,
				//&hsc_rx_mailbox.ul_datal,8);

		isotp_continue_receive(&shims, &rx_handle,(( hsc_rx_mailbox.ul_id & 0x1FFC0000) >> 18) |  hsc_rx_mailbox.ul_fid,&hsc_rx_mailbox.ul_datal,8, 6);
	}
 }

void ReceiveNormalMessage(can_mb_conf_t *mb)
{
	char MessageIDHigh = (mb->ul_id >> 8) & 0xFF;
	char MessageIDLow = mb->ul_id & 0xFF;

	//TODO Change this such that the header is placed at the start of the vehicle buffer and then used to avoid corruption issues
	
	//uint8_t CANMessageReceived[7] = {START_BYTE, 0x00, 0x0C, NETWORK_MESSAGE, N_PCI_FF ,MessageIDHigh, MessageIDLow};
	//memcpy(VehicleMessageBuffer,CANMessageReceived,7);
	//Message_t CanSfMessage;
	//CanSfMessage.buf = VehicleMessageBuffer;
	//CanSfMessage.Size = CurrentMessage.len+7;
	//WriteMessage(&CanSfMessage);
}
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
  
  /*ISO TP SHIM DEFINITIONS*/
