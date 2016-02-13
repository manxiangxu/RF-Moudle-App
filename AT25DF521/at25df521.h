/*
 * 24lc04.h
 *
 *  Created on: 2015-5-15
 *      Author: wangmg
 */

#ifndef _AT25DF521_H_
#define _AT25DF521_H_

#include "cc430f5137.h"


#define          BSY               0x01

#define          READ_MASK          0X03//0x0B
//diffrence between 0x03 and 0x0b:0x03 up to 33Mhz,0x0b up to 104Mhz,when use 0x0b,need additional dummy byte follow the 3 bytes address
#define          WRITE_MASK         0X02
#define          WRDI               0X04
#define          WREN               0X06
#define          RDSR               0X05
#define          WRSR1              0x01
#define          WRSR2              0x31

#define          RDID               0x15
#define          RDJEDEC            0x9f
#define          RESET              0xf0

#define          CHIP_ERASE         0x60//0xc7                //64k bytes
#define          PAGE_ERASE         0x81                //256 bytes
#define          BLOCK_ERASE_4K     0x20                // 4k bytes
#define          BLOCK_ERASE_32K    0xd8                //32k bytes



#define          CS_1           P1OUT |= BIT1
#define          CS_0           P1OUT &= ~BIT1
#define          CLK_1          P1OUT |= BIT4
#define          CLK_0          P1OUT &= ~BIT4
#define          DIO_1          P1OUT |= BIT3
#define          DIO_0          P1OUT &= ~BIT3
#define          DO_IN          (P1IN & BIT2)






void delay_nus(unsigned int us);
void delay_nms(unsigned int ms);

void Init_FlashMemory(void);
void Disable_DataWrite(void);
void Enable_DataWrite(void);
unsigned char ReadStatusRegister(void);
void WriteStatusRegister(unsigned char sr);
void WriteNBytes(unsigned int addr,unsigned char *buf,unsigned int len);
void ReadNBytes(unsigned int addr,unsigned char *buf,unsigned int len);
char WriteNBytesWithCheck(unsigned int addr,unsigned char *buf,unsigned int len);

void Flash_ReadID(unsigned char *buf);
void FlashReadJEDEC(unsigned char *buf);
void EraseWholeChip(void);
void EraseSinglePage(unsigned char addr);
void EraseFirstHalfChip(void);
void EraseSecondHalfChip(void);


#endif /* 24LC04_H_ */
