/*
 * timer.c
 *
 *  Created on: 2015-6-15
 *      Author: wangmg
 */

#include "globa.h"

unsigned int msCounter=0;
unsigned int LinkTimeCount=0;             //linking state count

//link
unsigned int  LinkRetryCount=0;
unsigned char LinkRetry=0;
//cleanup
unsigned int  CleanUpRetryCount=0;
unsigned char CleanUpRetry=0;
//direct
unsigned int  DirectRetryCount=0;
unsigned char DirectRetry=0;


void StartTimer1(void)
{
	TA1CCTL0 = CCIE;                          // CCR0 interrupt enabled
	TA1CCR0 = 20000;
	TA1CTL = TASSEL_2 + MC_1  +TACLR;         // SMCLK, upmode, clear TAR
}

void StopTimer1(void)
{
	TA1CTL = 0;         	// SMCLK, upmode, clear TAR
}

// Timer A0 interrupt service routine
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
	msCounter++;
	if(bDirectRetry==1)                                   //direct message retry
	{
		DirectRetryCount++;
		if(DirectRetryCount>=200)
		{
			DirectRetryCount=0;
			if(DirectRetry<3)
			{
				Transmit(RfTxBuffer, 28);
				DirectRetry++;
			}
		}
	}
	if(bCleanUpRetry==1)                                             //cleanup message retry
	{
		CleanUpRetryCount++;
		if(CleanUpRetryCount>=200)
		{
			CleanUpRetryCount=0;
			if(CleanUpRetry<3)
			{
				Transmit(RfTxBuffer, 28);
				CleanUpRetry++;
			}
		}
	}
	if(bLinkRetry==1)                                          //link broadcast retry
	{
		LinkRetryCount++;
		if(LinkRetryCount>=500)
		{
			LinkRetryCount=0;
			if(LinkRetry<3)
			{
				Transmit(RfTxBuffer, 28);
				LinkRetry++;
			}
			else
			{
				bLinkRetry=0;
				LinkRetry=0;
			}
		}
	}
	if(msCounter>=1000)                                 //link or unlink timeout
	{
		msCounter=0;
		if(bStartLink==1)
		{
			if(LinkTimeCount++>=LINKING_TIMEOUT)        //wait for 4 minutes linking timeout
			{
				bStartLink=0;
				LinkTimeCount=0;
			}
		}
	}
}


