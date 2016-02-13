/*
 * globa.h
 *
 *  Created on: 2015-6-11
 *      Author: wangmg
 */

#ifndef GLOBA_H_
#define GLOBA_H_


#include "cc430f5137.h"
#include <msp430.h>
#include <IN430.h>
#include "5xx_HAL\hal_UCS.h"
#include "5xx_HAL\hal_pmm.h"
#include "5xx_HAL\hal_pmap.h"
#include "5xx_HAL\RF1A.h"
#include "AT25DF521\at25df521.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>

//for test define
#define       UART_TEST      0
#define       RF_T_TEST      0
#define       RF_R_TEST      0


//
#define       RF_TO_UART     0

/***********************************************/
//device sub category
#define      MY_SUB_CATEGORY        0x10
//define my firmware version
#define      MY_HARDWARE_REV        0x01
#define      MY_FIRMWARE_REV        0x01
/***********************************************/





//for link or unlink
#define       LINKING_TIMEOUT     240
#define       UNLINKING_TIMEOUT   240

//transmit and receive timeout and transmit retry times and not receive ack retry times
#define       T_RETRY_TIMES    1
#define       T_TIMEOUT        30


#define       ZONE           1
#define       RFDB_HIGH	     1

#if ZONE == 3
#define  PATABLE_VAL        (0x50)          // 8D 0 dBm output
#define  PATABLE_VAL_10DB   (0xC6)          // 10 dBm output
#else
#define  PATABLE_VAL        (0x8D)          // 8D 0 dBm output
#define  PATABLE_VAL_10DB   (0xC3)          // 10 dBm output
#endif


#define      PKT_LENGTH       28
#define      CRC_LQI_IDX      (PKT_LENGTH+1)
#define  	 RSSI_IDX         (PACKET_LEN)
#define      CRC_OK           BIT7

//device id base
#define       DEVICE_INFO          0xff70


//id structure
typedef struct
{
	unsigned char version[2];                                //version[0]=hardware version version[1]=firmware version
	unsigned char category[2];                               //category[0]=category category[1]=sub category
	unsigned char MyId[4];                                   //MyId[0]=category MyId[1,2,3]=device ID
}_Config;

//device control type
typedef enum
{
	MODE_SLAVE         = 0x00,
	MODE_MASTER        = 0x01,
	MODE_EACH          = 0x03,
	MODE_DELETE        = 0xff
}_LINKMODE;


//configuration base
#define     CONFIGURATION_ADDR     (511*8)

//ID echo
#define     ID_ADDR                (510*8)

//all-link item sum
#define     ALL_LINK_SUM           510


//group id occupy mask
#define       GRP_ID_MASK            0xA0
#define       MASTER                 0x02
#define       SLAVER                 0x00
#define       ACTIVE                 0x01
#define       IN_ACTIVE              0x00

//checksum byte
#define       CHECK_SUM     0x5A
//max hops
#define       MAX_HOPS      0x03


//command
#define            REMOTE_SINGLE_COLOR           0x0001
#define            CMD_RFLINK                    0x0008
#define            CMD_LED_APPOINTMENT           0x0009
#define            SPECIFY_FIX_COLOR             0X0010
#define            CHANGE_ALL_LINK_GROUP         0x0015
#define            GET_DEVICE_INFORMATION        0x0016

//Update Program Commands
#define                  START_SEND_CODE                       0x5000
#define                  START_SEND_CODE_ACK                   0x5001

#define                  SEND_CODE_CONTENT                     0x5002
#define                  SEND_CODE_CONTENT_ACK                 0x5003

#define                  FINISH_SEND_CODE                      0x5004
#define                  FINISH_SEND_CODE_ACK                  0x5005


//message type
typedef enum{
	Direct                  = 0x00,
	DirectAck               = 0x20,
	DirectNack              = 0x30,
	AllLinkCleanUp          = 0x40,
	AllLinkCleanUpAck       = 0x60,
	AllLinkCleanUpNack      = 0x70,
	BroadCast               = 0x80,
	AllLinkBroadcast        = 0xc0
}_MSGTYPE;


//link data
typedef struct{
	unsigned char FlagByte;
	unsigned char GroupByte;
	unsigned char ID_HH;
	unsigned char ID_MH;
	unsigned char ID_ML;
	unsigned char ID_LL;
	unsigned char Dat1;
	unsigned char Dat2;
}_LINK_DAT;


//link and unlink state machine
typedef enum{
	DID_NOT_START          = 0x00,
	HAS_SEND_BROADCAST     = 0x01,
	HAS_SEND_DIRETMSG      = 0x02
}_LINK_STATE;
extern _LINK_STATE LinkState,UnlinkState;


