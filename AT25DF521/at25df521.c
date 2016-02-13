/*
 * 24lc04.c
 *
 *  Created on: 2015-5-15
 *      Author: wangmg
 */

#include "at25df521.h"


unsigned char FlashBuffer[256];

void delay_nus(unsigned int us)
{
	while(us--)
	{
		__delay_cycles(20);
	}
}

void delay_nms(unsigned int ms)
{
	while(ms--)
	{
		__delay_cycles(20000);
	}
}

void Init_FlashMemory(void)
{
	P1SEL=0;
	P1DIR |= BIT3 + BIT1 + BIT4;
	P1DIR &=~ BIT2;
	CS_1;
	CLK_0;
}

unsigned char SPI_RW_OneByte(unsigned char u8_writedata)
{
	unsigned char i,temp;
	temp=0;
	CLK_0;
	for(i=0;i<8;i++)
	{
		if(u8_writedata&0x80)DIO_1;
		else DIO_0;
		u8_writedata<<=1;
		CLK_1;
		temp<<=1;
		if(DO_IN)temp|=0x01;
		else temp&=~0x01;
		CLK_0;
	}
	return temp;
}

void ReadNBytes(unsigned int addr,unsigned char *buf,unsigned int len)
{
	unsigned int i;
	unsigned char temp;
	CS_0;
	SPI_RW_OneByte(READ_MASK);
	SPI_RW_OneByte(0);
	temp=(addr&0xff00)>>8;
	SPI_RW_OneByte(temp);
	temp=addr&0xff;
	SPI_RW_OneByte(temp);
	for(i=0;i<len;i++)
	{
		buf[i]=SPI_RW_OneByte(0);
	}
	CS_1;
}

void WriteBytes(unsigned int addr,unsigned char *buf,unsigned int len)
{
	unsigned int i=0;
	unsigned char temp;
	Enable_DataWrite();
	CS_0;
	SPI_RW_OneByte(WRITE_MASK);
	SPI_RW_OneByte(0);
	temp=(addr&0xff00)>>8;
	SPI_RW_OneByte(temp);
	temp=addr&0xff;
	SPI_RW_OneByte(temp);
	for(i=0;i<len;i++)
	{
		SPI_RW_OneByte(buf[i]);
	}
	CS_1;
	while(BSY==(ReadStatusRegister()&0x01));
}

void WriteNBytes(unsigned int addr,unsigned char *buf,unsigned int len)
{
	unsigned char i=0;
	unsigned int pageNum=0;
	unsigned int pageOffset=0;
	unsigned int pageRemain=0;

	pageNum=addr/256;
	pageOffset=addr%256;
	pageRemain=256-pageOffset;
	if(len<=pageRemain)pageRemain=len;
	while(1)
	{
		ReadNBytes(pageNum*256,FlashBuffer,256);                    //read whole page
		for(i=0;i<pageRemain;i++)
		{
			if(FlashBuffer[pageOffset+i]!=0xff)break;               //need erase page
		}
		if(i<pageRemain)
		{
			EraseSinglePage(pageNum);
			for(i=0;i<pageRemain;i++)
			{
				FlashBuffer[pageOffset+i]=buf[i];                   //copy data
			}
			WriteBytes(pageNum*256,FlashBuffer,256);
		}
		else
		{
			WriteBytes(addr,buf,pageRemain);
		}
		if(len==pageRemain)break;                                  //finish write success
		else
		{
			pageNum++;
			pageOffset=0;
			buf+=pageRemain;
			addr+=pageRemain;
			len-=pageRemain;
			if(len>256)pageRemain=256;
			else pageRemain=len;
		}
	}
}

char WriteNBytesWithCheck(unsigned int addr,unsigned char *buf,unsigned int len)
{
	unsigned char rty=0;
	unsigned int i=0;
	while(rty<3)
	{
		WriteNBytes(addr,buf,len);
		ReadNBytes(addr,FlashBuffer,len);
		for(i=0;i<len;i++)
		{
			if(FlashBuffer[i]!=buf[i])break;
		}
		if(i==len)return ('1');
		else rty++;
	}
	return ('0');
}

/***********************************************/
void Disable_DataWrite(void)
{
	CS_0;
	SPI_RW_OneByte(WRDI);
	CS_1;
}

void Enable_DataWrite(void)
{
	CS_0;
	SPI_RW_OneByte(WREN);
	CS_1;
}

unsigned char ReadStatusRegister(void)
{
	unsigned char status=0;
	CS_0;
	SPI_RW_OneByte(RDSR);
	status=SPI_RW_OneByte(0);
	CS_1;
	return status;
}

void WriteStatusRegister(unsigned char sr)
{
	Enable_DataWrite();
	CS_0;
	SPI_RW_OneByte(WRSR1);
	SPI_RW_OneByte(sr);
	CS_1;
}
/*************************************************/       //erase function
void EraseWholeChip(void)
{
	Enable_DataWrite();
	CS_0;
	SPI_RW_OneByte(CHIP_ERASE);
	CS_1;
	while(BSY==(ReadStatusRegister()&0x01));
}

void EraseFirstHalfChip(void)                    //store the ALL_Link data
{
	Enable_DataWrite();
	CS_0;
	SPI_RW_OneByte(BLOCK_ERASE_32K);
	SPI_RW_OneByte(0);
	SPI_RW_OneByte(0);
	SPI_RW_OneByte(0);
	CS_1;
	while(BSY==(ReadStatusRegister()&0x01));
}

void EraseSecondHalfChip(void)                  //store the ISP program
{
	Enable_DataWrite();
	CS_0;
	SPI_RW_OneByte(BLOCK_ERASE_32K);
	SPI_RW_OneByte(0);
	SPI_RW_OneByte(0x80);
	SPI_RW_OneByte(0);
	CS_1;
	while(BSY==(ReadStatusRegister()&0x01));
}

void EraseSinglePage(unsigned char addr)
{
	Enable_DataWrite();
	CS_0;
	SPI_RW_OneByte(PAGE_ERASE);
	SPI_RW_OneByte(0);
	SPI_RW_OneByte(addr);
	SPI_RW_OneByte(0);
	//SPI_RW_OneByte((addr&0xff00)>>8);
	//SPI_RW_OneByte(addr&0xff);
	CS_1;
	while(BSY==(ReadStatusRegister()&0x01));
}

/**************************************************/          //for test
void Flash_ReadID(unsigned char *buf)
{
	CS_0;
	SPI_RW_OneByte(RDID);
	*buf++=SPI_RW_OneByte(0);
	*buf=SPI_RW_OneByte(0);
	CS_1;
}
void FlashReadJEDEC(unsigned char *buf)
{
	CS_0;
	SPI_RW_OneByte(RDJEDEC);
	*buf++=SPI_RW_OneByte(0);
	*buf++=SPI_RW_OneByte(0);
	*buf++=SPI_RW_OneByte(0);
	*buf=SPI_RW_OneByte(0);
	CS_1;
}
/**************************************************/


