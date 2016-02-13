/*
 * setup.c
 *
 *  Created on: 2015-6-11
 *      Author: wangmg
 */
#include "globa.h"

_LINK_DAT ReadLink,WriteLink,UnLinkDat,LinkTemp;
_LINK_STATE LinkState;
_Config config,configTemp;
unsigned char LinkCat,LinkSubCat;

/*******************************************/       //for ALL-Link group cleanup command execute
unsigned char GroupNum=0;
volatile unsigned int AllLinkAddr=0;
volatile char CleanUpStatus='1';
/*******************************************/




//#pragma RETAIN(myid1)
//#pragma location=0xff70
//const unsigned long int myid1 = 0x29001010;
//



void IOInit(void)
{

	// Port 1:
	// P1.5 Uart RX Input
	// P1.6 Uart TX Output
	  P1DIR = 0xDB;                            	// Set P1.6 as TX output
	  P1SEL |= BIT5 + BIT6;                     // Select P1.5 & P1.6 to UART function
	  P1OUT |= BIT1;						    // Set SPICS to high P1.1 and P1.5 Pull up resistor



  // Port 2:
	  P2OUT = 0;
	  P2DIR = 0xFF;

  // Port 3:
	  P3OUT = 0x80;
	  P3DIR = 0xFF;

	  // Port 5
	  P5OUT = 0;
	  P5DIR = 0xFF;


	  /* Initialize Port J */
	  PJOUT = 0x00;
	  PJDIR = 0xFF;

}

void InitRadio(void)
{
  /* Set the High-Power Mode Request Enable bit so LPM3 can be entered
     with active radio enabled */
	PMMCTL0_H = 0xA5;
	PMMCTL0_L |= PMMHPMRE_L;
	PMMCTL0_H = 0x00;

#if ZONE == 1
	SetRFUS38K();
#endif

#if	ZONE == 2
	SetRFEU38K();
#endif

#if ZONE == 3
	SetRF433M1200();
#endif

#if RFDB_HIGH == 1
	WriteSinglePATable(PATABLE_VAL_10DB);
#else
	WriteSinglePATable(PATABLE_VAL);          // Write to Power Amplifier Table
#endif
}


void ReceiveOn(void)
{
	bReceiving=1;
	bTransmitting=0;

	RF1AIES |= BIT9;                         	// Falling edge of RFIFG9
	RF1AIFG &= ~BIT9;                           // Clear a pending interrupt
	RF1AIE  |= BIT9;                            // Enable the interrupt
                                                //Radio is in IDLE following a TX, so strobe SRX to enter Receive Mode
	Strobe(RF_SRX);                           // Strobe SRX
}

void ReceiveOff(void)
{
	bReceiving=0;
	bTransmitting=0;
	RF1AIE &= ~BIT9;                          // Disable RX interrupts
	RF1AIFG &= ~BIT9;                         // Clear pending IFG

	//It is possible that ReceiveOff is called while radio is receiving a packet.
	//Therefore, it is necessary to flush the RX FIFO after issuing IDLE strobe
	//such that the RXFIFO is empty prior to receiving a packet.
	Strobe( RF_SIDLE );
	Strobe( RF_SFRX  );
}


void Transmit(unsigned char *buffer, unsigned char length)
{
	while(bTransmitting==1);                  //check if data transmit done
	Strobe(RF_SIDLE);                         //Strobe IDLE
	Strobe(RF_SFTX);
	bReceiving=0;
	bTransmitting=1;
	RF1AIFG &= ~BIT9;                                           // Clear pending interrupts
	RF1AIE |= BIT9;                                             // Enable TX end-of-packet interrupt
	WriteBurstReg(RF_TXFIFOWR, buffer, length);
	Strobe( RF_STX );                                           // Strobe STX
}

//for rf transmit test
void RfTestTX(void)
{
	ReceiveOff();
	while (1)
	{
		Transmit(RfTxBuffer, 28);
		while(bTransmitting)
		{
        __no_operation();
		}
	//	__delay_cycles(20000000);                 //delay 2 seconds
	}
}

//for rf receive test
void RfTestRX(void)
{
	ReceiveOn();
	while (1)
 	{
		if(bReceivedMsg==1)
		{
			bReceivedMsg=0;
			RxBufferLength=ReadSingleReg( RXBYTES );
			ReadBurstReg(RF_RXFIFORD, RfRxBuffer, RxBufferLength);
			if(RfRxBuffer[CRC_LQI_IDX]&CRC_OK)
			{
				P1OUT^=0x01;                  //toggle port
			}
		}
 	}
}


void ReadConfigData(void)
{
	unsigned char i;
	unsigned char *ptrA;
	_Config TempConfig;
	/***********************************************/         //factory parameter
	ptrA=(unsigned char *)DEVICE_INFO;
	for(i=0;i<4;i++)
	{
		(&config.MyId[0])[i]=ptrA[i];
	}
	config.category[0]=config.MyId[0];
	config.category[1]=MY_SUB_CATEGORY;
	config.version[0]=MY_FIRMWARE_REV;
	config.version[1]=MY_HARDWARE_REV;
	/*********************************************/          //read eeprom data
	ReadNBytes(ID_ADDR,(unsigned char*)&TempConfig,8);
	/**********************************************/
	if(TempConfig.MyId[0]==config.MyId[0] && TempConfig.MyId[1]==config.MyId[1] && TempConfig.MyId[2]==config.MyId[2] && TempConfig.MyId[3]==config.MyId[3])        //id right mean has available data in eeprom
	{
		if(TempConfig.category[0]!=config.category[0] || TempConfig.category[1]!=config.category[1])   //has changed
		{
			config.category[0]=TempConfig.category[0];               //use new data
			config.category[1]=TempConfig.category[1];
		}
	}
	else                  //id wrong mean first time program run
	{
		WriteNBytes(ID_ADDR,(unsigned char*)&config,8);                       //save data
		ReadNBytes(ID_ADDR,(unsigned char*)&TempConfig,8);
		if(config.MyId[0]!=TempConfig.MyId[0] || config.MyId[1]!=TempConfig.MyId[1] || config.MyId[2]!=TempConfig.MyId[2] ||config.MyId[3]!=TempConfig.MyId[3]
		|| config.category[0]!=TempConfig.category[0] || config.category[1]!=TempConfig.category[1] || config.version[0]!=TempConfig.version[0] || config.version[1]!=TempConfig.version[1])
		{
			SendFlashErrorReport();
		//	while(1);
		}
	}
}

void ResetEEpromAllLinkData(void)
{
	EraseFirstHalfChip();
}