//flag structure
typedef union
{
	struct
	{
		unsigned int Bit0:1;			// Bit 0
		unsigned int Bit1:1;			// Bit 1
		unsigned int Bit2:1;			// Bit 2
		unsigned int Bit3:1;			// Bit 3
		unsigned int Bit4:1;			// Bit 4
		unsigned int Bit5:1;			// Bit 5
		unsigned int Bit6:1;			// Bit 6
		unsigned int Bit7:1;			// Bit 7
		unsigned int Bit8:1;			// Bit 0
		unsigned int Bit9:1;			// Bit 10
		unsigned int Bit10:1;			// Bit 11
		unsigned int Bit11:1;			// Bit 12
		unsigned int Bit12:1;			// Bit 13
		unsigned int Bit13:1;			// Bit 14
		unsigned int Bit14:1;			// Bit 15
		unsigned int Bit15:1;			// Bit 16
	}flag;
	unsigned int all_flags;
}IntFlags;
extern volatile IntFlags ModuleF1;

//flags for buttons which has long press function
#define     bStartLink                   ModuleF1.flag.Bit0                     //link start
#define     bTransmitDone                ModuleF1.flag.Bit1                     //unlink start
#define     bWor                         ModuleF1.flag.Bit2                     //wor mode
#define     bReceivedMsg                 ModuleF1.flag.Bit3                     //has received message
#define     bReceiving                   ModuleF1.flag.Bit4                     //in receiving state
#define     bTransmitting                ModuleF1.flag.Bit5                     //in transmitting state
#define     bUartCmdDone                 ModuleF1.flag.Bit6                     //has received a uart message
#define     bCleanUpRetry                ModuleF1.flag.Bit7
#define     bHasSendAllLinkBroadcast     ModuleF1.flag.Bit8
#define     bDirectRetry                 ModuleF1.flag.Bit9
#define     bLinkRetry                   ModuleF1.flag.Bit10                    //slave
#define     bDeviceAlreadyLink           ModuleF1.flag.Bit11
#define     bDeviceAlreadyUnlink         ModuleF1.flag.Bit12



//from main.c
extern unsigned char RfRxBuffer[64];
extern unsigned char RfTxBuffer[28];



//from timer.c
extern unsigned int LinkTimeCount;
extern unsigned int AllLinkCleanUpMessageAckTimeoutCount;


extern unsigned char LinkRetry;
extern unsigned int  LinkRetryCount;
extern unsigned char CleanUpRetry;
extern unsigned int  CleanUpRetryCount;
extern unsigned int  DirectRetryCount;
extern unsigned char DirectRetry;

extern void StartTimer1(void);

//from setup.c
extern _Config config,configTemp;
extern unsigned char RxBufferLength;
extern _LINK_DAT ReadLink,WriteLink,UnLinkDat,LinkTemp;
extern unsigned char GroupNum;

extern void RFMsgHandle(void);
extern void InitRadio(void);
extern void IOInit(void);
extern void RfTestTX(void);
extern void RfTestRX(void);
extern void TransmitUartMsgByRf(void);
extern void ReceiveOn(void);
extern void ReadConfigData(void);
extern void BroadcastTransmitRfLinkMsg(_LINKMODE Mode,unsigned char hops);
extern void SendALLLinkGroupCommand(unsigned char hops);
extern void SendAllLinkCleanUpFailureReport(void);
extern void StartSendAllLinkCleanUpMessage(unsigned char group);
extern void ResetEEpromAllLinkData(void);
extern void Transmit(unsigned char *buffer, unsigned char length);
extern void DirectlyTransmitRfLinkMsg(_MSGTYPE Type,_LINKMODE Mode,unsigned char hops);
extern unsigned char CheckBufferCheckSum(unsigned char *buf,unsigned char len);
extern unsigned char GetBufferCheckSum(unsigned char *buf,unsigned char len);
extern void RfTransmitDoneHandle(void);

//from uart.c

extern char UartRXBuffer[100];
extern char UartTXBuffer[100];
extern unsigned char LinkMode;
extern unsigned char LinkGroup;

extern void uart_enable_interrupt(void);
extern void uart_disable_interrupt(void);
extern void UartInit(void);
extern unsigned char CharToByte(char h,char l);
extern void ByteToChar(unsigned char sr,char* buf);
extern void UartMessageHandle(void);
extern void UartTXMsg(char *data, unsigned char length);
extern void SendFlashErrorReport(void);

#endif /* GLOBA_H_ */
