/*
 * j1850vpw.c
 *
 * Created: 2/25/2017 1:06:04 AM
 *  Author: adeck
 */ 

#include "j1850vpw.h"

static void VPWNetworkMessageStartDetect(const uint32_t id, const uint32_t index);


void VPWEnable()
{
	sysclk_enable_peripheral_clock(VPW_TX_TIMER_ID);
	sysclk_enable_peripheral_clock(VPW_RX_TIMER_ID);
	sysclk_enable_peripheral_clock(VPW_PIO_CHANNEL_ID);
	
	pio_set_input(VPW_PIO_CHANNEL,J1850_VPW_RX,PIO_DEFAULT);
	pio_handler_set(VPW_PIO_CHANNEL, VPW_PIO_CHANNEL_ID, J1850_VPW_RX, PIO_IT_FALL_EDGE, VPWNetworkMessageStartDetect);
	pio_enable_interrupt(PIOA, J1850_VPW_RX);
	
	tc_stop(VPW_TIMER, VPW_RX_TIMER_CHANNEL);
	tc_init(
	VPW_TIMER,
	VPW_RX_TIMER_CHANNEL,
	TC_CMR_TCCLKS_TIMER_CLOCK2|
	TC_CMR_BURST_NONE 
	);
	tc_start(VPW_TIMER, VPW_RX_TIMER_CHANNEL);
	
	NVIC_EnableIRQ(PIOA_IRQn);
	NVIC_SetPriority(PIOA_IRQn,5);
	GO_PASSIVE
	VPWEnter1xMode();
	VPWInitalizeCRCLUT();
}

void VPWDisable()
{
	pio_disable_interrupt(PIOA, J1850_VPW_RX);
	NVIC_DisableIRQ(PIOA_IRQn);
	sysclk_disable_peripheral_clock(VPW_TX_TIMER_ID);
	sysclk_disable_peripheral_clock(VPW_RX_TIMER_ID);
	sysclk_disable_peripheral_clock(VPW_PIO_CHANNEL_ID);
	
}

static void VPWNetworkMessageStartDetect(const uint32_t id, const uint32_t index)
{
	if(id == VPW_PIO_CHANNEL_ID && index == J1850_VPW_RX)
	{
		//Possible SOF, lets check that!
		cpu_irq_disable();
		VPWReceiveNetworkMessage(VehicleMessageBuffer, VPW_BUF_SIZE);	
		cpu_irq_enable();
	}	
}

