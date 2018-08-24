/*
 * MessageReceiver.c
 *
 * Created: 2/24/2017 2:05:37 PM
 *  Author: Aaron
 */ 
//USB Message Receive
#include "MessageHandler.h"

char MessageAvailable = 0;
Message_t IncommingMessage;
//Message in
void ReceiveUSBMessage(uint8_t port)
{
	//Read a byte
	int currByte = udi_cdc_getc();

	if(currByte == START_BYTE )
	{
		//Set the LEDs
		ui_com_rx_start();
		//Get the lengths and the command
		int LenA = udi_cdc_getc();
		int LenB = udi_cdc_getc();
		int CommandLength = (LenA << 8) | LenB;
		int ByteCounter = 0;	
		
		//Make sure we have enough bytes available
		if( (udi_cdc_get_available_rx_bytes() - 3) < CommandLength)
		{
			Error_T InvalidLengthByteError;
			InvalidLengthByteError.ThrowerID = THROWER_ID_COMMAND_RESPONSE_SYSTEM;
			InvalidLengthByteError.ErrorMajor = INVALID_LENGTH_BYTES;
			InvalidLengthByteError.ErrorMinor = ERROR_NO_MINOR_CODE;
			ThrowError(&InvalidLengthByteError);
			udi_cdc_flush_rx_buffer();
			return;
		}
		//Set the buffer up and read the data
		IncommingMessage.buf = IncomingMessageBuffer;
		IncommingMessage.Size = CommandLength;
		udi_cdc_read_buf(IncommingMessage.buf, IncommingMessage.Size);
		udi_cdc_flush_rx_buffer();
		//Set the LEDs
		ui_com_rx_stop();
		//We have a message available
		MessageAvailable = 1;
	}	
	else
	{
		//Was not a start byte
		Error_T InvalidLengthByteError;
		InvalidLengthByteError.ThrowerID = THROWER_ID_COMMAND_RESPONSE_SYSTEM;
		InvalidLengthByteError.ErrorMajor = INVALID_START_BYTE_EXCEPTION;
		InvalidLengthByteError.ErrorMinor = currByte;
		ThrowError(&InvalidLengthByteError);
		udi_cdc_flush_rx_buffer();
	}

}


//Message Run
void HandleMessage(Message_t *message)
{
	//Switch on the command byte
	switch(message->buf[0])
	{
		case STATUS_REQUEST:
			SendStatusReport(STATUS_REQUEST);
		break;
		
		case SET_TX_MODE:
			SetCommunicationMode(message);
		break;
		
		case VERSION_REQUEST:
			SendVersionReport();
		break;
		
		case SET_INTERFACE_MODE: // A0
			SetInterfaceMode(message);
		break;
		
		case SEND_MESSAGE: //A1
			WriteVehicleMessage(message);
		break;
		
		case READ_UNIQUE_ID:
			SendIdenifierReport();
		break;
		
		case ENTER_SECURE_MODE:
			EnterSecureMode( (IncomingMessageBuffer[1] << 24) | (IncomingMessageBuffer[2] << 16) | (IncomingMessageBuffer[3] << 8) | (IncomingMessageBuffer[4] << 0)  );
		break;

		case EXIT_SECURE_MODE:
			ExitSecureMode();
		break;
		
		case ENTER_VPW_1X:
			VPWEnter1xMode();
		break;
		
		case ENTER_VPW_4X:
			VPWEnter4xMode();
		break;
			
		case FIRMWARE_UPDATE:
			EnterBootloader();
		break;

		case CREATE_CAN_FILTER:
			CreateCanFilter(message);
		break;
		
		case DELETE_ALL_CAN_FILTERS:
			DeleteAllFilters();
		break;
		
		case DELETE_CAN_FILTER:
		//RemoveFilter(message->buf[1]);
		RemoveMailbox(message->buf[1]); 		
		break;
		
		case READ_ADC_VALUE: 
			ReadADCValues();
		break;
		
		case RESET_DEVICE:
			ResetDevice();
		break;
	}
}
//Soft Reset of the device
void ResetDevice()
{
	SystemConfiguration.bluetooth_unlocked = 0;
	SystemConfiguration.in_secure_mode = 0;
	SystemConfiguration.pc_com_mode = PC_COM_MODE_USB;
	SystemConfiguration.vehicle_com_mode = NONE;
	VPWFilterEnable = true;
	udi_cdc_flush_rx_buffer();
	SendStatusReport(RESET_DEVICE);
}


 //Create a filter for the CAN system.
 void CreateCanFilter(Message_t *message)
 {
	uint16_t FilterLength; 
	//Length of the filter
	FilterLength = message->buf[1] << 8 | message->buf[2];	
	//Creates the Filter
	CreateFilter(message->buf[3],message->buf+5,message->buf+FilterLength+5,message->buf+FilterLength+FilterLength+5,FilterLength);
	//Sets up a receiver mailbox (see CAN section of datasheet)
	InitalizeReceiverMailbox(message->buf[1],message->buf+5,message->buf+FilterLength+5);
	//Notify user we did it
	char tmpRtn[] = {START_BYTE,0x00,0x03,CREATE_CAN_FILTER,0x01,rx_mailbox_num-1};
	Message_t CanRxSettingsAck;
	CanRxSettingsAck.buf = tmpRtn;
	CanRxSettingsAck.Size = 6;
	WriteMessage(&CanRxSettingsAck);
 }
 
			
