#ifndef BQ27510_I2C_H
#define BQ27510_I2C_H

#include "mbed.h"

#define ADDR_FUEL (0x55<<1)

#define SCL_FUEL	PA_8
#define SDA_FUEL	PC_9

unsigned char LM4F120_SWI2CMST_rxByte(void);
int LM4F120_SWI2CMST_writeBlock(unsigned int numBytes, unsigned char multi,
								void* TxData);
int LM4F120_SWI2CMST_readBlock(unsigned int numBytes, void* RxData);

#endif