void VPWReceiveNetworkMessage(unsigned char *mbuf, uint32_t MaxBytes)
{
	uint32_t TimerValue = 0;
	uint32_t ByteCount = 0;
	uint8_t CurrentByte = 0;
	uint8_t ShiftValue = 0;
	uint32_t BusState = 0;
	uint32_t *MessageBufferStartAddress = mbuf;
	Message_t NetworkMessage;
	//Advance the buffer by 4 to leave room for KAVI Header
	mbuf += 5;

	//Start the timer
	tc_start(VPW_TIMER, VPW_RX_TIMER_CHANNEL);

	//Time the possible SOF
	while(CURRENT_BUS_RX_STATE == VPW_RX_ACTIVE)
	{
		if(tc_read_cv(VPW_TIMER, VPW_RX_TIMER_CHANNEL) > (RX_SOF_MAX / PulseWidthDivisor) )
		{
			Error_T InvalidLengthByteError;
			InvalidLengthByteError.ThrowerID = NETWORK_MESSAGE;
			InvalidLengthByteError.ErrorMajor = VPW_RETURN_CODE_SOF_TOO_LONG_EXCEPTION;
			InvalidLengthByteError.ErrorMinor = ERROR_NO_MINOR_CODE;
			ThrowError(&InvalidLengthByteError);

			return;
		}
	}
	//Check possible SOF Length and handle accordingly
	TimerValue = tc_read_cv(VPW_TIMER, VPW_RX_TIMER_CHANNEL);

	if( TimerValue > (RX_SOF_MIN/ PulseWidthDivisor))
	{
		//It was an SOF, start receiving the message
		while (ByteCount < MaxBytes)
		{
			//Reset current byte
			CurrentByte = 0;
			for(int bitNumber = 7; bitNumber >= 0; bitNumber--)
			{
				//Log bus state and start timing the state
				BusState = CURRENT_BUS_RX_STATE;
				tc_start(VPW_TIMER, VPW_RX_TIMER_CHANNEL);

				while(CURRENT_BUS_RX_STATE == BusState)
				{
					//Wait until the bus state changes, if we wait longer than EOD then were done!
					if(tc_read_cv(VPW_TIMER, VPW_RX_TIMER_CHANNEL) > (RX_EOD_MIN/ PulseWidthDivisor) )
					{
						goto HitEOD;
					}
				}

				//Get pulse width
				TimerValue = tc_read_cv(VPW_TIMER, VPW_RX_TIMER_CHANNEL);
				//Assign a value based on the length of the pulse
				if(TimerValue >= (RX_SHORT_MIN/ PulseWidthDivisor) && TimerValue <= (RX_SHORT_MAX/ PulseWidthDivisor))
				{
					ShiftValue = 1;
				}else if(TimerValue >= (RX_LONG_MIN/ PulseWidthDivisor) && TimerValue <= (RX_LONG_MAX/ PulseWidthDivisor))
				{
					ShiftValue = 0;
				}
				else
				{
					Error_T InvalidLengthByteError;
					InvalidLengthByteError.ThrowerID = NETWORK_MESSAGE;
					InvalidLengthByteError.ErrorMajor = VPW_RETURN_CODE_PULSE_TIMING_UNKOWN;
					InvalidLengthByteError.ErrorMinor = ERROR_NO_MINOR_CODE;
					ThrowError(&InvalidLengthByteError);
				}
				//Flip if assumed wrong state above
				if(BusState)
				{
					ShiftValue = !ShiftValue;
				}
				//Shift the value into the current byte
				CurrentByte |= (ShiftValue << bitNumber);
			}
			//Add the current byte to the main recieve buffer and increment the byte counter.
			*mbuf++ = CurrentByte;
			ByteCount++;
		
		}
	}
	else
	{
		return;
	}

 HitEOD:
	 //Reset buffer pointer to beginning
	 mbuf = MessageBufferStartAddress;
	 //Check the CRCs to ensure we got a good message
	 char crc = mbuf[ByteCount+4];
	 char actualCRC = VPWFastCRC(mbuf+5, ByteCount-1);
	 if(mbuf[ByteCount+4] != VPWFastCRC(mbuf+5, ByteCount-1))
	 {
		// uint8_t errrtn[] = {START_BYTE, 0x00,0x03,ERROR_RESPONSE, 0x00, VPW_RETURN_CODE_DATA_EXCEPTION};
		// WriteBufferOut(errrtn,6);
		 return;
	 }
	 
	ui_vehicle_vpw_rx_notify_off();
	
	if(RunFilters(mbuf+4, ByteCount, NULL, NULL))
	{
		//It passed the filters prep message with KAVI header
		
		//Increment ByteCount to account for command byte and network byte
		ByteCount++;
		ByteCount++;
		
		//Add KAVI header
		mbuf[0] = 0x02;
		mbuf[1] = (ByteCount >> 8) & 0xFF;
		mbuf[2] = (ByteCount & 0xFF);
		mbuf[3] = NETWORK_MESSAGE;
		mbuf[4] = 0x01;
		ByteCount += 3;
		//Send it out USB/BT
		NetworkMessage.buf = MessageBufferStartAddress;
		NetworkMessage.Size = ByteCount;
		WriteMessage(&NetworkMessage);
	}
 }
 


