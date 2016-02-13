#include "globa.h"

/*
 * main.c
 */
volatile IntFlags ModuleF1;
unsigned char RxBufferLength=0;
unsigned char RfRxBuffer[64];
unsigned char RfTxBuffer[28];

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    SetVCore(3);
	ResetRadioCore();
	Init_FLL(20000000 / 1000, 20000000 / 32768);
	InitRadio();
	IOInit();
	ModuleF1.all_flags=0;
	Init_FlashMemory();
	UartInit();
	ReadConfigData();
	ReceiveOn();
	StartTimer1();
	_EINT();
#if UART_TEST==1
	Uart_Test();
#endif
#if RF_T_TEST==1
	RfTestTX();
#endif
#if RF_R_TEST==1
	RfTestRX();
#endif
	while(1)
	{
		if(bUartCmdDone==1)                                                        //receive uart message
		{
			bUartCmdDone=0;
			UartMessageHandle();
		}
		else if(bTransmitDone==1)
		{
			bTransmitDone=0;
			RfTransmitDoneHandle();
		}
		else if(bReceivedMsg==1)                                                   //receive  rf message
		{
			bReceivedMsg=0;
			RxBufferLength=ReadSingleReg( RXBYTES );
			if(RxBufferLength<65)
			{
				_DINT();
				ReadBurstReg(RF_RXFIFORD, RfRxBuffer, RxBufferLength);
				_EINT();
				if(RxBufferLength==30 && RfRxBuffer[CRC_LQI_IDX]&CRC_OK)
				{
					if(CheckBufferCheckSum(RfRxBuffer,27))
						RFMsgHandle();
				}
			}
			else
			{
				ResetRadioCore();
				InitRadio();
				ReceiveOn();
			}
		}
	}
}


#pragma vector=CC1101_VECTOR
__interrupt void CC1101_ISR(void)
{
	switch(__even_in_range(RF1AIV,32))          // Prioritizing Radio Core Interrupt
	{
    	case  0: break;                         // No RF core interrupt pending
    	case  2: break;                         // RFIFG0
    	case  4: break;                         // GDO1 = LNA_PD signal                     // Go back to sleep
    	case  6: break;                         // RFIFG2
    	case  8: break;                         // RFIFG3
    	case 10: break;                         // RFIFG4
    	case 12: break;                         // RFIFG5
    	case 14: break;                         // RFIFG6
    	case 16: break;                         // RFIFG7
    	case 18: break;                         // RFIFG8
    	case 20:                                // RFIFG9
    		if(bTransmitting==1)                // TX end of packet
    		{
    			bTransmitting = 0;
    			bTransmitDone=1;
    			bReceiving=1;
    		}
    		else if(bReceiving==1)              // RX end of packet
    		{
    			bReceivedMsg=1;
    		}
    		break;
    	case 22: break;                         // RFIFG10
    	case 24: break;                         // RFIFG11
    	case 26: break;                         // RFIFG12
    	case 28: break;                         // RFIFG13
    	case 30: break;                         // WOR_EVENT0
    	case 32: break;                         // RFIFG15
	}
	RF1AIFG=0;
}