//calculate data checksum
unsigned char GetBufferCheckSum(unsigned char *buf,unsigned char len)
{
	unsigned char checksum=0;
	unsigned char i=0;
	for(i=0;i<len;i++)
	{
		checksum+=buf[i];
	}
	checksum=CHECK_SUM-checksum;
	return checksum;
}
//check data checksum
unsigned char CheckBufferCheckSum(unsigned char *buf,unsigned char len)
{
	unsigned char checksum=0;
	unsigned char i=0;
	for(i=0;i<len+1;i++)
	{
		checksum+=buf[i];
	}
	if(checksum==CHECK_SUM)return 1;
	return 0;
}
//save ALL-Link data
void SaveAllLinkData(_LINK_DAT linkDat)                  //ALL-Link data save
{
	unsigned int i=0;
	unsigned int addr=0;
	for(i=0;i<ALL_LINK_SUM;i++)
	{
		addr=i*8;
		ReadNBytes(addr,(unsigned char*)&ReadLink,8);
		switch(ReadLink.FlagByte)
		{
			case 0xA0:                                 //has ID,inactive,slave
			case 0xA2:                                 //has ID,inactive,master
				if(ReadLink.ID_HH==linkDat.ID_HH && ReadLink.ID_MH==linkDat.ID_MH && ReadLink.ID_ML==linkDat.ID_ML && ReadLink.ID_LL==linkDat.ID_LL && ReadLink.GroupByte==linkDat.GroupByte)            //all-link already exist,just in inactive state
				{
					ReadLink.FlagByte |= BIT0;
					WriteNBytes(addr,(unsigned char*)&ReadLink,1);
					return;
				}
				break;
			case 0xA1:                                 //has ID,active,slave
			case 0xA3:                                 //has ID,active,master
				if(ReadLink.ID_HH==linkDat.ID_HH && ReadLink.ID_MH==linkDat.ID_MH && ReadLink.ID_ML==linkDat.ID_ML && ReadLink.ID_LL==linkDat.ID_LL && ReadLink.GroupByte==linkDat.GroupByte)            //all-link already exist
				{
					return;
				}
				break;
			default:                                   //empty
				WriteNBytes(addr,(unsigned char*)&linkDat,8);
				return;
		}
	}
}
//unlink ALL-Link data
void UnlinkAllLinkData(_LINK_DAT linkDat)             //ALL-Link data unlink
{
	unsigned int i=0;
	unsigned int addr=0;
	for(i=0;i<ALL_LINK_SUM;i++)
	{
		addr=i*8;
		ReadNBytes(addr,(unsigned char*)&ReadLink,8);
		switch(ReadLink.FlagByte)
		{
			case 0xA0:                                 //has ID,inactive,slave
			case 0xA2:                                 //has ID,inactive,master
				if(ReadLink.ID_HH==linkDat.ID_HH && ReadLink.ID_MH==linkDat.ID_MH && ReadLink.ID_ML==linkDat.ID_ML && ReadLink.ID_LL==linkDat.ID_LL && ReadLink.GroupByte==linkDat.GroupByte)            //all-link already inactive
				{
					return;
				}
				break;
			case 0xA1:                                 //has ID,active,slave
			case 0xA3:                                 //has ID,active,master
				if(ReadLink.ID_HH==linkDat.ID_HH && ReadLink.ID_MH==linkDat.ID_MH && ReadLink.ID_ML==linkDat.ID_ML && ReadLink.ID_LL==linkDat.ID_LL && ReadLink.GroupByte==linkDat.GroupByte)            //all-link already exist
				{
					ReadLink.FlagByte &=~BIT0;                                    //set device inactive
					WriteNBytes(addr,(unsigned char*)&ReadLink,1);                //save ALL-Link data
					return;
				}
				break;
			default:                                   //reach the last ALL-LINK data
				return;
		}
	}
}
//for host add or sub all-link group
unsigned char AddOrSubAllLinkGroup(_LINK_DAT linkDat,unsigned char flag,unsigned char NewGrp)
{
	unsigned int i=0;
	unsigned int addr=0;
	if(flag)                          //add
	{
		for(i=0;i<ALL_LINK_SUM;i++)
		{
			addr=i*8;
			ReadNBytes(addr,(unsigned char*)&ReadLink,8);
			switch(ReadLink.FlagByte)
			{
				case 0xA0:
				case 0xA2:
					if(ReadLink.ID_HH==linkDat.ID_HH && ReadLink.ID_MH==linkDat.ID_MH && ReadLink.ID_ML==linkDat.ID_ML && ReadLink.ID_LL==linkDat.ID_LL && ReadLink.GroupByte==NewGrp) //already have inactive
					{
						ReadLink.FlagByte |=BIT0;                                     //set device active
						WriteNBytes(addr,(unsigned char*)&ReadLink,2);                //save ALL-Link data
						return 0x00;
					}
					break;
				case 0xA1:
				case 0xA3:
					if(ReadLink.ID_HH==linkDat.ID_HH && ReadLink.ID_MH==linkDat.ID_MH && ReadLink.ID_ML==linkDat.ID_ML && ReadLink.ID_LL==linkDat.ID_LL && ReadLink.GroupByte==NewGrp)return 0x00;
					break;
				default:
					linkDat.FlagByte=0xA3;
					linkDat.GroupByte=NewGrp;
					WriteNBytes(addr,(unsigned char*)&linkDat,6);                     //ingore append data
					return 0x00;
			}
		}
	}
	else                           //sub
	{
		for(i=0;i<ALL_LINK_SUM;i++)
		{
			addr=i*8;
			ReadNBytes(addr,(unsigned char*)&ReadLink,8);
			switch(ReadLink.FlagByte)
			{
				case 0xA0:
				case 0xA2:
					if(ReadLink.ID_HH==linkDat.ID_HH && ReadLink.ID_MH==linkDat.ID_MH && ReadLink.ID_ML==linkDat.ID_ML && ReadLink.ID_LL==linkDat.ID_LL && ReadLink.GroupByte==NewGrp)return 0x00; //already have inactive
					break;
				case 0xA1:
				case 0xA3:
					if(ReadLink.ID_HH==linkDat.ID_HH && ReadLink.ID_MH==linkDat.ID_MH && ReadLink.ID_ML==linkDat.ID_ML && ReadLink.ID_LL==linkDat.ID_LL && ReadLink.GroupByte==NewGrp)
					{
						ReadLink.FlagByte &=~BIT0;                                     //set device active
						WriteNBytes(addr,(unsigned char*)&ReadLink,1);                //save ALL-Link data
						return 0x00;
					}
					break;
				default:
					break;
			}
		}
	}
	return 0x01;
}
//check all-link broadcast message ID
unsigned char AllLinkBroadcastMessageCheckID(void)                              //check if the ALL-Link broadcast message is to me,ALL-Link broadcast message almost send from master to slave
{
	unsigned char i=0;
	for(i=0;i<ALL_LINK_SUM;i++)
	{
		ReadNBytes(i*8,(unsigned char*)&ReadLink,8);
		switch(ReadLink.FlagByte)
		{
			case 0xA0:                                 //has ID,inactive,slave
			case 0xA2:                                 //has ID,inactive,master
			case 0xA1:                                 //has ID,active,slave
				break;
			case 0xA3:                                 //has ID,active,master
				if(ReadLink.GroupByte==RfRxBuffer[7] || 0xff==RfRxBuffer[7])                //match group number or group number is 0xff
				{
					if(ReadLink.ID_HH==RfRxBuffer[0] && ReadLink.ID_MH==RfRxBuffer[1] && ReadLink.ID_ML==RfRxBuffer[2] && ReadLink.ID_LL==RfRxBuffer[3])return 1;
				}
				break;
			default:return 0;                            //has checked all exist All-link data
		}
	}
	return 0;
}

