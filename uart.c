/*
 * uart.c
 *
 *  Created on: 2015-6-11
 *      Author: wangmg
 */

#include "globa.h"

#define     CLOCK	    20000000
#define     BAUDRATE	115200
#define     RXSIZE      90

char UartRXBuffer[100];
char UartTXBuffer[100];

unsigned char RxBuf[26];

unsigned char UartRXIndex=0;
unsigned char LinkMode=0;
unsigned char LinkGroup=0;
unsigned int ReadLinkDatIndex=0;


void UartInit(void)
{

	  P1DIR &= ~BIT5;                           // Set P1.5 as RX input
	  P1DIR |= BIT6;                            // Set P1.6 as TX output
	  P1SEL |= BIT5 + BIT6;                     // Select P1.5 & P1.6 to UART function
	  UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
	  UCA0CTL1 |= UCSSEL_2;                     // SMCLK

	  double n = 1.0 * CLOCK / BAUDRATE;
	  UCA0BRW = (unsigned int) n;
	  unsigned char ucbrs = (unsigned char) ((n - UCA0BRW) * 8 + 0.5);
	  UCA0MCTL |= ucbrs << 1;

	  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	  UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
}

void uart_enable_interrupt(void)
{
    UCA0IE |= UCRXIE;                         	// Enable USCI_A0 RX interrupt
}

void uart_disable_interrupt(void)
{
	UCA0IE &= ~UCRXIE;                          //Disable USCI_A0 RX interrupt
}

void uart_transmit_data(unsigned char data)     //send unsigned char
{
	while (!(UCA0IFG&UCTXIFG));             	// USCI_A0 TX buffer ready?
    UCA0TXBUF = data;
}

void UartTXMsg(char *data, unsigned char length)//send string
{
    unsigned char i=0;
    for (i=0; i<length; i++)
    {
        uart_transmit_data(data[i]);
    }

}

unsigned char uart_receive_data()              //receive uart data
{
    return UCA0RXBUF;
}

void SendFlashErrorReport(void)
{
	UartTXBuffer[0]='{';
	UartTXBuffer[1]='F';
	UartTXBuffer[2]='l';
	UartTXBuffer[3]='a';
	UartTXBuffer[4]='s';
	UartTXBuffer[5]='h';
	UartTXBuffer[6]=' ';
	UartTXBuffer[7]='E';
	UartTXBuffer[8]='r';
	UartTXBuffer[9]='r';
	UartTXBuffer[10]='o';
	UartTXBuffer[11]='r';
	UartTXBuffer[12]='}';
	UartTXMsg(UartTXBuffer,13);
}

unsigned char CharToByte(char h,char l)       //two char combine to one byte
{
	char hnible;
	char lnible;
	unsigned char res;
	hnible = h;
	lnible = l;

	if(hnible > 'F')                    //wrong
		hnible -= ('a' - 'A');
	if(hnible>'9')
		hnible -= ('A' - 10);
	else
		hnible -= '0';

	if(lnible > 'F')
		lnible -= ('a' - 'A');
	if(lnible > 'F')
		lnible -= ('a' - 'A');
	if(lnible>'9')
		lnible -= ('A' - 10);
	else
		lnible -= '0';

	hnible <<= 4;
	res = hnible + lnible;

	return res;
}

void ByteToChar(unsigned char sr,char* buf)     //one byte split to two char
{
	char hnible;
	char lnible;
	hnible=(sr & 0xf0)>>4;
	lnible=sr & 0x0f;
	if(hnible<0x0a)
	{
		*buf++='0'+hnible;
	}
	else
	{
		*buf++='A'+(hnible-0x0a);
	}

	if(lnible<0x0a)
	{
		*buf='0'+lnible;
	}
	else
	{
		*buf='A'+(lnible-0x0a);
	}

}

// Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
	char rxin;
	switch(__even_in_range(UCA0IV,4))
	{
  		case 0:break;                             	// Vector 0 - no interrupt
  		case 2:                                   	// Vector 2 - RXIFG
  			rxin = UCA0RXBUF;
  			switch (rxin)
  			{
				case ('['): 						// Command package first char '['
					UartRXIndex = 0;
					UartRXBuffer[UartRXIndex] = rxin;
					bUartCmdDone=0;
					break;
				case (']'): 						// Command package last char ']'
					if (UartRXIndex < RXSIZE)
					{
						UartRXIndex++;
						UartRXBuffer[UartRXIndex] = rxin;
						bUartCmdDone=1;				    // Set receive opcode done flag
				//		__bic_SR_register_on_exit(LPM0_bits);
					}
					break;
				default:
					if (UartRXIndex < RXSIZE)
					{
						UartRXIndex++;
						UartRXBuffer[UartRXIndex] = rxin;
					}
					break;
  			}
  			break;
  		case 4:break;                             	// Vector 4 - TXIFG
  		default: break;
	}
}

void UartSendAck(unsigned int command,char status)
{
	UartTXBuffer[0]='(';
	UartTXBuffer[1]='u';
	ByteToChar((command+1)/256,&UartTXBuffer[2]);
	ByteToChar((command+1)%256,&UartTXBuffer[4]);
	UartTXBuffer[6]=status;
	UartTXBuffer[7]=')';
	UartTXMsg(UartTXBuffer,8);
}

void UartResponse(void)
{
	UartTXBuffer[0]='(';
	UartTXBuffer[1]='0';
	UartTXBuffer[2]='0';
	UartTXBuffer[3]=')';
	UartTXMsg(UartTXBuffer,4);          //response
}

void ConvertRecvBufCharToByte(char x)
{
	unsigned char i=0;
	switch(x)
	{
		case 'u':
			RxBuf[0]=CharToByte(UartRXBuffer[2],UartRXBuffer[3]);                   //command high byte
			RxBuf[1]=CharToByte(UartRXBuffer[4],UartRXBuffer[5]);                   //command low byte
			RxBuf[2]=CharToByte(UartRXBuffer[6],UartRXBuffer[7])/2;                 //payload length
			RxBuf[3]=CharToByte(UartRXBuffer[8],UartRXBuffer[9]);                   //addr high byte
			RxBuf[4]=CharToByte(UartRXBuffer[10],UartRXBuffer[11]);                 //addr low byte
			for(i=0;i<RxBuf[2]+1;i++)
			{
				RxBuf[i+5]=CharToByte(UartRXBuffer[12+i*2],UartRXBuffer[13+i*2]);   //payload+checksum
			}
			break;
		case 'v':
			RfTxBuffer[0] = config.MyId[0];                                         //source id
			RfTxBuffer[1] = config.MyId[1];
			RfTxBuffer[2] = config.MyId[2];
			RfTxBuffer[3] = config.MyId[3];

			RfTxBuffer[4]=CharToByte(UartRXBuffer[2],UartRXBuffer[3]);                   //target id[0]
			RfTxBuffer[5]=CharToByte(UartRXBuffer[4],UartRXBuffer[5]);                   //target id[1]
			RfTxBuffer[6]=CharToByte(UartRXBuffer[6],UartRXBuffer[7]);                   //target id[2]
			RfTxBuffer[7]=CharToByte(UartRXBuffer[8],UartRXBuffer[9]);                   //target id[3]

			RfTxBuffer[8]=Direct;

			RfTxBuffer[9]=CharToByte(UartRXBuffer[10],UartRXBuffer[11]);                 //command high byte
			RfTxBuffer[10]=CharToByte(UartRXBuffer[12],UartRXBuffer[13]);                 //command low byte

			RfTxBuffer[11]=CharToByte(UartRXBuffer[14],UartRXBuffer[15])/2;               //payload length          maximum 13
			RfTxBuffer[12]=CharToByte(UartRXBuffer[16],UartRXBuffer[17]);                 //addr high byte
			RfTxBuffer[13]=CharToByte(UartRXBuffer[18],UartRXBuffer[19]);                 //addr low byte

			for(i=0;i<RfTxBuffer[11];i++)
			{
				RfTxBuffer[i+14]=CharToByte(UartRXBuffer[20+i*2],UartRXBuffer[21+i*2]);   //payload+checksum
			}
			RfTxBuffer[27]=GetBufferCheckSum(RfTxBuffer,27);
			Transmit(RfTxBuffer, 28);
			break;
		default:break;
	}
}

