//*******************************************************************************
//  Flash Library for flash memory controller of MSP430F5xxx/6xxx family
//    File: hal_flash.h
//
//    Texas Instruments
//
//    Version 1.0
//    11/24/09
//
//    V1.0  Initial Version
//*******************************************************************************

#ifndef __hal_FLASH
#define __hal_FLASH

#include <stdint.h>

//************************************************************************
// Defines
//************************************************************************

#define FLASH_STATUS_OK     0
#define FLASH_STATUS_ERROR  1

/* ***************************************************************************
 * Flash_SegmentErase
 *  Erase a single segment of the flash memory
 *
 * \param *Flash_ptr: Pointer into the flash segment to erase
 *
*/
extern void Flash_SegmentErase(uint16_t *Flash_ptr);


/* ***************************************************************************
 * Flash_EraseCheck
 *  Erase Check of the flash memory
 *
 * \param *Flash_ptr: Pointer into the flash segment to erase
 * \param len:        give the len in word
 *
*/
extern uint8_t Flash_EraseCheck(uint16_t *Flash_ptr, uint16_t len);


/* ***************************************************************************
 * FlashWrite_8
 *  Write data into the flash memory (Byte format)
 *
 * \param *Data_ptr:  Pointer to the Data to write
 * \param *Flash_ptr: Pointer into the flash to write data to
 * \param count:      number of data to write
 *
*/
extern void FlashWrite_8(uint8_t *Data_ptr, uint8_t *Flash_ptr, uint16_t count);


/* ***************************************************************************
 * FlashWrite_16
 *  Write data into the flash memory (Word format)
 *
 * \param *Data_ptr:  Pointer to the Data to write
 * \param *Flash_ptr: Pointer into the flash to write data to
 * \param count:      number of data to write
 *
*/
extern void FlashWrite_16(uint16_t *Data_ptr, uint16_t *Flash_ptr, uint16_t count);


/* ***************************************************************************
 * FlashWrite_32
 *  Write data into the flash memory (Long format)
 *
 * \param *Data_ptr:  Pointer to the Data to write
 * \param *Flash_ptr: Pointer into the flash to write data to
 * \param count:      number of data to write
 *
*/
extern void FlashWrite_32(uint32_t *Data_ptr, uint32_t *Flash_ptr, uint16_t count);


/* ***************************************************************************
 * FlashMemoryFill_32
 *  Fill data into the flash memory (Long format)
 *
 * \param value:      Pointer to the Data to write
 * \param *Flash_ptr: Pointer into the flash to write data to
 * \param count:      number of data to write (= byte * 4)
 *
*/
extern void FlashMemoryFill_32(uint32_t value, uint32_t *Flash_ptr, uint16_t count);


#endif /* __hal_FLASH */

