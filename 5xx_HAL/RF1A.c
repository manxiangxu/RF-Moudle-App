#include "RF1A.h"
#include "cc430F5137.h"

// *****************************************************************************
// @fn          Strobe
// @brief       Send a command strobe to the radio. Includes workaround for RF1A7
// @param       unsigned char strobe        The strobe command to be sent
// @return      unsigned char statusByte    The status byte that follows the strobe
// *****************************************************************************
unsigned char Strobe(unsigned char strobe)
{
  unsigned char statusByte = 0;
  unsigned int  gdo_state;
  
  // Check for valid strobe command 
  if((strobe == 0xBD) || ((strobe >= RF_SRES) && (strobe <= RF_SNOP)))
  {
    // Clear the Status read flag 
    RF1AIFCTL1 &= ~(RFSTATIFG);    
    
    // Wait for radio to be ready for next instruction
    while( !(RF1AIFCTL1 & RFINSTRIFG));
    
    // Write the strobe instruction
    if ((strobe > RF_SRES) && (strobe < RF_SNOP))
    {
      gdo_state = ReadSingleReg(IOCFG2);    // buffer IOCFG2 state
      WriteSingleReg(IOCFG2, 0x29);         // chip-ready to GDO2
      
      RF1AINSTRB = strobe; 
      if ( (RF1AIN&0x04)== 0x04 )           // chip at sleep mode
      {
        if ( (strobe == RF_SXOFF) || (strobe == RF_SPWD) || (strobe == RF_SWOR) ) { }
        else  	
        {
          while ((RF1AIN&0x04)== 0x04);     // chip-ready ?
          // Delay for ~810usec at 1.05MHz CPU clock, see erratum RF1A7
          __delay_cycles(710*8);	            
        }
      }
      WriteSingleReg(IOCFG2, gdo_state);    // restore IOCFG2 setting
    
      while( !(RF1AIFCTL1 & RFSTATIFG) );
    }
    else		                    // chip active mode (SRES)
    {	
      RF1AINSTRB = strobe; 	   
    }
    statusByte = RF1ASTATB;
  }
  return statusByte;
}

unsigned char Strobe_NoWait(unsigned char strobe)
{
  unsigned char statusByte = 0;
  unsigned int  gdo_state;
  
  // Check for valid strobe command 
  if((strobe == 0xBD) || ((strobe >= RF_SRES) && (strobe <= RF_SNOP)))
  {
    // Clear the Status read flag 
    RF1AIFCTL1 &= ~(RFSTATIFG);    
    
    // Wait for radio to be ready for next instruction
    while( !(RF1AIFCTL1 & RFINSTRIFG));
    
    // Write the strobe instruction
    if ((strobe > RF_SRES) && (strobe < RF_SNOP))
    {
      gdo_state = ReadSingleReg(IOCFG2);    // buffer IOCFG2 state
      WriteSingleReg(IOCFG2, 0x29);         // chip-ready to GDO2
      
      RF1AINSTRB = strobe; 
      if ( (RF1AIN&0x04)== 0x04 )           // chip at sleep mode
      {
        if ( (strobe == RF_SXOFF) || (strobe == RF_SPWD) || (strobe == RF_SWOR) ) { }
        else  	
        {
          while ((RF1AIN&0x04)== 0x04);     // chip-ready ?
          // Delay for ~810usec at 1.05MHz CPU clock, see erratum RF1A7
         // __delay_cycles(850);	            
        }
      }
      WriteSingleReg(IOCFG2, gdo_state);    // restore IOCFG2 setting
    
      while( !(RF1AIFCTL1 & RFSTATIFG) );
    }
    else		                    // chip active mode (SRES)
    {	
      RF1AINSTRB = strobe; 	   
    }
    statusByte = RF1ASTATB;
  }
  return statusByte;
}

