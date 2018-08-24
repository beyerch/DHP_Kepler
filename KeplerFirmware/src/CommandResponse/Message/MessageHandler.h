/*
 * MessageReceiver.h
 *
 * Created: 2/24/2017 2:05:56 PM
 *  Author: Aaron
 */ 


#ifndef MESSAGEHANDLER_H_
#define MESSAGEHANDLER_H_

#include <asf.h>
#include "Messages.h"
#include "ErrorHandler.h"
#include "KeplerConfiguration.h"
#include "ui.h"
#include "j1850vpw.h"
#include "console.h"
#include "security.h"
#include "string.h"
#include "compiler.h"
#include "adc.h"
#include "CanFilter.h"
#include "kcan.h"

#define STATUS_MESSAGE_LENGTH 7
#define MESSAGE_BYTES_TO_LENGTH_LSB 3

//Message buffer for USB 
static char IncomingMessageBuffer[MESSAGE_BUFFER_SIZE];
//Message buffer for vehicle communication
static unsigned char VehicleMessageBuffer[MESSAGE_BUFFER_SIZE];

extern Message_t IncommingMessage;

extern char MessageAvailable;

void ReceiveUSBMessage(uint8_t port);
void RunCommand(Message_t message);
void HandleMessage(Message_t *message);
void WriteMessage(Message_t *OutgoingMessage);
void SendStatusReport(uint8_t ResponseByte);
void SendVersionReport(void);
void SendIdenifierReport(void);
void WriteVehicleMessage(Message_t *OutgoingMessage);
void SetCommunicationMode(Message_t *message);
void SetInterfaceMode(Message_t *OutgoingMessage);
void CreateCanFilter(Message_t *message);
void EnterBootloader(void);
void ResetDevice(void);



#endif /* MESSAGERECEIVER_H_ */