//ALL-Link broadcast message
void BroadcastTransmitRfLinkMsg(_LINKMODE Mode,unsigned char hops)
{
	RfTxBuffer[0] = config.MyId[0];                   //my device id
	RfTxBuffer[1] = config.MyId[1];
	RfTxBuffer[2] = config.MyId[2];
	RfTxBuffer[3] = config.MyId[3];

	RfTxBuffer[4] = config.category[0];                      //my device type
	RfTxBuffer[5] = config.category[1];
	RfTxBuffer[6] = config.version[0];                          //firmware version
	RfTxBuffer[7] = config.version[1];

	RfTxBuffer[8] = BroadCast|MAX_HOPS|(hops<<2);     //message flag

	RfTxBuffer[9] = CMD_RFLINK/256;                   //command
	RfTxBuffer[10] = CMD_RFLINK%256;

	RfTxBuffer[11] = Mode;                            //master
	RfTxBuffer[12] = 0;
	RfTxBuffer[13] = 0;
	RfTxBuffer[14] = 0;
	RfTxBuffer[15] = 0;
	RfTxBuffer[16] = 0;
	RfTxBuffer[17] = 0;
	RfTxBuffer[18] = 0;
	RfTxBuffer[19] = 0;
	RfTxBuffer[20] = 0;
	RfTxBuffer[21] = 0;
	RfTxBuffer[22] = 0;
	RfTxBuffer[23] = 0;
	RfTxBuffer[24] = 0;
	RfTxBuffer[25] = 0;
	RfTxBuffer[26] = 0;
	RfTxBuffer[27] = GetBufferCheckSum(RfTxBuffer,27);
	Transmit(RfTxBuffer, 28);
}
//ALL-Link direct message
void DirectlyTransmitRfLinkMsg(_MSGTYPE Type,_LINKMODE Mode,unsigned char hops)
{
	RfTxBuffer[0] = config.MyId[0];                   //source id
	RfTxBuffer[1] = config.MyId[1];
	RfTxBuffer[2] = config.MyId[2];
	RfTxBuffer[3] = config.MyId[3];

	RfTxBuffer[4] = RfRxBuffer[0];
	RfTxBuffer[5] = RfRxBuffer[1];
	RfTxBuffer[6] = RfRxBuffer[2];
	RfTxBuffer[7] = RfRxBuffer[3];

	RfTxBuffer[8] = Type|MAX_HOPS|(hops<<2);         //message flag

	RfTxBuffer[9] = CMD_RFLINK/256;                  //command
	RfTxBuffer[10] = CMD_RFLINK%256;

	RfTxBuffer[11] = Mode;                           //master

	RfTxBuffer[12] = LinkGroup;                      //group
	RfTxBuffer[13] = 0;
	RfTxBuffer[14] = 0;
	RfTxBuffer[15] = 0;
	RfTxBuffer[16] = 0;
	RfTxBuffer[17] = 0;
	RfTxBuffer[18] = 0;
	RfTxBuffer[19] = 0;
	RfTxBuffer[20] = 0;
	RfTxBuffer[21] = 0;
	RfTxBuffer[22] = 0;
	RfTxBuffer[23] = 0;
	RfTxBuffer[24] = 0;
	RfTxBuffer[25] = 0;
	RfTxBuffer[26] = 0;
	RfTxBuffer[27] = GetBufferCheckSum(RfTxBuffer,27);
	Transmit(RfTxBuffer, 28);
}
//transmit uart message
void TransmitUartMsgByRf(void)
{
	RfTxBuffer[0] = config.MyId[0];                                         //source id
	RfTxBuffer[1] = config.MyId[1];
	RfTxBuffer[2] = config.MyId[2];
	RfTxBuffer[3] = config.MyId[3];

	RfTxBuffer[4] = CharToByte(UartRXBuffer[2],UartRXBuffer[3]);            //target id
	RfTxBuffer[5] = CharToByte(UartRXBuffer[4],UartRXBuffer[5]);
	RfTxBuffer[6] = CharToByte(UartRXBuffer[6],UartRXBuffer[7]);
	RfTxBuffer[7] = CharToByte(UartRXBuffer[8],UartRXBuffer[9]);

	RfTxBuffer[8] = CharToByte(UartRXBuffer[10],UartRXBuffer[11]);          //Flag

	RfTxBuffer[9] = CharToByte(UartRXBuffer[12],UartRXBuffer[13]);          //command
	RfTxBuffer[10] = CharToByte(UartRXBuffer[14],UartRXBuffer[15]);

	RfTxBuffer[11] = CharToByte(UartRXBuffer[16],UartRXBuffer[17]);         //data1
	RfTxBuffer[12] = CharToByte(UartRXBuffer[18],UartRXBuffer[19]);         //data2
	RfTxBuffer[13] = CharToByte(UartRXBuffer[20],UartRXBuffer[21]);
	RfTxBuffer[14] = CharToByte(UartRXBuffer[22],UartRXBuffer[23]);
	RfTxBuffer[15] = CharToByte(UartRXBuffer[24],UartRXBuffer[25]);
	RfTxBuffer[16] = CharToByte(UartRXBuffer[26],UartRXBuffer[27]);
	RfTxBuffer[17] = CharToByte(UartRXBuffer[28],UartRXBuffer[29]);
	RfTxBuffer[18] = CharToByte(UartRXBuffer[30],UartRXBuffer[31]);
	RfTxBuffer[19] = CharToByte(UartRXBuffer[32],UartRXBuffer[33]);
	RfTxBuffer[20] = CharToByte(UartRXBuffer[34],UartRXBuffer[35]);
	RfTxBuffer[21] = CharToByte(UartRXBuffer[36],UartRXBuffer[37]);
	RfTxBuffer[22] = CharToByte(UartRXBuffer[38],UartRXBuffer[39]);
	RfTxBuffer[23] = CharToByte(UartRXBuffer[40],UartRXBuffer[41]);
	RfTxBuffer[24] = CharToByte(UartRXBuffer[42],UartRXBuffer[43]);
	RfTxBuffer[25] = CharToByte(UartRXBuffer[44],UartRXBuffer[45]);
	RfTxBuffer[26] = CharToByte(UartRXBuffer[46],UartRXBuffer[47]);
	RfTxBuffer[27] = GetBufferCheckSum(RfTxBuffer,27);
    Transmit(RfTxBuffer, 28);
}
//send all-link group command
void SendALLLinkGroupCommand(unsigned char hops)
{
	RfTxBuffer[0] = config.MyId[0];                                         //source id
	RfTxBuffer[1] = config.MyId[1];
	RfTxBuffer[2] = config.MyId[2];
	RfTxBuffer[3] = config.MyId[3];

	RfTxBuffer[4] = CharToByte(UartRXBuffer[2],UartRXBuffer[3]);            //target id
	RfTxBuffer[5] = CharToByte(UartRXBuffer[4],UartRXBuffer[5]);
	RfTxBuffer[6] = CharToByte(UartRXBuffer[6],UartRXBuffer[7]);
	RfTxBuffer[7] = GroupNum= CharToByte(UartRXBuffer[8],UartRXBuffer[9]);       //group  save group number

	RfTxBuffer[8] = CharToByte(UartRXBuffer[10],UartRXBuffer[11]);//AllLinkBroadcast|MAX_HOPS|(hops<<2);               //flag

	RfTxBuffer[9] = CharToByte(UartRXBuffer[12],UartRXBuffer[13]);       //command
	RfTxBuffer[10] = CharToByte(UartRXBuffer[14],UartRXBuffer[15]);


	RfTxBuffer[11] = CharToByte(UartRXBuffer[16],UartRXBuffer[17]);
	RfTxBuffer[12] = CharToByte(UartRXBuffer[18],UartRXBuffer[19]);
	RfTxBuffer[13] = CharToByte(UartRXBuffer[20],UartRXBuffer[21]);
	RfTxBuffer[14] = CharToByte(UartRXBuffer[22],UartRXBuffer[23]);
	RfTxBuffer[15] = CharToByte(UartRXBuffer[24],UartRXBuffer[25]);
	RfTxBuffer[16] = CharToByte(UartRXBuffer[26],UartRXBuffer[27]);
	RfTxBuffer[17] = CharToByte(UartRXBuffer[28],UartRXBuffer[29]);
	RfTxBuffer[18] = CharToByte(UartRXBuffer[30],UartRXBuffer[31]);
	RfTxBuffer[19] = CharToByte(UartRXBuffer[32],UartRXBuffer[33]);
	RfTxBuffer[20] = CharToByte(UartRXBuffer[34],UartRXBuffer[35]);
	RfTxBuffer[21] = CharToByte(UartRXBuffer[36],UartRXBuffer[37]);
	RfTxBuffer[22] = CharToByte(UartRXBuffer[38],UartRXBuffer[39]);
	RfTxBuffer[23] = CharToByte(UartRXBuffer[40],UartRXBuffer[41]);
	RfTxBuffer[24] = CharToByte(UartRXBuffer[42],UartRXBuffer[43]);
	RfTxBuffer[25] = CharToByte(UartRXBuffer[44],UartRXBuffer[45]);
	RfTxBuffer[26] = CharToByte(UartRXBuffer[46],UartRXBuffer[47]);
	RfTxBuffer[27] = GetBufferCheckSum(RfTxBuffer,27);
	Transmit(RfTxBuffer, 28);
}
//Link completed ACK
void LinkCompletedMessageSendToUart(_LINK_DAT link)
{
	UartTXBuffer[0]='(';

	UartTXBuffer[1]='L';
	UartTXBuffer[2]='F';

	if(link.FlagByte & 0x02)                            //mode
	{
		ByteToChar(MODE_MASTER,&UartTXBuffer[3]);
	}
	else
	{
		ByteToChar(MODE_SLAVE,&UartTXBuffer[3]);
	}
	ByteToChar(link.GroupByte,&UartTXBuffer[5]);        //group

	ByteToChar(link.ID_HH,&UartTXBuffer[7]);            //ID
	ByteToChar(link.ID_MH,&UartTXBuffer[9]);
	ByteToChar(link.ID_ML,&UartTXBuffer[11]);
	ByteToChar(link.ID_LL,&UartTXBuffer[13]);

	ByteToChar(LinkCat,&UartTXBuffer[15]);             //cat
	ByteToChar(LinkSubCat,&UartTXBuffer[17]);          //subcat

	UartTXBuffer[19]=')';
	UartTXMsg(UartTXBuffer,20);
}

