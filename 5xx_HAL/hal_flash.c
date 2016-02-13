//*******************************************************************************
//  Flash Library for flash memory controller of MSP430F5xxx/6xxx family
//    File: hal_flash.c
//
//    Texas Instruments
//
//    Version 1.0
//    11/24/09
//
//    V1.0  Initial Version
//*******************************************************************************

/* ***********************************************************
* THIS PROGRAM IS PROVIDED "AS IS". TI MAKES NO WARRANTIES OR
* REPRESENTATIONS, EITHER EXPRESS, IMPLIED OR STATUTORY,
* INCLUDING ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
* COMPLETENESS OF RESPONSES, RESULTS AND LACK OF NEGLIGENCE.
* TI DISCLAIMS ANY WARRANTY OF TITLE, QUIET ENJOYMENT, QUIET
* POSSESSION, AND NON-INFRINGEMENT OF ANY THIRD PARTY
* INTELLECTUAL PROPERTY RIGHTS WITH REGARD TO THE PROGRAM OR
* YOUR USE OF THE PROGRAM.
*
* IN NO EVENT SHALL TI BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
* CONSEQUENTIAL OR INDIRECT DAMAGES, HOWEVER CAUSED, ON ANY
* THEORY OF LIABILITY AND WHETHER OR NOT TI HAS BEEN ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGES, ARISING IN ANY WAY OUT
* OF THIS AGREEMENT, THE PROGRAM, OR YOUR USE OF THE PROGRAM.
* EXCLUDED DAMAGES INCLUDE, BUT ARE NOT LIMITED TO, COST OF
* REMOVAL OR REINSTALLATION, COMPUTER TIME, LABOR COSTS, LOSS
* OF GOODWILL, LOSS OF PROFITS, LOSS OF SAVINGS, OR LOSS OF
* USE OR INTERRUPTION OF BUSINESS. IN NO EVENT WILL TI'S
* AGGREGATE LIABILITY UNDER THIS AGREEMENT OR ARISING OUT OF
* YOUR USE OF THE PROGRAM EXCEED FIVE HUNDRED DOLLARS
* (U.S.$500).
*
* Unless otherwise stated, the Program written and copyrighted
* by Texas Instruments is distributed as "freeware".  You may,
* only under TI's copyright in the Program, use and modify the
* Program without any charge or restriction.  You may
* distribute to third parties, provided that you transfer a
* copy of this license to the third party and the third party
* agrees to these terms by its first use of the Program. You
* must reproduce the copyright notice and any other legend of
* ownership on each copy or partial copy, of the Program.
*
* You acknowledge and agree that the Program contains
* copyrighted material, trade secrets and other TI proprietary
* information and is protected by copyright laws,
* international copyright treaties, and trade secret laws, as
* well as other intellectual property laws.  To protect TI's
* rights in the Program, you agree not to decompile, reverse
* engineer, disassemble or otherwise translate any object code
* versions of the Program to a human-readable form.  You agree
* that in no event will you alter, remove or destroy any
* copyright notice included in the Program.  TI reserves all
* rights not specifically granted under this license. Except
* as specifically provided herein, nothing in this agreement
* shall be construed as conferring by implication, estoppel,
* or otherwise, upon you, any license or other right under any
* TI patents, copyrights or trade secrets.
*
* You may not use the Program in non-TI devices.
* ********************************************************* */

#include "hal_flash.h"
#include "msp430.h"

/* ***************************************************************************
 * Flash_SegmentErase
 *  Erase a single segment of the flash memory
 *
 * \param *Flash_ptr: Pointer into the flash segment to erase
 *
*/
void Flash_SegmentErase(uint16_t *Flash_ptr)
{
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY+ERASE;                      // Set Erase bit
  *Flash_ptr = 0;                           // Dummy write to erase Flash seg
  while (FCTL3 & BUSY);                     // test busy
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
}

/* ***************************************************************************
 * Flash_EraseCheck
 *  Erase Check of the flash memory
 *
 * \param *Flash_ptr: Pointer into the flash segment to erase
 * \param len:        give the len in word
 *
*/
uint8_t Flash_EraseCheck(uint16_t *Flash_ptr, uint16_t len)
{
  uint16_t i;
  for (i=0;i<len;i++)                        // was erasing successfully?
    if (*(Flash_ptr+i) != 0xFF){
      return (FLASH_STATUS_ERROR);
    }

  return (FLASH_STATUS_OK);
}


/* ***************************************************************************
 * FlashWrite_8
 *  Write data into the flash memory (Byte format)
 *
 * \param *Data_ptr:  Pointer to the Data to write
 * \param *Flash_ptr: Pointer into the flash to write data to
 * \param count:      number of data to write
 *
*/
void FlashWrite_8(uint8_t *Data_ptr, uint8_t *Flash_ptr, uint16_t count)
{
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY+WRT;                        // Enable byte/word write mode
  while (count > 0)
  {
    while (FCTL3 & BUSY);                   // test busy
    *Flash_ptr = *Data_ptr;                 // Write to Flash
    Flash_ptr++;
    Data_ptr++;
    count--;
  }

  FCTL1 = FWKEY;                            // Clear write bit
  FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
}

/* ***************************************************************************
 * FlashWrite_16
 *  Write data into the flash memory (Word format)
 *
 * \param *Data_ptr:  Pointer to the Data to write
 * \param *Flash_ptr: Pointer into the flash to write data to
 * \param count:      number of data to write
 *
*/
void FlashWrite_16(uint16_t *Data_ptr, uint16_t *Flash_ptr, uint16_t count)
{
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY+WRT;                        // Enable 16-bit write
  while (count > 0)
  {
    while (FCTL3 & BUSY);                   // test busy
    *Flash_ptr = *Data_ptr;                 // Write to Flash
    Flash_ptr ++;
    Data_ptr ++;
    count--;
  }

  FCTL1 = FWKEY;                            // Clear Erase bit
  FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
}


/* ***************************************************************************
 * FlashWrite_32
 *  Write data into the flash memory (Long format)
 *
 * \param *Data_ptr:  Pointer to the Data to write
 * \param *Flash_ptr: Pointer into the flash to write data to
 * \param count:      number of data to write
 *
*/
void FlashWrite_32(uint32_t *Data_ptr, uint32_t *Flash_ptr, uint16_t count)
{
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY+BLKWRT;                     // Enable long-word write
  while (count > 0)
  {
    while (FCTL3 & BUSY);                   // test busy
    *Flash_ptr = *Data_ptr;                 // Write to Flash
    Flash_ptr ++;
    Data_ptr ++;
    count--;
  }

  FCTL1 = FWKEY;                            // Clear Erase bit
  FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
}



/* ***************************************************************************
 * FlashMemoryFill_32
 *  Fill data into the flash memory (Long format)
 *
 * \param value:      Value with which to fill flash
 * \param *Flash_ptr: Pointer into the flash to write data to
 * \param count:      number of data to write (= byte * 4)
 *
*/
void FlashMemoryFill_32(uint32_t value, uint32_t *Flash_ptr, uint16_t count)
{
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY+BLKWRT;                     // Enable long-word write
  while (count > 0)
  {
    while (FCTL3 & BUSY);                   // test busy
    *Flash_ptr = value;                     // Write to Flash
    Flash_ptr ++;
    count--;
  }

  FCTL1 = FWKEY;                            // Clear Erase bit
  FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
}