// *****************************************************************************
// @fn          ReadSingleReg
// @brief       Read a single byte from the radio register
// @param       unsigned char addr      Target radio register address
// @return      unsigned char data_out  Value of byte that was read
// *****************************************************************************
unsigned char ReadSingleReg(unsigned char addr)
{
  unsigned char data_out;
  
  // Check for valid configuration register address, 0x3E refers to PATABLE 
  if ((addr <= 0x2E) || (addr == 0x3E))
    // Send address + Instruction + 1 dummy byte (auto-read)
    RF1AINSTR1B = (addr | RF_SNGLREGRD);    
  else
    // Send address + Instruction + 1 dummy byte (auto-read)
    RF1AINSTR1B = (addr | RF_STATREGRD);    
  
  while (!(RF1AIFCTL1 & RFDOUTIFG) );
  data_out = RF1ADOUT1B;                    // Read data and clears the RFDOUTIFG

  return data_out;
}

// *****************************************************************************
// @fn          WriteSingleReg
// @brief       Write a single byte to a radio register
// @param       unsigned char addr      Target radio register address
// @param       unsigned char value     Value to be written
// @return      none
// *****************************************************************************
void WriteSingleReg(unsigned char addr, unsigned char value)
{   
  while (!(RF1AIFCTL1 & RFINSTRIFG));       // Wait for the Radio to be ready for next instruction
  RF1AINSTRB = (addr | RF_REGWR);	    // Send address + Instruction

  RF1ADINB = value; 			    // Write data in 

  __no_operation(); 
}
        
// *****************************************************************************
// @fn          ReadBurstReg
// @brief       Read multiple bytes to the radio registers
// @param       unsigned char addr      Beginning address of burst read
// @param       unsigned char *buffer   Pointer to data table
// @param       unsigned char count     Number of bytes to be read
// @return      none
// *****************************************************************************
void ReadBurstReg(unsigned char addr, unsigned char *buffer, unsigned char count)
{
  unsigned int i;
  
  while (!(RF1AIFCTL1 & RFINSTRIFG));       // Wait for INSTRIFG
  RF1AINSTR1B = (addr | RF_REGRD);          // Send addr of first conf. reg. to be read 
                                            // ... and the burst-register read instruction
  for (i = 0; i < (count-1); i++)
  {
    while (!(RFDOUTIFG&RF1AIFCTL1));        // Wait for the Radio Core to update the RF1ADOUTB reg
    buffer[i] = RF1ADOUT1B;                 // Read DOUT from Radio Core + clears RFDOUTIFG
                                            // Also initiates auo-read for next DOUT byte
  }
  buffer[count-1] = RF1ADOUT0B;             // Store the last DOUT from Radio Core
}  

// *****************************************************************************
// @fn          WriteBurstReg
// @brief       Write multiple bytes to the radio registers
// @param       unsigned char addr      Beginning address of burst write
// @param       unsigned char *buffer   Pointer to data table
// @param       unsigned char count     Number of bytes to be written
// @return      none
// *****************************************************************************
void WriteBurstReg(unsigned char addr, unsigned char *buffer, unsigned char count)
{  
  unsigned char i;

  while (!(RF1AIFCTL1 & RFINSTRIFG));       // Wait for the Radio to be ready for next instruction
  RF1AINSTRW = ((addr | RF_REGWR)<<8 ) + buffer[0]; // Send address + Instruction

  for (i = 1; i < count; i++)
  {
    RF1ADINB = buffer[i];                   // Send data
    while (!(RFDINIFG & RF1AIFCTL1));       // Wait for TX to finish
  } 
  i = RF1ADOUTB;                            // Reset RFDOUTIFG flag which contains status byte
}

// *****************************************************************************
// @fn          ResetRadioCore
// @brief       Reset the radio core using RF_SRES command
// @param       none
// @return      none
// *****************************************************************************
void ResetRadioCore (void)
{
  Strobe(RF_SRES);                          // Reset the Radio Core
  Strobe(RF_SNOP);                          // Reset Radio Pointer
}

void SleepRadioCore(void)
{
	Strobe(RF_SXOFF);                         // sleep the Radio Core
    Strobe(RF_SNOP);                          // Reset Radio Pointer
}


