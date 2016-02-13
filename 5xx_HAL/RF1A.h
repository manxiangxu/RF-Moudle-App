
/* ------------------------------------------------------------------------------------------------
 *                                          Defines
 * ------------------------------------------------------------------------------------------------
 */

void ResetRadioCore (void);
unsigned char Strobe(unsigned char strobe);
unsigned char Strobe_NoWait(unsigned char strobe);
void WriteSingleReg(unsigned char addr, unsigned char value);
void WriteBurstReg(unsigned char addr, unsigned char *buffer, unsigned char count);
unsigned char ReadSingleReg(unsigned char addr);
void ReadBurstReg(unsigned char addr, unsigned char *buffer, unsigned char count);
void WriteSinglePATable(unsigned char value);
void WriteBurstPATable(unsigned char *buffer, unsigned char count); 

void SleepRadioCore(void);

void halRfWriteReg(unsigned char address, unsigned char data);
void SetRFEU1200(void);
void SetRFEU38K(void);
void SetRFEU200K(void);
void SetRFUS38K(void);
void SetRF927M250K(void);
void SetRF903M38K(void);
void SetRF433M38K(void);
void SetRF433M1200(void);