void LinkExistCompletedMessageSendToUart(_LINK_DAT link)
{
	UartTXBuffer[0]='(';

	UartTXBuffer[1]='L';
	UartTXBuffer[2]='E';

	if(link.FlagByte & 0x02)                            //mode
	{
		ByteToChar(MODE_MASTER,&UartTXBuffer[3]);
	}
	else
	{
		ByteToChar(MODE_SLAVE,&UartTXBuffer[3]);
	}
	ByteToChar(link.GroupByte,&UartTXBuffer[5]);        //group

	ByteToChar(link.ID_HH,&UartTXBuffer[7]);            //ID
	ByteToChar(link.ID_MH,&UartTXBuffer[9]);
	ByteToChar(link.ID_ML,&UartTXBuffer[11]);
	ByteToChar(link.ID_LL,&UartTXBuffer[13]);

	ByteToChar(LinkCat,&UartTXBuffer[15]);             //cat
	ByteToChar(LinkSubCat,&UartTXBuffer[17]);          //subcat

	UartTXBuffer[19]=')';
	UartTXMsg(UartTXBuffer,20);
}

//Unlink completed ACK
void UnlinkCompletedMessageSendToUart(_LINK_DAT link)
{
	UartTXBuffer[0]='(';

	UartTXBuffer[1]='u';
	UartTXBuffer[2]='f';

	if(link.FlagByte & 0x02)                            //mode
	{
		ByteToChar(MODE_MASTER,&UartTXBuffer[3]);
	}
	else
	{
		ByteToChar(MODE_SLAVE,&UartTXBuffer[3]);
	}
	ByteToChar(link.GroupByte,&UartTXBuffer[5]);        //group

	ByteToChar(link.ID_HH,&UartTXBuffer[7]);            //ID
	ByteToChar(link.ID_MH,&UartTXBuffer[9]);
	ByteToChar(link.ID_ML,&UartTXBuffer[11]);
	ByteToChar(link.ID_LL,&UartTXBuffer[13]);

	ByteToChar(LinkCat,&UartTXBuffer[15]);              //cat
	ByteToChar(LinkSubCat,&UartTXBuffer[17]);           //subcat

	UartTXBuffer[19]=')';
	UartTXMsg(UartTXBuffer,20);
}

void UnlinkExistCompletedMessageSendToUart(_LINK_DAT link)
{
	UartTXBuffer[0]='(';

	UartTXBuffer[1]='u';
	UartTXBuffer[2]='e';

	if(link.FlagByte & 0x02)                            //mode
	{
		ByteToChar(MODE_MASTER,&UartTXBuffer[3]);
	}
	else
	{
		ByteToChar(MODE_SLAVE,&UartTXBuffer[3]);
	}
	ByteToChar(link.GroupByte,&UartTXBuffer[5]);        //group

	ByteToChar(link.ID_HH,&UartTXBuffer[7]);            //ID
	ByteToChar(link.ID_MH,&UartTXBuffer[9]);
	ByteToChar(link.ID_ML,&UartTXBuffer[11]);
	ByteToChar(link.ID_LL,&UartTXBuffer[13]);

	ByteToChar(LinkCat,&UartTXBuffer[15]);              //cat
	ByteToChar(LinkSubCat,&UartTXBuffer[17]);           //subcat

	UartTXBuffer[19]=')';
	UartTXMsg(UartTXBuffer,20);
}
//send received message to host
void SendRfMessageToUart(void)
{
	UartTXBuffer[0]='{';
	UartTXBuffer[1]='R';
	ByteToChar(RfRxBuffer[0],&UartTXBuffer[2]);
	ByteToChar(RfRxBuffer[1],&UartTXBuffer[4]);
	ByteToChar(RfRxBuffer[2],&UartTXBuffer[6]);
	ByteToChar(RfRxBuffer[3],&UartTXBuffer[8]);
	ByteToChar(RfRxBuffer[4],&UartTXBuffer[10]);
	ByteToChar(RfRxBuffer[5],&UartTXBuffer[12]);
	ByteToChar(RfRxBuffer[6],&UartTXBuffer[14]);
	ByteToChar(RfRxBuffer[7],&UartTXBuffer[16]);
	ByteToChar(RfRxBuffer[8],&UartTXBuffer[18]);
	ByteToChar(RfRxBuffer[9],&UartTXBuffer[20]);
	ByteToChar(RfRxBuffer[10],&UartTXBuffer[22]);
	ByteToChar(RfRxBuffer[11],&UartTXBuffer[24]);
	ByteToChar(RfRxBuffer[12],&UartTXBuffer[26]);
	ByteToChar(RfRxBuffer[13],&UartTXBuffer[28]);
	ByteToChar(RfRxBuffer[14],&UartTXBuffer[30]);
	ByteToChar(RfRxBuffer[15],&UartTXBuffer[32]);
	ByteToChar(RfRxBuffer[16],&UartTXBuffer[34]);
	ByteToChar(RfRxBuffer[17],&UartTXBuffer[36]);
	ByteToChar(RfRxBuffer[18],&UartTXBuffer[38]);
	ByteToChar(RfRxBuffer[19],&UartTXBuffer[40]);
	ByteToChar(RfRxBuffer[20],&UartTXBuffer[42]);
	ByteToChar(RfRxBuffer[21],&UartTXBuffer[44]);
	ByteToChar(RfRxBuffer[22],&UartTXBuffer[46]);
	ByteToChar(RfRxBuffer[23],&UartTXBuffer[48]);
	ByteToChar(RfRxBuffer[24],&UartTXBuffer[50]);
	ByteToChar(RfRxBuffer[25],&UartTXBuffer[52]);
	ByteToChar(RfRxBuffer[26],&UartTXBuffer[54]);
	ByteToChar(RfRxBuffer[27],&UartTXBuffer[56]);
	UartTXBuffer[58]='}';
	UartTXMsg(UartTXBuffer,59);
}