// *****************************************************************************
// @fn          WritePATable
// @brief       Write data to power table
// @param       unsigned char value		Value to write
// @return      none
// *****************************************************************************
void WriteSinglePATable(unsigned char value)
{
  while( !(RF1AIFCTL1 & RFINSTRIFG));
  RF1AINSTRW = 0x3E00 + value;              // PA Table single write
  
  while( !(RF1AIFCTL1 & RFINSTRIFG));
  RF1AINSTRB = RF_SNOP;                     // reset PA_Table pointer
}

// *****************************************************************************
// @fn          WritePATable
// @brief       Write to multiple locations in power table 
// @param       unsigned char *buffer	Pointer to the table of values to be written 
// @param       unsigned char count	Number of values to be written
// @return      none
// *****************************************************************************
void WriteBurstPATable(unsigned char *buffer, unsigned char count)
{
  volatile char i = 0; 
  
  while( !(RF1AIFCTL1 & RFINSTRIFG));
  RF1AINSTRW = 0x7E00 + buffer[i];          // PA Table burst write   

  for (i = 1; i < count; i++)
  {
    RF1ADINB = buffer[i];                   // Send data
    while (!(RFDINIFG & RF1AIFCTL1));       // Wait for TX to finish
  } 
  i = RF1ADOUTB;                            // Reset RFDOUTIFG flag which contains status byte

  while( !(RF1AIFCTL1 & RFINSTRIFG));
  RF1AINSTRB = RF_SNOP;                     // reset PA Table pointer
}



//
// Rf settings for CC430
//
void SetRFEU1200(void)
{
	halRfWriteReg(IOCFG0,0x06);  //GDO0 Output Configuration
	halRfWriteReg(FIFOTHR,0x47); //RX FIFO and TX FIFO Thresholds
	halRfWriteReg(PKTCTRL0,0x05);//Packet Automation Control
	halRfWriteReg(FSCTRL1,0x06); //Frequency Synthesizer Control
	halRfWriteReg(FREQ2,0x21);   //Frequency Control Word, High Byte
	halRfWriteReg(FREQ1,0x62);   //Frequency Control Word, Middle Byte
	halRfWriteReg(FREQ0,0x76);   //Frequency Control Word, Low Byte
	halRfWriteReg(MDMCFG4,0xF5); //Modem Configuration
	halRfWriteReg(MDMCFG3,0x83); //Modem Configuration
	halRfWriteReg(MDMCFG2,0x13); //Modem Configuration
	halRfWriteReg(DEVIATN,0x15); //Modem Deviation Setting
	halRfWriteReg(MCSM0,0x10);   //Main Radio Control State Machine Configuration
	halRfWriteReg(FOCCFG,0x16);  //Frequency Offset Compensation Configuration
	halRfWriteReg(WORCTRL,0xFB); //Wake On Radio Control
	halRfWriteReg(FSCAL3,0xE9);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL2,0x2A);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL1,0x00);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL0,0x1F);  //Frequency Synthesizer Calibration
	halRfWriteReg(TEST2,0x81);   //Various Test Settings
	halRfWriteReg(TEST1,0x35);   //Various Test Settings
	halRfWriteReg(TEST0,0x09);   //Various Test Settings
}