//Writes a message out to the user depending on the mode the interface is in (Bluetooth or USB)
void WriteMessage(Message_t *OutgoingMessage)
{
	Error_T InvalidLengthByteError;
	switch(SystemConfiguration.pc_com_mode)
	{
		case PC_COM_MODE_USB:
			ui_com_tx_start();
			udi_cdc_write_buf(OutgoingMessage->buf, OutgoingMessage->Size);
			ui_com_tx_stop();	
		break;
		
		case PC_COM_MODE_BLUETOOTH:
			//usart_serial_write_packet(BOARD_USART, OutgoingMessage->buf, OutgoingMessage->Size);
		break;
		
#ifdef SYSTEM_DEBUG
		case PC_COM_MODE_INTERNAL_DUAL:
		
		break;
#endif

		default:
			//Thats awkward. . .I have no way to talk to the world! :'(

			InvalidLengthByteError.ThrowerID = THROWER_ID_COMMAND_RESPONSE_SYSTEM;
			InvalidLengthByteError.ErrorMajor = UNREACHABLE_STATE_REACHED_EXCPETION;
			InvalidLengthByteError.ErrorMinor = SystemConfiguration.pc_com_mode;
			ThrowError(&InvalidLengthByteError);
		break;
		
	}
	
}

//Writes a message to the vehicle depdning on what mode the interface is in
void WriteVehicleMessage(Message_t *OutgoingMessage)
{
	//Can_Message_t CanTXMessage;
	switch(SystemConfiguration.vehicle_com_mode)
	{
		case VPW_MODE:
			//Disable interrupts here do we dont accidently screw up the transmission
			cpu_irq_disable();
			VPWSendNetworkMessage(OutgoingMessage->buf + 1,	OutgoingMessage->Size-1);
			//Re-enable interrupts
			cpu_irq_enable();
		break;
		
		case CAN_MODE:
		 HandleSendCanRequest(OutgoingMessage);
		break;
	}
}

//Send a status report. The response byte is used to tell the user what we are responding to as 
//multiple commands will return a status report. 
void SendStatusReport(uint8_t ResponseByte)
{
	uint8_t StatusReportBuf[] = {START_BYTE, 0x00, (STATUS_MESSAGE_LENGTH-MESSAGE_BYTES_TO_LENGTH_LSB), ResponseByte, SystemConfiguration.vehicle_com_mode, SystemConfiguration.in_secure_mode, SystemConfiguration.pc_com_mode, SystemConfiguration.bluetooth_unlocked};
	Message_t StatusReportMessage;
	
	StatusReportMessage.buf = StatusReportBuf;
	StatusReportMessage.Size = STATUS_MESSAGE_LENGTH;
	
	WriteMessage(&StatusReportMessage);
}
//Sends the version
void SendVersionReport()
{
	uint8_t VersionReportBuf[] = {START_BYTE, 0x00,0x7,VERSION_REQUEST, ENGINEERING_FIRMWARE, MAJOR, MINOR, DATE_MM, DATE_DD, DATE_YY};
	Message_t VersionReport;
	VersionReport.buf = VersionReportBuf;
	VersionReport.Size = 10;
	WriteMessage(&VersionReport);
}
//Reads the flash ID as per the datasheet/ASF documentation and then sends it out to the user
void SendIdenifierReport()
{
	uint32_t* ID = GetUniqueID();
	if(ID == NULL)
	{
		Error_T GetIDFailedError;
		GetIDFailedError.ThrowerID = READ_UNIQUE_ID;
		GetIDFailedError.ErrorMajor = FLASH_ID_READ_FAILED;
		GetIDFailedError.ErrorMinor = ERROR_NO_MINOR_CODE;
		ThrowError(&GetIDFailedError); 
		return; 
	}
	
	uint8_t tmpRtn[20] = {0x02, 0x00, 0x14, READ_UNIQUE_ID};
	memcpy(tmpRtn+4,ID,20);
	Message_t IDMessage;
	IDMessage.buf = tmpRtn;
	IDMessage.Size = 20;
	WriteMessage(&IDMessage);
}

