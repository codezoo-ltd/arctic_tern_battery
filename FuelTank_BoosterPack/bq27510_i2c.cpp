/*
******************************************************************************
*                                  INCLUDE FILES
******************************************************************************
*/
#include "bq27510_i2c.h"

extern I2C i2c;
/*
******************************************************************************
*                                  FUNCTION DEFINITIONS
******************************************************************************
*/

/**
  * @brief	transmit multiple bytes
  * @param 	unsigned int numBytes,
			unsigned char multi,
			void* TxData
  * @retval  0 : writeBlock successfully
  *         -1 : writeBlock failed, no Ack respond
  */
int LM4F120_SWI2CMST_writeBlock(unsigned int numBytes, unsigned char multi,
								void* TxData)
{
	char *temp;
	int ret;
	//
	// Initialize array pointer
	//
	temp = (char *)TxData;

	ret = i2c.write( (ADDR_FUEL|0), temp, numBytes );

	if(ret)
		return -1;
	//
	// Need STOP condition? Yes, send STOP condition
	// 
	if (multi == 0)                           
	{
		i2c.stop();
	}
	
	wait_ms(1);

	return 0;
}

/**
  * @brief	receive multiple bytes
  * @param 	unsigned int numBytes,
			void* RxData
  * @retval  0 : readBlock successfully
  *         -1 : readBlock failed, no Ack respond or all data is 0
  */
int LM4F120_SWI2CMST_readBlock(unsigned int numBytes, void* RxData)
{
	char* temp;
	int ret,i,j,valid_data;
	//
	// Initialize array pointer
	//
	temp = (char *)RxData;           

	ret = i2c.read( (ADDR_FUEL|1), temp, numBytes );

	if(ret)
		return -1;

	i2c.stop();

    //
    // If the battery capacity is too low
    // We cannot read any data from bq27510
    // The SDA line is always 0
    // In this case, we return -1 to let main program show "charge battery"
    //
    temp = (char *)RxData;
    for (i = 0; i < numBytes-1; i++) {
    	for(j = 0; j < 8; j++) {
    		if(temp[j] != 0) {
    			valid_data = 1;
    		}
    	}
    	temp++;
    }

    if (valid_data)
    	return 0;
    else
    	return -1;
}