void SetRFEU38K(void)
{
	halRfWriteReg(IOCFG0,0x06);  //GDO0 Output Configuration
	halRfWriteReg(FIFOTHR,0x47); //RX FIFO and TX FIFO Thresholds
	halRfWriteReg(PKTCTRL0,0x05);//Packet Automation Control
	halRfWriteReg(FSCTRL1,0x06); //Frequency Synthesizer Control
	halRfWriteReg(FREQ2,0x21);   //Frequency Control Word, High Byte
	halRfWriteReg(FREQ1,0x62);   //Frequency Control Word, Middle Byte
	halRfWriteReg(FREQ0,0x76);   //Frequency Control Word, Low Byte
	halRfWriteReg(MDMCFG4,0xCA); //Modem Configuration
	halRfWriteReg(MDMCFG3,0x83); //Modem Configuration
	halRfWriteReg(MDMCFG2,0x13); //Modem Configuration
	halRfWriteReg(DEVIATN,0x35); //Modem Deviation Setting

	halRfWriteReg(MCSM0,0x10);   //Main Radio Control State Machine Configuration
	halRfWriteReg(FOCCFG,0x16);  //Frequency Offset Compensation Configuration
	halRfWriteReg(AGCCTRL2,0x43);//AGC Control
	halRfWriteReg(WORCTRL,0xFB); //Wake On Radio Control
	halRfWriteReg(FSCAL3,0xE9);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL2,0x2A);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL1,0x00);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL0,0x1F);  //Frequency Synthesizer Calibration
	halRfWriteReg(TEST2,0x81);   //Various Test Settings
	halRfWriteReg(TEST1,0x35);   //Various Test Settings
	halRfWriteReg(TEST0,0x09);   //Various Test Settings
}
void SetRFUS38K(void)
{
	halRfWriteReg(IOCFG0,0x06);  //GDO0 Output Configuration
	halRfWriteReg(FIFOTHR,0x47); //RX FIFO and TX FIFO Thresholds
	halRfWriteReg(PKTLEN,0x1C);
	halRfWriteReg(PKTCTRL0,0x04);//Packet Automation Control
//	halRfWriteReg(PKTCTRL1,0x0c);//Packet Automation Control

	halRfWriteReg(FSCTRL1,0x06); //Frequency Synthesizer Control

	halRfWriteReg(FREQ2,0x23);   //Frequency Control Word, High Byte
	halRfWriteReg(FREQ1,0x31);   //Frequency Control Word, Middle Byte
	halRfWriteReg(FREQ0,0x3B);   //Frequency Control Word, Low Byte

	/*          //920M
	 halRfWriteReg(FREQ2,0x23);   //Frequency Control Word, High Byte
	 halRfWriteReg(FREQ1,0x62);   //Frequency Control Word, Middle Byte
	 halRfWriteReg(FREQ0,0x76);   //Frequency Control Word, Low Byte
	 */

	halRfWriteReg(MDMCFG4,0xCA); //Modem Configuration
	halRfWriteReg(MDMCFG3,0x83); //Modem Configuration
	halRfWriteReg(MDMCFG2,0x13); //Modem Configuration
	halRfWriteReg(DEVIATN,0x35); //Modem Deviation Setting

	halRfWriteReg(MCSM1,0x3F);   //automatic into receive mode when transmit done or receive complete
	halRfWriteReg(MCSM0,0x10);   //Main Radio Control State Machine Configuration0
	halRfWriteReg(FOCCFG,0x16);  //Frequency Offset Compensation Configuration
	halRfWriteReg(AGCCTRL2,0x43);//AGC Control
	halRfWriteReg(WORCTRL,0xFB); //Wake On Radio Control
	halRfWriteReg(FSCAL3,0xE9);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL2,0x2A);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL1,0x00);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL0,0x1F);  //Frequency Synthesizer Calibration

	halRfWriteReg(TEST2,0x81);   //Various Test Settings
	halRfWriteReg(TEST1,0x35);   //Various Test Settings
	halRfWriteReg(TEST0,0x09);   //Various Test Settings
}

void SetRF927M250K(void)
{
	halRfWriteReg(IOCFG0,0x06);  //GDO0 Output Configuration
	halRfWriteReg(PKTCTRL0,0x05);//Packet Automation Control
	halRfWriteReg(FSCTRL1,0x0C); //Frequency Synthesizer Control
	halRfWriteReg(FREQ2,0x23);   //Frequency Control Word, High Byte
	halRfWriteReg(FREQ1,0xA7);   //Frequency Control Word, Middle Byte
	halRfWriteReg(FREQ0,0x62);   //Frequency Control Word, Low Byte
	halRfWriteReg(MDMCFG4,0x2D); //Modem Configuration
	halRfWriteReg(MDMCFG3,0x3B); //Modem Configuration
	halRfWriteReg(MDMCFG2,0x93); //Modem Configuration
	halRfWriteReg(DEVIATN,0x62); //Modem Deviation Setting
	halRfWriteReg(MCSM0,0x10);   //Main Radio Control State Machine Configuration
	halRfWriteReg(FOCCFG,0x1D);  //Frequency Offset Compensation Configuration
	halRfWriteReg(BSCFG,0x1C);   //Bit Synchronization Configuration
	halRfWriteReg(AGCCTRL2,0xC7);//AGC Control
	halRfWriteReg(AGCCTRL1,0x00);//AGC Control
	halRfWriteReg(AGCCTRL0,0xB0);//AGC Control
	halRfWriteReg(WORCTRL,0xFB); //Wake On Radio Control
	halRfWriteReg(FREND1,0xB6);  //Front End RX Configuration
	halRfWriteReg(FSCAL3,0xEA);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL2,0x2A);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL1,0x00);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL0,0x1F);  //Frequency Synthesizer Calibration
	halRfWriteReg(TEST0,0x09);   //Various Test Settings
}