/*
void VPWReceiveNetworkMessage(unsigned char *mbuf, uint32_t MaxBytes)
{
	 uint32_t TimerValue = 0;
	 uint32_t ByteCount = 0;
	 uint8_t CurrentByte = 0;
	 uint8_t ShiftValue = 0;
	 uint32_t BusState = 0;
	 unsigned char *MessageBufferStartAddress = mbuf;
	 Message_t NetworkMessage;
	 
	 //Advance the buffer by 4 to leave room for standard Header
	 mbuf += 4;
	
	 //Start the timer
	 tc_start(VPW_TIMER, VPW_RX_TIMER_CHANNEL);

	 //Time the possible SOF
	 while(CURRENT_BUS_RX_STATE == VPW_RX_ACTIVE)
	 {
		 if(tc_read_cv(VPW_TIMER, VPW_RX_TIMER_CHANNEL) > (RX_SOF_MAX / PulseWidthDivisor) )
		 {
			 Error_T InvalidLengthByteError;
			 InvalidLengthByteError.ThrowerID = NETWORK_MESSAGE;
			 InvalidLengthByteError.ErrorMajor = VPW_RETURN_CODE_SOF_TOO_LONG_EXCEPTION;
			 InvalidLengthByteError.ErrorMinor = ERROR_NO_MINOR_CODE;
			 ThrowError(&InvalidLengthByteError);
			 
			
			 return;
		 }
	 }
	 //Check possible SOF Length and handle accordingly
	 TimerValue = tc_read_cv(VPW_TIMER, VPW_RX_TIMER_CHANNEL);

	 if( TimerValue > (RX_SOF_MIN/ PulseWidthDivisor))
	 {
		 //It was an SOF, start receiving the message
		
		 NetworkMessage.buf = mbuf;
		 
		 while (ByteCount < MaxBytes)
		 {
			 //Reset current byte
			 CurrentByte = 0;
			 
			 ui_vehicle_vpw_rx_notify();
			 
			 for(int bitNumber = 7; bitNumber >= 0; bitNumber--)
			 {
				 //Log bus state and start timing the state
				 BusState = CURRENT_BUS_RX_STATE;
				 tc_start(VPW_TIMER, VPW_RX_TIMER_CHANNEL);

				 while(CURRENT_BUS_RX_STATE == BusState)
				 {
					 //Wait until the bus state changes, if we wait longer than EOD then were done!
					 if(tc_read_cv(VPW_TIMER, VPW_RX_TIMER_CHANNEL) > (RX_EOD_MIN/ PulseWidthDivisor) )
					 {
						 goto HitEOD;
					 }
				 }

				 //Get pulse width
				 TimerValue = tc_read_cv(VPW_TIMER, VPW_RX_TIMER_CHANNEL);
				 //Assign a value based on the length of the pulse
				 if(TimerValue >= (RX_SHORT_MIN/ PulseWidthDivisor) && TimerValue <= (RX_SHORT_MAX/ PulseWidthDivisor))
				 {
					 ShiftValue = 1;
				 }else if(TimerValue >= (RX_LONG_MIN/ PulseWidthDivisor) && TimerValue <= (RX_LONG_MAX/ PulseWidthDivisor))
				 {
					 ShiftValue = 0;
				 }
				 else
				 {
					Error_T InvalidLengthByteError;
					InvalidLengthByteError.ThrowerID = NETWORK_MESSAGE;
					InvalidLengthByteError.ErrorMajor = VPW_RETURN_CODE_PULSE_TIMING_UNKOWN;
					InvalidLengthByteError.ErrorMinor = ERROR_NO_MINOR_CODE;
					ThrowError(&InvalidLengthByteError);
				 }
				 //Flip if assumed wrong state above
				 if(BusState)
				 {
					 ShiftValue = !ShiftValue;
				 }
				 //Shift the value into the current byte
				 CurrentByte |= (ShiftValue << bitNumber);
			 }
			 //Add the current byte to the main receive buffer and increment the byte counter.
			 *mbuf++ = CurrentByte;
			 ByteCount++;

			 //Check the header filter if it is enabled!
			 if(ByteCount == 2 && VPWFilterEnable == true  )			 {				 //Get the current header				 uint32_t header = ( *(mbuf-2) << 16) | (*(mbuf-1) << 8) | (*(mbuf) << 0);
				 //Compare the headers, return if they do not match
				 if( ! ((header & VPWHeaderMSK) == (VPWHeaderID & VPWHeaderMSK)) )
				 {
					ui_vehicle_vpw_rx_notify_off();
					 return;
					 
				 }
			 }
		 }
	 }
	 else
	 {
		 return;
	 }

	 HitEOD:
	 //Reset buffer pointer to beginning
	 mbuf = MessageBufferStartAddress;
	 //Check the CRCs to ensure we got a good message
	 if(mbuf[ByteCount+3] != VPWFastCRC(mbuf+4, ByteCount-1))
	 {
		 //SetVehicleVPW();
	 }
	 
	 //Add One (1) to ByteCount to account for command byte
	 ByteCount++;
	 //Add KAVI header
	 mbuf[0] = 0x02;
	 mbuf[1] = (ByteCount >> 8) & 0xFF;
	 mbuf[2] = (ByteCount & 0xFF);
	 mbuf[3] = NETWORK_MESSAGE;
	 ByteCount += 3;
	 ui_vehicle_vpw_rx_notify_off();
	 //Send it out USB/BT
	 NetworkMessage.buf = MessageBufferStartAddress;
	 NetworkMessage.Size = ByteCount;
	 WriteMessage(&NetworkMessage);
}
*/