void SendRfMessageResponse(_MSGTYPE MsgTyp,unsigned char hops)
{
	RfTxBuffer[0] = config.MyId[0];                   //source id
	RfTxBuffer[1] = config.MyId[1];
	RfTxBuffer[2] = config.MyId[2];
	RfTxBuffer[3] = config.MyId[3];

	RfTxBuffer[4] = RfRxBuffer[0];
	RfTxBuffer[5] = RfRxBuffer[1];
	RfTxBuffer[6] = RfRxBuffer[2];
	RfTxBuffer[7] = RfRxBuffer[3];

	RfTxBuffer[8] = MsgTyp|MAX_HOPS|(hops<<2);         //message flag

	RfTxBuffer[9] = RfRxBuffer[9];;                  //command
	RfTxBuffer[10] = RfRxBuffer[10];;

	RfTxBuffer[11] = RfRxBuffer[11];;                           //master

	RfTxBuffer[12] = RfRxBuffer[12];;                      //group
	RfTxBuffer[13] = RfRxBuffer[13];;
	RfTxBuffer[14] = RfRxBuffer[14];;
	RfTxBuffer[15] = RfRxBuffer[15];;
	RfTxBuffer[16] = RfRxBuffer[16];;
	RfTxBuffer[17] = RfRxBuffer[17];;
	RfTxBuffer[18] = RfRxBuffer[18];;
	RfTxBuffer[19] = RfRxBuffer[19];;
	RfTxBuffer[20] = RfRxBuffer[20];;
	RfTxBuffer[21] = RfRxBuffer[21];;
	RfTxBuffer[22] = RfRxBuffer[22];;
	RfTxBuffer[23] = RfRxBuffer[23];;
	RfTxBuffer[24] = RfRxBuffer[24];;
	RfTxBuffer[25] = RfRxBuffer[25];;
	RfTxBuffer[26] = RfRxBuffer[26];;
	RfTxBuffer[27] = GetBufferCheckSum(RfTxBuffer,27);
	Transmit(RfTxBuffer, 28);
}

//send all-link clean up status report
void SendAllLinkCleanUpStatusReport(char status/*,_LINK_DAT LinkDat*/)
{
	UartTXBuffer[0]='{';
	UartTXBuffer[1]='P';
	UartTXBuffer[2]=status;
/*	UartTXBuffer[3]=LinkDat.ID_HH;
	UartTXBuffer[4]=LinkDat.ID_MH;
	UartTXBuffer[5]=LinkDat.ID_ML;
	UartTXBuffer[6]=LinkDat.ID_LL;*/
	UartTXBuffer[3]='}';
	UartTXMsg(UartTXBuffer,4);
}
//send change all-link group report
void SendChangeALLLinkGroupReport(unsigned char status)
{
	UartTXBuffer[0]='{';
	UartTXBuffer[1]='C';
	ByteToChar(status,&UartTXBuffer[2]);
	UartTXBuffer[4]='}';
	UartTXMsg(UartTXBuffer,5);
}

//for host update device's program'ack
void UartSendRfIspAck(unsigned int command)
{
	UartTXBuffer[0]='(';
	UartTXBuffer[1]='v';
	ByteToChar((command+1)/256,&UartTXBuffer[2]);
	ByteToChar((command+1)%256,&UartTXBuffer[4]);
	UartTXBuffer[6]=')';
	UartTXMsg(UartTXBuffer,7);
}
//for host get other device's information
void UartSendRfDeviceInformation(void)
{
	UartTXBuffer[0]='{';
	UartTXBuffer[1]='e';
	ByteToChar(RfRxBuffer[11],&UartTXBuffer[2]);
	ByteToChar(RfRxBuffer[12],&UartTXBuffer[4]);
	ByteToChar(RfRxBuffer[13],&UartTXBuffer[6]);
	ByteToChar(RfRxBuffer[14],&UartTXBuffer[8]);
	UartTXBuffer[10]='}';
	UartTXMsg(UartTXBuffer,11);
}



unsigned char CheckBroadcastLinkId(_LINKMODE MyMode)
{
	unsigned char i=0;
	for(i=0;i<ALL_LINK_SUM;i++)
	{
		ReadNBytes(i*8,(unsigned char*)&ReadLink,8);
		switch(ReadLink.FlagByte)
		{
			case 0xA0:                                 //has ID,inactive,slave
				if(MyMode==MODE_MASTER)
				{
					if(ReadLink.ID_HH==RfRxBuffer[0] && ReadLink.ID_MH==RfRxBuffer[1] && ReadLink.ID_ML==RfRxBuffer[2] && ReadLink.ID_LL==RfRxBuffer[3] && ReadLink.GroupByte==LinkGroup)
					{
						bDeviceAlreadyLink=0;
						return 1;
					}
				}
				break;
			case 0xA2:                                 //has ID,inactive,master
				if(MyMode==MODE_SLAVE)
				{
					if(ReadLink.ID_HH==RfRxBuffer[0] && ReadLink.ID_MH==RfRxBuffer[1] && ReadLink.ID_ML==RfRxBuffer[2] && ReadLink.ID_LL==RfRxBuffer[3] && ReadLink.GroupByte==RfRxBuffer[12])
					{
						bDeviceAlreadyLink=0;
						return 1;
					}
				}
			case 0xA3:                                 //has ID,active,master
				if(MyMode==MODE_SLAVE)
				{
					if(ReadLink.ID_HH==RfRxBuffer[0] && ReadLink.ID_MH==RfRxBuffer[1] && ReadLink.ID_ML==RfRxBuffer[2] && ReadLink.ID_LL==RfRxBuffer[3] && ReadLink.GroupByte==RfRxBuffer[12])
					{
						bDeviceAlreadyLink=1;
						return 1;
					}
				}
				break;
			case 0xA1:                                 //has ID,active,slave                         //already link
				if(MyMode==MODE_MASTER)
				{
					if(ReadLink.ID_HH==RfRxBuffer[0] && ReadLink.ID_MH==RfRxBuffer[1] && ReadLink.ID_ML==RfRxBuffer[2] && ReadLink.ID_LL==RfRxBuffer[3] && ReadLink.GroupByte==LinkGroup)
					{
						bDeviceAlreadyLink=1;                //device data already exsit
						return 1;
					}
				}
				break;
			default:
				bDeviceAlreadyLink=0;
				return 1;                            //has checked all exist All-link data
		}
	}
	bDeviceAlreadyLink=0;
	return 0;
}

unsigned char CheckBroadcastUnlinkId(_LINKMODE MyMode)
{
	unsigned char i=0;
	for(i=0;i<ALL_LINK_SUM;i++)
	{
		ReadNBytes(i*8,(unsigned char*)&ReadLink,8);
		switch(ReadLink.FlagByte)
		{
			case 0xA0:                                 //has ID,inactive,slave
				if(MyMode==MODE_MASTER)
				{
					if(ReadLink.ID_HH==RfRxBuffer[0] && ReadLink.ID_MH==RfRxBuffer[1] && ReadLink.ID_ML==RfRxBuffer[2] && ReadLink.ID_LL==RfRxBuffer[3] && ReadLink.GroupByte==LinkGroup)
					{
						bDeviceAlreadyUnlink=1;            //device data already unlink
						return 1;
					}
				}
				break;
			case 0xA2:                                 //has ID,inactive,master
				if(MyMode==MODE_SLAVE)
				{
					if(ReadLink.ID_HH==RfRxBuffer[0] && ReadLink.ID_MH==RfRxBuffer[1] && ReadLink.ID_ML==RfRxBuffer[2] && ReadLink.ID_LL==RfRxBuffer[3] && ReadLink.GroupByte==RfRxBuffer[12])
					{
						bDeviceAlreadyUnlink=1;
						return 1;
					}
				}
				break;
			case 0xA3:                                 //has ID,active,master
				if(MyMode==MODE_SLAVE)
				{
					if(ReadLink.ID_HH==RfRxBuffer[0] && ReadLink.ID_MH==RfRxBuffer[1] && ReadLink.ID_ML==RfRxBuffer[2] && ReadLink.ID_LL==RfRxBuffer[3] && ReadLink.GroupByte==RfRxBuffer[12])
					{
						bDeviceAlreadyUnlink=0;
						return 1;
					}
				}
				break;
			case 0xA1:                                 //has ID,active,slave                         //already link
				if(MyMode==MODE_MASTER)
				{
					if(ReadLink.ID_HH==RfRxBuffer[0] && ReadLink.ID_MH==RfRxBuffer[1] && ReadLink.ID_ML==RfRxBuffer[2] && ReadLink.ID_LL==RfRxBuffer[3] && ReadLink.GroupByte==LinkGroup)
					{
						bDeviceAlreadyUnlink=0;
						return 1;
					}
				}
				break;
			default:
				bDeviceAlreadyUnlink=0;
				return 0;                            //has checked all exist All-link data
		}
	}
	bDeviceAlreadyUnlink=0;
	return 0;
}