void SetRF903M38K(void)
{
	halRfWriteReg(IOCFG0,0x06);  //GDO0 Output Configuration
	halRfWriteReg(FIFOTHR,0x47); //RX FIFO and TX FIFO Thresholds
	halRfWriteReg(PKTCTRL0,0x05);//Packet Automation Control
	halRfWriteReg(FSCTRL1,0x06); //Frequency Synthesizer Control
	halRfWriteReg(FREQ2,0x22);   //Frequency Control Word, High Byte
	halRfWriteReg(FREQ1,0xBB);   //Frequency Control Word, Middle Byte
	halRfWriteReg(FREQ0,0x13);   //Frequency Control Word, Low Byte
	halRfWriteReg(MDMCFG4,0xCA); //Modem Configuration
	halRfWriteReg(MDMCFG3,0x83); //Modem Configuration
	halRfWriteReg(MDMCFG2,0x13); //Modem Configuration
	halRfWriteReg(DEVIATN,0x35); //Modem Deviation Setting
	halRfWriteReg(MCSM0,0x10);   //Main Radio Control State Machine Configuration
	halRfWriteReg(FOCCFG,0x16);  //Frequency Offset Compensation Configuration
	halRfWriteReg(AGCCTRL2,0x43);//AGC Control
	halRfWriteReg(WORCTRL,0xFB); //Wake On Radio Control
	halRfWriteReg(FSCAL3,0xE9);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL2,0x2A);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL1,0x00);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL0,0x1F);  //Frequency Synthesizer Calibration
	halRfWriteReg(TEST2,0x81);   //Various Test Settings
	halRfWriteReg(TEST1,0x35);   //Various Test Settings
	halRfWriteReg(TEST0,0x09);   //Various Test Settings
}


void SetRF433M38K(void)
{
	halRfWriteReg(IOCFG0,0x06);  //GDO0 Output Configuration
	halRfWriteReg(FIFOTHR,0x47); //RX FIFO and TX FIFO Thresholds
	halRfWriteReg(PKTCTRL0,0x05);//Packet Automation Control
	halRfWriteReg(FSCTRL1,0x06); //Frequency Synthesizer Control
	halRfWriteReg(FREQ2,0x10);   //Frequency Control Word, High Byte
	halRfWriteReg(FREQ1,0xA7);   //Frequency Control Word, Middle Byte
	halRfWriteReg(FREQ0,0x62);   //Frequency Control Word, Low Byte
	halRfWriteReg(MDMCFG4,0xCA); //Modem Configuration
	halRfWriteReg(MDMCFG3,0x83); //Modem Configuration
	halRfWriteReg(MDMCFG2,0x13); //Modem Configuration
	halRfWriteReg(DEVIATN,0x35); //Modem Deviation Setting
	halRfWriteReg(MCSM0,0x10);   //Main Radio Control State Machine Configuration
	halRfWriteReg(FOCCFG,0x16);  //Frequency Offset Compensation Configuration
	halRfWriteReg(AGCCTRL2,0x43);//AGC Control
	halRfWriteReg(WORCTRL,0xFB); //Wake On Radio Control
	halRfWriteReg(FSCAL3,0xE9);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL2,0x2A);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL1,0x00);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL0,0x1F);  //Frequency Synthesizer Calibration
	halRfWriteReg(TEST2,0x81);   //Various Test Settings
	halRfWriteReg(TEST1,0x35);   //Various Test Settings
	halRfWriteReg(TEST0,0x09);   //Various Test Settings

}