uint8_t VPWSendNetworkMessage(unsigned char *mbuf, unsigned short n)
{
	
	pio_disable_interrupt(PIOA, J1850_VPW_RX);
	char bits, ch, mask;
	unsigned int period;
	tc_stop(VPW_TIMER, VPW_TX_TIMER_CHANNEL);
	tc_start(VPW_TIMER, VPW_TX_TIMER_CHANNEL);
	//Append CRC
	char crc = VPWFastCRC(mbuf,n);
	mbuf[n] = crc;
	n++;
	// wait_vpw_idle();
	//Resets timer
	tc_sync_trigger(VPW_TIMER);
	//Go Active on the J1850+ pin
	//ToggleVehicleVPWSend();
	GO_ACTIVE
	while(tc_read_cv(VPW_TIMER,VPW_TX_TIMER_CHANNEL) < TX_SOF);

	while(n != 0)
	{
		ui_vehicle_vpw_tx_notify();
		n--;
		ch=*mbuf++;
		bits = 8;
		mask = 1;
		while(bits)
		{
			bits--;
			if(bits&1)
			{
				//Go passive on the J1850+ pin
				GO_PASSIVE
				tc_sync_trigger(VPW_TIMER);
				period = ((ch & (mask << bits)) >> bits) ? TX_LONG : TX_SHORT;
				while(tc_read_cv(VPW_TIMER,VPW_TX_TIMER_CHANNEL) <= period)
				{
					if(IS_VPW_ACTIVE_LEVEL)
					{
						//Need to reset pointers and resend here, this was a bus collision.
						ui_vehicle_vpw_tx_notify_off();
						return 0; //Bus collision :(
					}
				}
			}
			else
			{
				GO_ACTIVE
				tc_sync_trigger(VPW_TIMER);
				period = ((ch & (mask << bits)) >> bits) ? TX_SHORT : TX_LONG;
				while(tc_read_cv(VPW_TIMER,VPW_TX_TIMER_CHANNEL) <= period);
			}
		}//while bits
	}//while n
	GO_PASSIVE
	tc_sync_trigger(VPW_TIMER);
	while(tc_read_cv(VPW_TIMER,VPW_TX_TIMER_CHANNEL) < TX_EOF);

	tc_stop(VPW_TIMER, VPW_TX_TIMER_CHANNEL);
	pio_enable_interrupt(PIOA, J1850_VPW_RX);
	ui_vehicle_vpw_tx_notify_off();
	return VPW_RETURN_CODE_OK;
}

void VPWEnter1xMode(void)
{
	tc_stop(VPW_TIMER, VPW_TX_TIMER_CHANNEL);

	tc_init(VPW_TIMER,VPW_TX_TIMER_CHANNEL,
	TC_CMR_TCCLKS_TIMER_CLOCK2|
	TC_CMR_BURST_NONE
	);
	tc_start(VPW_TIMER, VPW_TX_TIMER_CHANNEL);

	Is4xMode = false;
	PulseWidthDivisor = 1;

	uint8_t tmpRtn[] = {START_BYTE, 0x00,0x02,ENTER_VPW_1X, 0x01};
	Message_t EnterLowSpeedMessage;
	EnterLowSpeedMessage.buf = tmpRtn;
	EnterLowSpeedMessage.Size = 5;
	WriteMessage(&EnterLowSpeedMessage);
}

void VPWEnter4xMode(void)
{
	tc_stop(VPW_TIMER, VPW_TX_TIMER_CHANNEL);

	tc_init(VPW_TIMER,VPW_TX_TIMER_CHANNEL,
	TC_CMR_TCCLKS_TIMER_CLOCK1|
	TC_CMR_BURST_NONE
	);
	
	tc_start(VPW_TIMER, VPW_TX_TIMER_CHANNEL);

	Is4xMode = true;
	PulseWidthDivisor = 4;
	
	uint8_t tmpRtn[] = {START_BYTE, 0x00,0x02,ENTER_VPW_4X, 0x01};
	Message_t EnterHighSpeedMessage;
	EnterHighSpeedMessage.buf = tmpRtn;
	EnterHighSpeedMessage.Size = 5;
	WriteMessage(&EnterHighSpeedMessage);


}

void VPWInitalizeCRCLUT()
{
    uint8_t  remainder;
   
    for (int dividend = 0; dividend < 256; ++dividend)
    {
        remainder = dividend << (WIDTH - 8);
        
        for (uint8_t bit = 8; bit > 0; --bit)
        { 		
            if (remainder & TOPBIT)
            {
                remainder = (remainder << 1) ^ POLYNOMIAL;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
        crcTable[dividend] = remainder;
    }
}   

uint8_t VPWFastCRC(uint8_t const message[], int nBytes)
{
    uint8_t data;
    uint8_t remainder = 0xFF;

   
    for (int byte = 0; byte < nBytes; ++byte)
    {
        data = message[byte] ^ (remainder >> (WIDTH - 8));
        remainder = crcTable[data] ^ (remainder << 8);
    }
    return (~remainder);

} 