unsigned char DirectMsgCheckLinkId(void)
{
	unsigned char i=0;
	for(i=0;i<ALL_LINK_SUM;i++)
	{
		ReadNBytes(i*8,(unsigned char*)&ReadLink,8);
		switch(ReadLink.FlagByte)
		{
			case 0xA0:                                 //has ID,inactive,slave
			case 0xA2:                                 //has ID,inactive,master
				if(ReadLink.ID_HH==RfRxBuffer[0] && ReadLink.ID_MH==RfRxBuffer[1] && ReadLink.ID_ML==RfRxBuffer[2] && ReadLink.ID_LL==RfRxBuffer[3] && ReadLink.GroupByte==RfRxBuffer[12])
				{
					bDeviceAlreadyLink=0;
					return 1;
				}
				break;
			case 0xA1:                                 //has ID,active,slave
				break;
			case 0xA3:                                 //has ID,active,master
				if(ReadLink.ID_HH==RfRxBuffer[0] && ReadLink.ID_MH==RfRxBuffer[1] && ReadLink.ID_ML==RfRxBuffer[2] && ReadLink.ID_LL==RfRxBuffer[3] && ReadLink.GroupByte==RfRxBuffer[12])
				{
					bDeviceAlreadyLink=1;
					return 1;
				}
				break;
			default:
				bDeviceAlreadyLink=0;
				return 1;                            //has checked all exist All-link data
		}
	}
	bDeviceAlreadyLink=0;
	return 0;
}

unsigned char DirectMsgCheckUnlinkId(void)
{
	unsigned char i=0;
	for(i=0;i<ALL_LINK_SUM;i++)
	{
		ReadNBytes(i*8,(unsigned char*)&ReadLink,8);
		switch(ReadLink.FlagByte)
		{
			case 0xA0:                                 //has ID,inactive,slave
			case 0xA2:                                 //has ID,inactive,master
				if(ReadLink.ID_HH==RfRxBuffer[0] && ReadLink.ID_MH==RfRxBuffer[1] && ReadLink.ID_ML==RfRxBuffer[2] && ReadLink.ID_LL==RfRxBuffer[3] && ReadLink.GroupByte==RfRxBuffer[12])
				{
					bDeviceAlreadyUnlink=1;
					return 1;
				}
				break;
			case 0xA1:                                 //has ID,active,slave
				break;
			case 0xA3:                                 //has ID,active,master
				if(ReadLink.ID_HH==RfRxBuffer[0] && ReadLink.ID_MH==RfRxBuffer[1] && ReadLink.ID_ML==RfRxBuffer[2] && ReadLink.ID_LL==RfRxBuffer[3] && ReadLink.GroupByte==RfRxBuffer[12])
				{
					bDeviceAlreadyUnlink=0;
					return 1;
				}
				break;
			default:
				bDeviceAlreadyUnlink=0;
				return 0;                            //has checked all exist All-link data
		}
	}
	bDeviceAlreadyUnlink=0;
	return 0;
}