void SetRF433M1200(void)
{
	//
	// Rf settings for CC430
	//
	halRfWriteReg(IOCFG0,0x06);  //GDO0 Output Configuration
	halRfWriteReg(FIFOTHR,0x47); //RX FIFO and TX FIFO Thresholds
	halRfWriteReg(PKTCTRL0,0x05);//Packet Automation Control
	halRfWriteReg(FSCTRL1,0x06); //Frequency Synthesizer Control
	halRfWriteReg(FREQ2,0x10);   //Frequency Control Word, High Byte
	halRfWriteReg(FREQ1,0xA7);   //Frequency Control Word, Middle Byte
	halRfWriteReg(FREQ0,0x62);   //Frequency Control Word, Low Byte
	halRfWriteReg(MDMCFG4,0xF5); //Modem Configuration
	halRfWriteReg(MDMCFG3,0x83); //Modem Configuration
	halRfWriteReg(MDMCFG2,0x13); //Modem Configuration
	halRfWriteReg(DEVIATN,0x15); //Modem Deviation Setting
	halRfWriteReg(MCSM0,0x10);   //Main Radio Control State Machine Configuration
	halRfWriteReg(FOCCFG,0x16);  //Frequency Offset Compensation Configuration
	halRfWriteReg(WORCTRL,0xFB); //Wake On Radio Control
	halRfWriteReg(FSCAL3,0xE9);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL2,0x2A);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL1,0x00);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL0,0x1F);  //Frequency Synthesizer Calibration
	halRfWriteReg(TEST2,0x81);   //Various Test Settings
	halRfWriteReg(TEST1,0x35);   //Various Test Settings
	halRfWriteReg(TEST0,0x09);   //Various Test Settings
}


void SetRF432M250K(void)
{
	halRfWriteReg(IOCFG0,0x06);  //GDO0 Output Configuration
	halRfWriteReg(PKTCTRL0,0x05);//Packet Automation Control
	halRfWriteReg(FSCTRL1,0x0C); //Frequency Synthesizer Control
	halRfWriteReg(FREQ2,0x10);   //Frequency Control Word, High Byte
	halRfWriteReg(FREQ1,0x9D);   //Frequency Control Word, Middle Byte
	halRfWriteReg(FREQ0,0x89);   //Frequency Control Word, Low Byte
	halRfWriteReg(MDMCFG4,0x2D); //Modem Configuration
	halRfWriteReg(MDMCFG3,0x3B); //Modem Configuration
	halRfWriteReg(MDMCFG2,0x13); //Modem Configuration
	halRfWriteReg(DEVIATN,0x62); //Modem Deviation Setting
	halRfWriteReg(MCSM0,0x10);   //Main Radio Control State Machine Configuration
	halRfWriteReg(FOCCFG,0x1D);  //Frequency Offset Compensation Configuration
	halRfWriteReg(BSCFG,0x1C);   //Bit Synchronization Configuration
	halRfWriteReg(AGCCTRL2,0xC7);//AGC Control
	halRfWriteReg(AGCCTRL1,0x00);//AGC Control
	halRfWriteReg(AGCCTRL0,0xB0);//AGC Control
	halRfWriteReg(WORCTRL,0xFB); //Wake On Radio Control
	halRfWriteReg(FREND1,0xB6);  //Front End RX Configuration
	halRfWriteReg(FSCAL3,0xEA);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL2,0x2A);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL1,0x00);  //Frequency Synthesizer Calibration
	halRfWriteReg(FSCAL0,0x1F);  //Frequency Synthesizer Calibration
	halRfWriteReg(TEST0,0x09);   //Various Test Settings
}

void halRfWriteReg(unsigned char address, unsigned char data)
{
	  while (!(RF1AIFCTL1 & RFINSTRIFG));       // Wait for the Radio to be ready for next instruction
	  RF1AINSTRB = (address | RF_REGWR);	    // Send address + Instruction

	  RF1ADINB = data; 			    // Write data in

	  __no_operation();
}