char WriteUpdateProgramFlag(void)
{
	unsigned char Flag[2];
	Flag[0]=0xAA;
	Flag[1]=0x55;
	return WriteNBytesWithCheck(0xfffe,Flag,2);
}

//uart received message handle
void UartMessageHandle(void)
{
	unsigned int command=0;
	uart_disable_interrupt();
	UartResponse();
	switch(UartRXBuffer[1])
	{
        /******/                                 //for test
		case 'w':
			UartTXBuffer[0]='(';
			UartTXBuffer[1]=config.MyId[0];
			UartTXBuffer[2]=config.MyId[1];
			UartTXBuffer[3]=config.MyId[2];
			UartTXBuffer[4]=config.MyId[3];
			UartTXBuffer[5]=config.category[0];
			UartTXBuffer[6]=config.category[1];
			UartTXBuffer[7]=config.version[0];
			UartTXBuffer[8]=config.version[1];
			UartTXBuffer[9]=')';
			UartTXMsg(UartTXBuffer,10);
			break;
		/******/
		case 'u':                                //isp updata program
			ConvertRecvBufCharToByte(UartRXBuffer[1]);
			command=(unsigned int)RxBuf[0]*256+RxBuf[1];
			switch(command)
			{
				case START_SEND_CODE:                      //start program download,erase app code flash
					//Uart Packet          [command(2 bytes)]
					EraseSecondHalfChip();                      //earse program memory
					UartSendAck(command,'1');
					break;
				case SEND_CODE_CONTENT:                    //data write to flash
					//Uart Packet          [command(2 bytes)+lenth(1 byte payload lenth)+flash addr(2 bytes)+payload(maximum 16 bytes)]
					UartSendAck(command,WriteNBytesWithCheck((unsigned int)RxBuf[3]*256+RxBuf[4],&RxBuf[5],RxBuf[2]));
					break;
				case FINISH_SEND_CODE:                     //finish program download
					//Uart Packet          [command(2 bytes)]
					if(WriteUpdateProgramFlag())
					{
						UartSendAck(command,'1');
						WDTCTL = WDTPW | WDTIS2_L;	                // start watchdog timer
						while(1);
					}
					else
					{
						UartSendAck(command,'0');
					}
					break;
				default:break;
			}
			break;
		case 'v':
			ConvertRecvBufCharToByte(UartRXBuffer[1]);
			break;
		case 'a':                                //get RF module information
			UartTXBuffer[0]='(';
			UartTXBuffer[1]='a';
			ByteToChar(config.MyId[0],&UartTXBuffer[2]);
			ByteToChar(config.MyId[1],&UartTXBuffer[4]);
			ByteToChar(config.MyId[2],&UartTXBuffer[6]);
			ByteToChar(config.MyId[3],&UartTXBuffer[8]);
			ByteToChar(config.version[0],&UartTXBuffer[10]);
			ByteToChar(config.version[1],&UartTXBuffer[12]);
			ByteToChar(config.category[0],&UartTXBuffer[14]);
			ByteToChar(config.category[1],&UartTXBuffer[16]);
			UartTXBuffer[18]=')';
			UartTXMsg(UartTXBuffer,19);
			break;
		case 'b':                               //get RF module configuration,reserved
			ReadNBytes(CONFIGURATION_ADDR,(unsigned char*)&ReadLink,8);
			UartTXBuffer[0]='(';
			UartTXBuffer[1]='b';
			ByteToChar(ReadLink.FlagByte,&UartTXBuffer[2]);
			ByteToChar(ReadLink.GroupByte,&UartTXBuffer[4]);
			ByteToChar(ReadLink.ID_HH,&UartTXBuffer[6]);
			ByteToChar(ReadLink.ID_MH,&UartTXBuffer[8]);
			ByteToChar(ReadLink.ID_ML,&UartTXBuffer[10]);
			ByteToChar(ReadLink.ID_LL,&UartTXBuffer[12]);
			ByteToChar(ReadLink.Dat1,&UartTXBuffer[14]);
			ByteToChar(ReadLink.Dat2,&UartTXBuffer[16]);
			UartTXBuffer[18]=')';
			UartTXMsg(UartTXBuffer,19);
			break;
		case 'j':                               //set RF configuration
			WriteLink.FlagByte=CharToByte(UartRXBuffer[2],UartRXBuffer[3]);
			WriteLink.GroupByte=CharToByte(UartRXBuffer[4],UartRXBuffer[5]);
			WriteLink.ID_HH=CharToByte(UartRXBuffer[6],UartRXBuffer[7]);
			WriteLink.ID_MH=CharToByte(UartRXBuffer[8],UartRXBuffer[9]);
			WriteLink.ID_ML=CharToByte(UartRXBuffer[10],UartRXBuffer[11]);
			WriteLink.ID_LL=CharToByte(UartRXBuffer[12],UartRXBuffer[13]);
			WriteLink.Dat1=CharToByte(UartRXBuffer[14],UartRXBuffer[15]);
			WriteLink.Dat2=CharToByte(UartRXBuffer[16],UartRXBuffer[17]);

			UartTXBuffer[0]='(';
			UartTXBuffer[1]='j';
			UartTXBuffer[2]=WriteNBytesWithCheck(CONFIGURATION_ADDR,(unsigned char*)&WriteLink,8);
			UartTXBuffer[3]=')';
			UartTXMsg(UartTXBuffer,4);
			break;
		case 'c':                              //get first ALL-Link data
			ReadLinkDatIndex=0;
			ReadNBytes(ReadLinkDatIndex*8,(unsigned char*)&ReadLink,8);
			if((ReadLink.FlagByte&0xf0) == 0xA0)              //has all-link data
			{
				if((ReadLink.FlagByte&ACTIVE)==ACTIVE)
				{
					UartTXBuffer[0]='(';
					UartTXBuffer[1]='c';
					ByteToChar(ReadLink.FlagByte,&UartTXBuffer[2]);
					ByteToChar(ReadLink.GroupByte,&UartTXBuffer[4]);
					ByteToChar(ReadLink.ID_HH,&UartTXBuffer[6]);
					ByteToChar(ReadLink.ID_MH,&UartTXBuffer[8]);
					ByteToChar(ReadLink.ID_ML,&UartTXBuffer[10]);
					ByteToChar(ReadLink.ID_LL,&UartTXBuffer[12]);
					ByteToChar(ReadLink.Dat1,&UartTXBuffer[14]);
					ByteToChar(ReadLink.Dat2,&UartTXBuffer[16]);
					UartTXBuffer[18]=')';
					UartTXMsg(UartTXBuffer,19);                 //response
				}
			}
			else
			{
				UartTXBuffer[0]='(';
				UartTXBuffer[1]='c';
				UartTXBuffer[2]='0';
				UartTXBuffer[3]=')';
				UartTXMsg(UartTXBuffer,4);
			}
			break;
		case 'd':                             //get next ALL-Link data
			ReadNBytes((++ReadLinkDatIndex)*8,(unsigned char*)&ReadLink,8);
			if((ReadLink.FlagByte&0xf0) == 0xA0)              //has all-link data
			{
				if((ReadLink.FlagByte&ACTIVE)==ACTIVE)
				{
					UartTXBuffer[0]='(';
					UartTXBuffer[1]='d';
					ByteToChar(ReadLink.FlagByte,&UartTXBuffer[2]);
					ByteToChar(ReadLink.GroupByte,&UartTXBuffer[4]);
					ByteToChar(ReadLink.ID_HH,&UartTXBuffer[6]);
					ByteToChar(ReadLink.ID_MH,&UartTXBuffer[8]);
					ByteToChar(ReadLink.ID_ML,&UartTXBuffer[10]);
					ByteToChar(ReadLink.ID_LL,&UartTXBuffer[12]);
					ByteToChar(ReadLink.Dat1,&UartTXBuffer[14]);
					ByteToChar(ReadLink.Dat2,&UartTXBuffer[16]);
					UartTXBuffer[18]=')';
					UartTXMsg(UartTXBuffer,19);          //response
				}
			}
			else
			{
				UartTXBuffer[0]='(';
				UartTXBuffer[1]='d';
				UartTXBuffer[2]='0';
				UartTXBuffer[3]=')';
				UartTXMsg(UartTXBuffer,4);
			}
			break;
		case 't':                          //transmit RF message out by radio
			//51 char [t+8id+2flag+4command+34char append data(17byte)]
			switch(CharToByte(UartRXBuffer[10],UartRXBuffer[11]))
			{
				case Direct:
					TransmitUartMsgByRf();
					bDirectRetry=1;
					DirectRetry=0;
					DirectRetryCount=0;
					break;
				case BroadCast:
					break;
				case AllLinkBroadcast:
					SendALLLinkGroupCommand(0);
					break;
				case AllLinkCleanUp:
					break;
				default:break;
			}
			break;
		case 'f':                        //link start
			LinkMode=CharToByte(UartRXBuffer[2],UartRXBuffer[3]);        //link mode
			LinkGroup=CharToByte(UartRXBuffer[4],UartRXBuffer[5]);       //link group
			BroadcastTransmitRfLinkMsg((_LINKMODE)LinkMode,0);           //link start
			bStartLink=1;                             //set the link flag
			LinkState=HAS_SEND_BROADCAST;
			LinkTimeCount=0;                          //used for link timeout
			break;
		case 'g':                      //link cancel
			if(bStartLink==1)
			{
				bStartLink=0;                          //exit linking state
				LinkState=DID_NOT_START;
				LinkTimeCount=0;
			}
			UartTXBuffer[0]='(';
			UartTXBuffer[1]='g';
			UartTXBuffer[2]='1';
			UartTXBuffer[3]=')';
			UartTXMsg(UartTXBuffer,4);          //response
			break;
		case 'h':                      //reset RF module
			ResetEEpromAllLinkData();
			UartTXBuffer[0]='(';
			UartTXBuffer[1]='h';
			UartTXBuffer[2]='1';
			UartTXBuffer[3]=')';
			UartTXMsg(UartTXBuffer,4);          //response
			break;
		case 'k':                 //Set Host Device Category
			configTemp.category[0]=CharToByte(UartRXBuffer[2],UartRXBuffer[3]);
			configTemp.category[1]=CharToByte(UartRXBuffer[4],UartRXBuffer[5]);
		//	configTemp.version[0]=CharToByte(UartRXBuffer[6],UartRXBuffer[7]);
		//	configTemp.version[1]=CharToByte(UartRXBuffer[8],UartRXBuffer[9]);
			if(config.category[0]!=configTemp.category[0]|| config.category[1]!=configTemp.category[1])
			{
				config.category[0]=configTemp.category[0];
				config.category[1]=configTemp.category[1];
				UartTXBuffer[0]='(';
				UartTXBuffer[1]='k';
				UartTXBuffer[2]=WriteNBytesWithCheck(ID_ADDR,(unsigned char*)&config,8);
				UartTXBuffer[3]=')';
				UartTXMsg(UartTXBuffer,4);
			}
			else
			{
				UartTXBuffer[0]='(';
				UartTXBuffer[1]='k';
				UartTXBuffer[2]='1';
				UartTXBuffer[3]=')';
				UartTXMsg(UartTXBuffer,4);
			}
			break;
		case 'l':                      //find first record which match the group number
			LinkTemp.GroupByte=CharToByte(UartRXBuffer[2],UartRXBuffer[3]);
			for(ReadLinkDatIndex=0;ReadLinkDatIndex<ALL_LINK_SUM;ReadLinkDatIndex++)
			{
				ReadNBytes(ReadLinkDatIndex*8,(unsigned char*)&ReadLink,8);
				if((ReadLink.FlagByte & 0xf0) == 0xA0)             //
				{
					if(ReadLink.GroupByte==LinkTemp.GroupByte && ReadLink.FlagByte&ACTIVE==ACTIVE)  //match
					{
						UartTXBuffer[0]='(';
						UartTXBuffer[1]='l';
						ByteToChar(ReadLink.FlagByte,&UartTXBuffer[2]);
						ByteToChar(ReadLink.GroupByte,&UartTXBuffer[4]);
						ByteToChar(ReadLink.ID_HH,&UartTXBuffer[6]);
						ByteToChar(ReadLink.ID_MH,&UartTXBuffer[8]);
						ByteToChar(ReadLink.ID_ML,&UartTXBuffer[10]);
						ByteToChar(ReadLink.ID_LL,&UartTXBuffer[12]);
						ByteToChar(ReadLink.Dat1,&UartTXBuffer[14]);
						ByteToChar(ReadLink.Dat2,&UartTXBuffer[16]);
						UartTXBuffer[18]=')';
						UartTXMsg(UartTXBuffer,19);          //response
						break;
					}
				}
			}
			if(ReadLinkDatIndex==ALL_LINK_SUM)
			{
				UartTXBuffer[0]='(';
				UartTXBuffer[1]='l';
				UartTXBuffer[2]='0';
				UartTXBuffer[3]=')';
				UartTXMsg(UartTXBuffer,4);
			}
			break;
		case 'm':                                //find next
			LinkTemp.GroupByte=CharToByte(UartRXBuffer[2],UartRXBuffer[3]);
			for(ReadLinkDatIndex=ReadLinkDatIndex+1;ReadLinkDatIndex<ALL_LINK_SUM;ReadLinkDatIndex++)
			{
				ReadNBytes(ReadLinkDatIndex*8,(unsigned char*)&ReadLink,8);
				if((ReadLink.FlagByte & 0xf0) == 0xA0)             //
				{
					if(ReadLink.GroupByte==LinkTemp.GroupByte && ReadLink.FlagByte&ACTIVE==ACTIVE)  //match
					{
						UartTXBuffer[0]='(';
						UartTXBuffer[1]='m';
						ByteToChar(ReadLink.FlagByte,&UartTXBuffer[2]);
						ByteToChar(ReadLink.GroupByte,&UartTXBuffer[4]);
						ByteToChar(ReadLink.ID_HH,&UartTXBuffer[6]);
						ByteToChar(ReadLink.ID_MH,&UartTXBuffer[8]);
						ByteToChar(ReadLink.ID_ML,&UartTXBuffer[10]);
						ByteToChar(ReadLink.ID_LL,&UartTXBuffer[12]);
						ByteToChar(ReadLink.Dat1,&UartTXBuffer[14]);
						ByteToChar(ReadLink.Dat2,&UartTXBuffer[16]);
						UartTXBuffer[18]=')';
						UartTXMsg(UartTXBuffer,19);          //response
						break;
					}
				}
			}
			if(ReadLinkDatIndex==ALL_LINK_SUM)
			{
				UartTXBuffer[0]='(';
				UartTXBuffer[1]='m';
				UartTXBuffer[2]='0';
				UartTXBuffer[3]=')';
				UartTXMsg(UartTXBuffer,4);
			}
			break;
		case 'q':                   //delete first
			LinkTemp.GroupByte=CharToByte(UartRXBuffer[2],UartRXBuffer[3]);
			LinkTemp.ID_HH=CharToByte(UartRXBuffer[4],UartRXBuffer[5]);
			LinkTemp.ID_MH=CharToByte(UartRXBuffer[6],UartRXBuffer[7]);
			LinkTemp.ID_ML=CharToByte(UartRXBuffer[8],UartRXBuffer[9]);
			LinkTemp.ID_LL=CharToByte(UartRXBuffer[10],UartRXBuffer[11]);
		//  LinkTemp.Dat1=CharToByte(UartRXBuffer[14],UartRXBuffer[15]);
		//	LinkTemp.Dat2=CharToByte(UartRXBuffer[16],UartRXBuffer[17]);
			for(ReadLinkDatIndex=0;ReadLinkDatIndex<ALL_LINK_SUM;ReadLinkDatIndex++)
			{
				ReadNBytes(ReadLinkDatIndex*8,(unsigned char*)&ReadLink,8);
				if((ReadLink.FlagByte & 0xf0) == 0xA0)             //not exist
				{

					if(ReadLink.GroupByte==LinkTemp.GroupByte && ReadLink.ID_HH==LinkTemp.ID_HH && ReadLink.ID_MH==LinkTemp.ID_MH && ReadLink.ID_ML==LinkTemp.ID_ML && ReadLink.ID_LL==LinkTemp.ID_LL)  //match
					{
						ReadLink.FlagByte &=~ACTIVE;
						UartTXBuffer[0]='(';
						UartTXBuffer[1]='q';
						UartTXBuffer[2]=WriteNBytesWithCheck(ReadLinkDatIndex*8,(unsigned char*)&ReadLink,8);
						UartTXBuffer[3]=')';
						UartTXMsg(UartTXBuffer,4);          //response
						break;
					}
				}
			}
			if(ReadLinkDatIndex==ALL_LINK_SUM)
			{
				UartTXBuffer[0]='(';
				UartTXBuffer[1]='q';
				UartTXBuffer[2]='0';
				UartTXBuffer[3]=')';
				UartTXMsg(UartTXBuffer,4);
			}
			break;
		case 'r':                                          //delete record by device id
			LinkTemp.ID_HH=CharToByte(UartRXBuffer[2],UartRXBuffer[3]);
			LinkTemp.ID_MH=CharToByte(UartRXBuffer[4],UartRXBuffer[5]);
			LinkTemp.ID_ML=CharToByte(UartRXBuffer[6],UartRXBuffer[7]);
			LinkTemp.ID_LL=CharToByte(UartRXBuffer[8],UartRXBuffer[9]);
			for(ReadLinkDatIndex=0;ReadLinkDatIndex<ALL_LINK_SUM;ReadLinkDatIndex++)
			{
				ReadNBytes(ReadLinkDatIndex*8,(unsigned char*)&ReadLink,1);
				if((ReadLink.FlagByte & 0xf0) == 0xA0)             //finish
				{

					if(ReadLink.ID_HH==LinkTemp.ID_HH && ReadLink.ID_MH==LinkTemp.ID_MH && ReadLink.ID_ML==LinkTemp.ID_ML && ReadLink.ID_LL==LinkTemp.ID_LL)  //match
					{
						ReadLink.FlagByte &=~ACTIVE;
						WriteNBytesWithCheck(ReadLinkDatIndex*8,(unsigned char*)&ReadLink,8);
					}
				}
				else break;                //seek done

			}
			UartTXBuffer[0]='(';
			UartTXBuffer[1]='r';
			UartTXBuffer[2]='1';
			UartTXBuffer[3]=')';
			UartTXMsg(UartTXBuffer,4);
			break;
	}
	uart_enable_interrupt();
}