//rf receive message handle
void RFMsgHandle(void)
{
	unsigned int i=0;
	unsigned int command;
#if RF_TO_UART==1
	SendRfMessageToUart();
	return;
#endif
	command=RfRxBuffer[9]*256+RfRxBuffer[10];
	switch(RfRxBuffer[8] & 0xf0)
	{
		case Direct:                               //Direct Message
			if(RfRxBuffer[4] == config.MyId[0] && RfRxBuffer[5] == config.MyId[1] && RfRxBuffer[6] == config.MyId[2] && RfRxBuffer[7] == config.MyId[3])   //send to me
			{
				switch(command)
				{
					case CMD_RFLINK:
						switch(RfRxBuffer[11])
						{
							case MODE_SLAVE:                    //in linking state,slave only receive master's direct message,so it is fault
							case MODE_EACH:                     //mode each only in broadcast message,so it is fault
								break;
							case MODE_MASTER:                                  //i am slave
								if(LinkState==HAS_SEND_BROADCAST && (LinkMode==MODE_SLAVE || LinkMode==MODE_EACH))
								{
									if(!DirectMsgCheckLinkId())break;
									LinkMode=MODE_SLAVE;
									bLinkRetry=0;
									LinkGroup=RfRxBuffer[12];                              //get the group number
									DirectlyTransmitRfLinkMsg(DirectAck,MODE_SLAVE,0);
									WriteLink.FlagByte=GRP_ID_MASK|0x03;                 //add mask and set active mode,target is master
									WriteLink.GroupByte=RfRxBuffer[12];                    //group
									WriteLink.ID_HH=RfRxBuffer[0];                         //ID
									WriteLink.ID_MH=RfRxBuffer[1];
									WriteLink.ID_ML=RfRxBuffer[2];
									WriteLink.ID_LL=RfRxBuffer[3];
									WriteLink.Dat1=RfRxBuffer[13];                         //2bytes append data
									WriteLink.Dat2=RfRxBuffer[14];
									SaveAllLinkData(WriteLink);
									LinkState=DID_NOT_START;
									bStartLink=0;
									if(!bDeviceAlreadyLink)          //link complete give message to host
									{
										LinkCompletedMessageSendToUart(WriteLink);
									}
									else                            //already link,do nothing
									{
										LinkExistCompletedMessageSendToUart(WriteLink);
										bDeviceAlreadyLink=0;
									}

								}
								break;
							case MODE_DELETE:                           //unlink,i am slave
								if(LinkState==HAS_SEND_BROADCAST && LinkMode==MODE_DELETE)
								{
									if(!DirectMsgCheckUnlinkId())break;
									bLinkRetry=0;
									LinkGroup=RfRxBuffer[12];                              //get the group number
									DirectlyTransmitRfLinkMsg(DirectAck,MODE_DELETE,0);
									UnLinkDat.GroupByte=RfRxBuffer[12];                    //group
									UnLinkDat.ID_HH=RfRxBuffer[0];                         //ID
									UnLinkDat.ID_MH=RfRxBuffer[1];
									UnLinkDat.ID_ML=RfRxBuffer[2];
									UnLinkDat.ID_LL=RfRxBuffer[3];                         //append data ignore
									UnlinkAllLinkData(UnLinkDat);
									LinkState=DID_NOT_START;
									bStartLink=0;
									if(!bDeviceAlreadyUnlink)          //unlink complete give message to host
									{
										UnlinkCompletedMessageSendToUart(UnLinkDat);
									}
									else                            //already unlink,do nothing
									{
										UnlinkExistCompletedMessageSendToUart(UnLinkDat);
										bDeviceAlreadyUnlink=0;
									}
								}
								break;
							default:
								break;
						}
						break;
					default:
						SendRfMessageToUart();                   //send to host
						break;
				}
			}
			break;
		case BroadCast:                               //Broadcast Message
			switch(command)
			{
				case CMD_RFLINK:
					if(bStartLink==1)                    //already in linking state
					{
						switch(RfRxBuffer[11])
						{
							case MODE_SLAVE:
								if(LinkMode==MODE_MASTER)                     //I AM MASTER already send broadcast message
								{
									if(!CheckBroadcastLinkId(MODE_MASTER))break;
									LinkCat=RfRxBuffer[4];                //slave cat
									LinkSubCat=RfRxBuffer[5];             //slave subcat
									DirectlyTransmitRfLinkMsg(Direct,MODE_MASTER,0);
									LinkState=HAS_SEND_DIRETMSG;
									bLinkRetry=1;
									LinkRetryCount=0;
									LinkRetry=0;
								}
								break;
							case MODE_MASTER:
								if(LinkMode==MODE_SLAVE)                      //I AM SLAVER need resend broadcast message
								{
									if(!CheckBroadcastLinkId(MODE_SLAVE))break;
									LinkCat=RfRxBuffer[4];              //target cat
									LinkSubCat=RfRxBuffer[5];           //target subcat
									BroadcastTransmitRfLinkMsg(MODE_SLAVE,0);
									LinkState=HAS_SEND_BROADCAST;
									bLinkRetry=1;
									LinkRetryCount=0;
									LinkRetry=0;
								}
								break;
							case MODE_EACH:
								if(LinkMode==MODE_EACH)                      //CAN BE EACH i will be master,and will send direct message
								{
									if(!CheckBroadcastLinkId(MODE_MASTER))break;
									LinkMode=MODE_MASTER;
									LinkCat=RfRxBuffer[4];              //target cat
									LinkSubCat=RfRxBuffer[5];           //target subcat
									DirectlyTransmitRfLinkMsg(Direct,MODE_MASTER,0);                         //I AM MASTER
									LinkState=HAS_SEND_DIRETMSG;
									bLinkRetry=1;
									LinkRetryCount=0;
									LinkRetry=0;
								}
								break;
							case MODE_DELETE:
								if(LinkMode==MODE_DELETE)                    //UNLINK,i am already in unlink state,so i am master
								{
									if(!CheckBroadcastUnlinkId(MODE_MASTER))break;
									LinkCat=RfRxBuffer[4];                //target cat
									LinkSubCat=RfRxBuffer[5];             //target subcat
									DirectlyTransmitRfLinkMsg(Direct,MODE_DELETE,0);
									LinkState=HAS_SEND_DIRETMSG;
									bLinkRetry=1;
									LinkRetryCount=0;
									LinkRetry=0;
								}
								break;
							default:
								break;
						}
					}
					break;
				default:                   //send to uart
					SendRfMessageToUart();
					break;
			}
			break;
		case AllLinkBroadcast:                               //ALL-Link Broadcast Message
			if(!AllLinkBroadcastMessageCheckID())break;             //not to me,break
			SendRfMessageToUart();
			break;
		case DirectAck:                                      //ACK of Direct Message
			if(RfRxBuffer[4] == config.MyId[0] && RfRxBuffer[5] == config.MyId[1] && RfRxBuffer[6] == config.MyId[2] && RfRxBuffer[7] == config.MyId[3])   //send to me
			{
				bDirectRetry=0;
				switch(command)
				{
					case CMD_RFLINK:                         //self is master,target is slave
						if(bStartLink==1)                    //already in linking state
						{
							switch(RfRxBuffer[11])
							{
								case MODE_SLAVE:
									if(LinkMode==MODE_MASTER && LinkState==HAS_SEND_DIRETMSG)
									{
										bLinkRetry=0;
										WriteLink.FlagByte=GRP_ID_MASK|0x01;                   //add mask and set active mode,target is slave
										WriteLink.GroupByte=RfRxBuffer[12];                    //group number :equal LinkGroup(set by uart)
										WriteLink.ID_HH=RfRxBuffer[0];                         //ID
										WriteLink.ID_MH=RfRxBuffer[1];
										WriteLink.ID_ML=RfRxBuffer[2];
										WriteLink.ID_LL=RfRxBuffer[3];
										WriteLink.Dat1=RfRxBuffer[13];                         //2bytes append data
										WriteLink.Dat2=RfRxBuffer[14];
										SaveAllLinkData(WriteLink);
										if(bDeviceAlreadyLink)             //i am master already link target
										{
											bDeviceAlreadyLink=0;
											LinkState=HAS_SEND_BROADCAST;
											LinkExistCompletedMessageSendToUart(WriteLink);
										}
										else
										{
											LinkCompletedMessageSendToUart(WriteLink);
											LinkState=DID_NOT_START;             //reset state machine
											bStartLink=0;
										}
									}
									break;
								case MODE_DELETE:
									if(LinkMode==MODE_DELETE && LinkState==HAS_SEND_DIRETMSG)
									{
										bLinkRetry=0;
										UnLinkDat.GroupByte=RfRxBuffer[12];                    //group
										UnLinkDat.ID_HH=RfRxBuffer[0];                         //ID
										UnLinkDat.ID_MH=RfRxBuffer[1];
										UnLinkDat.ID_ML=RfRxBuffer[2];
										UnLinkDat.ID_LL=RfRxBuffer[3];                         //append data ignore
										UnlinkAllLinkData(UnLinkDat);
										if(bDeviceAlreadyUnlink)             //i am master already link target
										{
											bDeviceAlreadyUnlink=0;
											LinkState=HAS_SEND_BROADCAST;
											UnlinkExistCompletedMessageSendToUart(UnLinkDat);
										}
										else
										{
											UnlinkCompletedMessageSendToUart(UnLinkDat);
											LinkState=DID_NOT_START;             //reset state machine
											bStartLink=0;
										}
									}
									break;
								case MODE_MASTER:                         //master can not send link ack.so it is fault
								case MODE_EACH:                           //mode only in broadcast message,so it is fault
									break;
							}
						}
						break;
					case CHANGE_ALL_LINK_GROUP:
						if(RfRxBuffer[13]==0x00)                 //success
						{
							WriteLink.ID_HH=RfRxBuffer[0];
							WriteLink.ID_MH=RfRxBuffer[1];
							WriteLink.ID_ML=RfRxBuffer[2];
							WriteLink.ID_LL=RfRxBuffer[3];
							RfRxBuffer[13]=AddOrSubAllLinkGroup(WriteLink,RfRxBuffer[11],RfRxBuffer[12]);        //new group
						}
						SendChangeALLLinkGroupReport(RfRxBuffer[13]);
						break;
					case START_SEND_CODE:
					case SEND_CODE_CONTENT:
					case FINISH_SEND_CODE:
						UartSendRfIspAck(command);
						break;
					default:                 //send to uart
						SendRfMessageToUart();
						break;
				}
			}
			break;
		case DirectNack:                                     //NAK of Direct Message
			if(RfRxBuffer[4] == config.MyId[0] && RfRxBuffer[5] == config.MyId[1] && RfRxBuffer[6] == config.MyId[2] && RfRxBuffer[7] == config.MyId[3])
			{
				bDirectRetry=0;
				SendRfMessageToUart();
			}
			break;
		case AllLinkCleanUp:                                 //ALL-Link Cleanup Message
			if(RfRxBuffer[4] == config.MyId[0] && RfRxBuffer[5] == config.MyId[1] && RfRxBuffer[6] == config.MyId[2] && RfRxBuffer[7] == config.MyId[3])
			{
				SendRfMessageResponse(AllLinkCleanUpAck,0);
				SendRfMessageToUart();
			}
			break;
		case AllLinkCleanUpAck:                              //ACK of ALL-Link Cleanup Message,send next Message
			if(RfRxBuffer[4] == config.MyId[0] && RfRxBuffer[5] == config.MyId[1] && RfRxBuffer[6] == config.MyId[2] && RfRxBuffer[7] == config.MyId[3])
			{
				bCleanUpRetry=0;
				for(i=AllLinkAddr/8;i<ALL_LINK_SUM;i++)
				{
					AllLinkAddr=i*8;
					ReadNBytes(AllLinkAddr,(unsigned char*)&ReadLink,8);
					if((ReadLink.FlagByte & 0xf0)!=0xA0)                                 //check mask
					{
						SendAllLinkCleanUpStatusReport(CleanUpStatus);                 //ALL-Link cleanup complete
						CleanUpStatus='1';               //reset
						break;                           //didn't have ALL-Link data
					}
					if(ReadLink.GroupByte!=GroupNum)continue;                         //group number not match,check next record
					switch(ReadLink.FlagByte)
					{
						case 0xA0:                                 //has ID,inactive,slave
						case 0xA2:                                 //has ID,inactive,master
							break;
						case 0xA3:
						case 0xA1:                                 //has ID,active,slave
							RfTxBuffer[0] = config.MyId[0];                            //source id
							RfTxBuffer[1] = config.MyId[1];
							RfTxBuffer[2] = config.MyId[2];
							RfTxBuffer[3] = config.MyId[3];
							RfTxBuffer[4] = ReadLink.ID_HH;
							RfTxBuffer[5] = ReadLink.ID_MH;
							RfTxBuffer[6] = ReadLink.ID_ML;
							RfTxBuffer[7] = ReadLink.ID_LL;
							RfTxBuffer[8] = AllLinkCleanUp;                   //flag
							RfTxBuffer[27]=GetBufferCheckSum(RfTxBuffer,27);
							AllLinkAddr+=8;                                     //set the start address for ALL-Link clean up message's ID
							Transmit(RfTxBuffer, 28);
							return;
						default:break;
					}
				}
			}
			break;
		case AllLinkCleanUpNack:                             //NAK of ALL-Link Cleanup Message,retry
			if(RfRxBuffer[4] == config.MyId[0] && RfRxBuffer[5] == config.MyId[1] && RfRxBuffer[6] == config.MyId[2] && RfRxBuffer[7] == config.MyId[3])
			{
				bCleanUpRetry=0;
				//SendAllLinkCleanUpStatusReport('0');
			}
			break;
		default:
			break;
	}
}