//Changes the communication mode
void SetCommunicationMode(Message_t *message)
{
	Error_T InvalidLengthByteError;	
	switch(message->buf[2])
	{
		case PC_COM_MODE_USB:
			SystemConfiguration.pc_com_mode = PC_COM_MODE_USB;
		break;
		
		case PC_COM_MODE_BLUETOOTH:
			SystemConfiguration.pc_com_mode = PC_COM_MODE_BLUETOOTH;		
		break;
		
#ifdef SYSTEM_DEBUG
		case PC_COM_MODE_INTERNAL_DUAL:
			SystemConfiguration.pc_com_mode = PC_COM_MODE_INTERNAL_DUAL;		
		break;
#endif

		default:
		//Thats awkward. . .I would have no way to talk to the world! :'(
			InvalidLengthByteError.ThrowerID = SET_TX_MODE;
			InvalidLengthByteError.ErrorMajor = INVALID_PC_COM_MODE_EXCEPTION;
			InvalidLengthByteError.ErrorMinor = SystemConfiguration.pc_com_mode;
			ThrowError(&InvalidLengthByteError);
		break;
	}
	SendStatusReport(SET_TX_MODE);
}
//Changes the interface mode (Vehicle bus)
void SetInterfaceMode(Message_t *message)
{
	Error_T InvalidInterfaceModeError;	
	switch(message->buf[1])
	{
		case VPW_MODE:
			//shutdownCAN();
			VPWEnable();
			ui_vehicle_enable_vpw();
			ui_vehicle_disable_can();
			SystemConfiguration.vehicle_com_mode = VPW_MODE;
			WriteLine("System Mode...VPW");
		break;

		case CAN_MODE:
			SystemConfiguration.vehicle_com_mode = CAN_MODE;
			//TODO: ERROR HANLDING ON BAUD
			InitalizeCanSystem(message->buf[2], sysclk_get_cpu_hz());
			ui_vehicle_enable_can();
			ui_vehicle_disable_vpw();
			WriteLine("System Mode...HSCAN");
	break;
		
		case NONE:
			VPWDisable();
			ui_vehicle_disable_can();
			SystemConfiguration.vehicle_com_mode = NONE;
			WriteLine("System Mode...NONE");
		break;

		default:
			InvalidInterfaceModeError.ThrowerID = SET_INTERFACE_MODE;
			InvalidInterfaceModeError.ErrorMajor = INVALID_INTERFACE_MODE_EXCEPTION;
			InvalidInterfaceModeError.ErrorMinor = SystemConfiguration.pc_com_mode;
			ThrowError(&InvalidInterfaceModeError);
		break;
	}
	
	SendStatusReport(SET_INTERFACE_MODE);
}
//Sets the boot flags to boot from ROM (Bootloader) for firmware update using SAM-BA (See Datasheet/SAM-BA documentation)
void EnterBootloader()
{
	while( ((EFC->EEFC_FSR) & EEFC_FSR_FRDY) == 0)
	{
		//Wait for Flash Ready
	}
	//Boot to bootloader on reset
	uint32_t Fcommand = (0x5A << 24) | 0x010C;
	EFC->EEFC_FCR = Fcommand;
	while( ((EFC->EEFC_FSR) & EEFC_FSR_FRDY) == 0)
	{
		//Wait for Flash Ready
	}
	//Reset!
	RSTC->RSTC_CR = (0xA5 << 24) | RSTC_CR_PERRST | RSTC_CR_PROCRST;
}