void RfTransmitDoneHandle(void)
{
	unsigned int i=0;
	unsigned int Command=0;
	Command=RfTxBuffer[9]*256+RfTxBuffer[10];
	switch(RfTxBuffer[8]&0xf0)
	{
		case AllLinkBroadcast:                               //ALL-Link Broadcast Message,need send ALL-Link cleanup message to every group device
			bHasSendAllLinkBroadcast=1;                     //set flag
			AllLinkAddr=0;
			for(i=0;i<ALL_LINK_SUM;i++)
			{
				AllLinkAddr=i*8;
				ReadNBytes(AllLinkAddr,(unsigned char*)&ReadLink,8);
				if((ReadLink.FlagByte & 0xf0)!=0xA0)
				{
					CleanUpStatus='0';
					SendAllLinkCleanUpStatusReport(CleanUpStatus);              //no all_link data
					bHasSendAllLinkBroadcast=0;
					break;
				}
				if(ReadLink.GroupByte!=RfTxBuffer[7])continue;                         //group number not match,seek next group data
				GroupNum=RfTxBuffer[7];
				switch(ReadLink.FlagByte)
				{
					case 0xA0:                                 //has ID,inactive,slave
					case 0xA2:                                 //has ID,inactive,master
					case 0xA3:
						break;
					case 0xA1:                                 //has ID,active,slave
						RfTxBuffer[0] = config.MyId[0];                            //source id
						RfTxBuffer[1] = config.MyId[1];
						RfTxBuffer[2] = config.MyId[2];
						RfTxBuffer[3] = config.MyId[3];
						RfTxBuffer[4] = ReadLink.ID_HH;
						RfTxBuffer[5] = ReadLink.ID_MH;
						RfTxBuffer[6] = ReadLink.ID_ML;
						RfTxBuffer[7] = ReadLink.ID_LL;
						RfTxBuffer[8] = AllLinkCleanUp;                   //flag
						RfTxBuffer[27]=GetBufferCheckSum(RfTxBuffer,27);
						AllLinkAddr+=8;
						Transmit(RfTxBuffer, 28);
						return;
					default:break;
				}
			}
			break;
		case AllLinkCleanUp:                                 //ALL-Link Cleanup Message,need receive ack
			if(bHasSendAllLinkBroadcast==1)               //already send all_link broadcast
			{
				bHasSendAllLinkBroadcast=0;
				bCleanUpRetry=1;
				CleanUpRetry=0;
				CleanUpRetryCount=0;
			}
			else if(CleanUpRetry==3)                      //has retry 3 times
			{
				CleanUpRetry=0;
				bCleanUpRetry=0;
				bHasSendAllLinkBroadcast=1;
				CleanUpStatus='2';                       //lost some one
				for(i=AllLinkAddr/8;i<ALL_LINK_SUM;i++)   //seek another group link data
				{
					AllLinkAddr=i*8;
					ReadNBytes(AllLinkAddr,(unsigned char*)&ReadLink,8);
					if((ReadLink.FlagByte & 0xf0)!=0xA0)
					{
						SendAllLinkCleanUpStatusReport(CleanUpStatus);
						bHasSendAllLinkBroadcast=0;
						break;                           //didn't have ALL-Link data
					}
					if(ReadLink.GroupByte!=GroupNum)continue;                         //group number not match,seek next data group
					switch(ReadLink.FlagByte)
					{
						case 0xA0:                                 //has ID,inactive,slave
						case 0xA2:                                 //has ID,inactive,master
						case 0xA3:
							break;
						case 0xA1:                                 //has ID,active,slave
							RfTxBuffer[0] = config.MyId[0];                            //source id
							RfTxBuffer[1] = config.MyId[1];
							RfTxBuffer[2] = config.MyId[2];
							RfTxBuffer[3] = config.MyId[3];
							RfTxBuffer[4] = ReadLink.ID_HH;
							RfTxBuffer[5] = ReadLink.ID_MH;
							RfTxBuffer[6] = ReadLink.ID_ML;
							RfTxBuffer[7] = ReadLink.ID_LL;
							RfTxBuffer[8] = AllLinkCleanUp;                   //flag
							RfTxBuffer[27]=GetBufferCheckSum(RfTxBuffer,27);
							AllLinkAddr+=8;                                     //set the start address for ALL-Link clean up message's ID
							Transmit(RfTxBuffer, 28);
							return;
						default:break;
					}
				}
			}
			break;
		case Direct:                                         //Direct Message,need receive ack,to remote direct message just for link and unlink
			switch(Command)
			{
				case CMD_RFLINK:                                                          //i am master
				default:
					if(DirectRetry==3)
					{
						bDirectRetry=0;
						DirectRetry=0;
					}
					break;
			}
			break;
		case BroadCast:                                      //link or unlink
		case AllLinkCleanUpAck:                              //ACK of ALL-Link Cleanup Message
		case DirectNack:                                     //NAK of Direct Message
		case AllLinkCleanUpNack:                             //NAK of ALL-Link Cleanup Message
		case DirectAck:
			break;
		default:break;
	}
}


