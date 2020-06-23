/* ========================================
 * 
 * This source code file contains macros and 
 * functions to interface with the SPI Master
 * of the PSoC.
 *
 * ========================================
*/


/* Project dependencies. */
#include "SPI_Interface.h"


/*******************************************************************************
* Function Name: SPI_IMU_Interface_tradeByte
********************************************************************************
*
* Summary:
*   This Function writes 1 byte (TX) on the MOSI line while reading (RX)
*   one byte from the slave on the MISO line (simultaneous TX/RX)
*
* Parameters:  
*   byte: 1-byte word to TX
*
* Return:
*   Received 1-byte word 
*
*******************************************************************************/
uint8_t SPI_IMU_Interface_tradeByte(uint8_t byte) 
{
	/* Enable the Slave */
	CS_IMU_Write(0);
	
	/* Load the TX Buffer */
	SPIM_IMU_WriteTxData(byte);
	
	/* Wait for TX */
    while( !(SPIM_IMU_ReadTxStatus() & SPIM_IMU_STS_BYTE_COMPLETE) );

	/* Read the RX Buffer */
	uint8_t data = SPIM_IMU_ReadRxData();
	
	/* Disable the Slave */
	CS_IMU_Write(1);
	
	return data;
}


/*******************************************************************************
* Function Name: SPI_IMU_Interface_ReadByte
********************************************************************************
*
* Summary:
*   This Function requests 1 byte from the SPI Slave.
*   First, it sends (TX) a 1-byte address/instruction to the Slave
*   that replies on the next clock cycle.
*   One byte from the slave is read on the MISO line 
*   on the next clock cycle, while transmetting a dummy byte (0x00)
*
* Parameters:  
*   byte: 1-byte address/instruction to TX
*
* Return:
*   Received 1-byte word 
*
*******************************************************************************/
uint8_t SPI_IMU_Interface_ReadByte(uint8_t byteTX) 
{
    /* Enable the Slave */
    CS_IMU_Write(0);
    	
    /* Load the TX Buffer */
    SPIM_IMU_WriteTxData(byteTX);
    	
    /* SPI Dummy Byte */
    SPIM_IMU_WriteTxData(SPI_DUMMY_BYTE);
    	
    /* Wait for TX/RX */
    while(!(SPIM_IMU_ReadTxStatus() & SPIM_IMU_STS_SPI_DONE));
    	
    /* Read the RX Buffer */
    SPIM_IMU_ReadRxData();
    uint8_t byteRX = SPIM_IMU_ReadRxData();
    	
    /* Disable the Slave */
    CS_IMU_Write(1);
    	
    return byteRX;
}


/*******************************************************************************
* Function Name: SPI_IMU_Interface_Multi_RW
********************************************************************************
*
* Summary:
*   This Function FIRST sends *bytesTX* bytes to the SPI Slave.
*   Then, it reads *bytesRX* bytes from the slave while 
*   transmitting dummy bytes (0x00).
*   Read/write operations are not simultaneous: data may be requested
*   to the slave with the TX operation and then read afterwards.
*
* Parameters:  
*   dataTX: Pointer to the input (TX) data array
*   bytesTX: Number of bytes to transmit
*   bytesRX: Number of bytes to receive
*
* Return:
*   Pointer to the output (RX) data array
*
*******************************************************************************/
void SPI_IMU_Interface_Multi_RW(uint8_t* dataTX, uint8_t bytesTX, uint8_t* dataRX, uint8_t bytesRX) {
    /* Enable the Slave */
    CS_IMU_Write(0);
        
    int8_t count = bytesTX, index = 0;
    	
    /* Transmit Data */
    while ( count > 0 ) 
    {
    	/* Load the TX buffer with Data*/
        SPIM_IMU_PutArray(&dataTX[index*SPI_TxBufferSize], (count > SPI_TxBufferSize ? SPI_TxBufferSize : count));
        /* Wait for TX */
        while( !(SPIM_IMU_ReadTxStatus() & SPIM_IMU_STS_SPI_DONE) );
        
        /* Update count */
        count -= SPI_TxBufferSize;
        index++;
    }
        
    /* Clear the RX Buffer */
    SPIM_IMU_ClearFIFO();
    SPIM_IMU_ClearRxBuffer();
        
    /* Init the Dummy TX Buffer */
    uint8_t dummyTX[SPI_RxBufferSize];
    memset(dummyTX, SPI_DUMMY_BYTE, SPI_RxBufferSize);
        
    /* Update count for RX */
    count = bytesRX;
    index = 0;
        
    /* Get the RX Data */
    while ( count > 0 ) 
    {
        /* Load the TX buffer with Dummy Bytes*/
        SPIM_IMU_PutArray(dummyTX, (count > SPI_TxBufferSize ? SPI_TxBufferSize : count));
        /* Wait for TX */
        while( !(SPIM_IMU_ReadTxStatus() & SPIM_IMU_STS_SPI_DONE) );
        /* Read the RX Buffer */
        for( uint8_t j = 0; j < (count > SPI_TxBufferSize ? SPI_TxBufferSize : count); j++ ) 
        {
            dataRX[j + index*SPI_RxBufferSize] = SPIM_IMU_ReadRxData();   
        }
        count -= SPI_RxBufferSize;
        index++;  
    }
        
    /* Disable the Slave */
    CS_IMU_Write(1);
    	
    /* Clear */
    SPIM_IMU_ClearFIFO();
    SPIM_IMU_ClearRxBuffer();
    SPIM_IMU_ClearTxBuffer();
}


/*******************************************************************************
* Function Name: SPI_EEPROM_Interface_tradeByte
********************************************************************************
*
* Summary:
*   This Function writes 1 byte (TX) on the MOSI line while reading (RX)
*   one byte from the slave on the MISO line (simultaneous TX/RX)
*
* Parameters:  
*   byte: 1-byte word to TX
*
* Return:
*   Received 1-byte word 
*
*******************************************************************************/
uint8_t SPI_EEPROM_Interface_tradeByte(uint8_t byte) 
{
	/* Enable the Slave */
	CS_EEPROM_Write(0);
	
	/* Load the TX Buffer */
	SPIM_EEPROM_WriteTxData(byte);
	
	/* Wait for TX */
    while( !(SPIM_EEPROM_ReadTxStatus() & SPIM_EEPROM_STS_BYTE_COMPLETE) );
    
	/* Read the RX Buffer */
	uint8_t data = SPIM_EEPROM_ReadRxData();
	
	/* Disable the Slave */
	CS_EEPROM_Write(1);
	
	return data;
}


/*******************************************************************************
* Function Name: SPI_EEPROM_Interface_ReadByte
********************************************************************************
*
* Summary:
*   This Function requests 1 byte from the SPI Slave.
*   First, it sends (TX) a 1-byte address/instruction to the Slave
*   that replies on the next clock cycle.
*   One byte from the slave is read on the MISO line 
*   on the next clock cycle, while transmetting a dummy byte (0x00)
*
* Parameters:  
*   byte: 1-byte address/instruction to TX
*
* Return:
*   Received 1-byte word 
*
*******************************************************************************/
uint8_t SPI_EEPROM_Interface_ReadByte(uint8_t byteTX) 
{
    /* Enable the Slave */
    CS_EEPROM_Write(0);
    	
    /* Load the TX Buffer */
    SPIM_EEPROM_WriteTxData(byteTX);
    	
    /* SPI Dummy Byte */
    SPIM_EEPROM_WriteTxData(SPI_DUMMY_BYTE);
    	
    /* Wait for TX/RX */
    while(!(SPIM_EEPROM_ReadTxStatus() & SPIM_EEPROM_STS_SPI_DONE));
    	
    /* Read the RX Buffer */
    SPIM_EEPROM_ReadRxData();
    uint8_t byteRX = SPIM_EEPROM_ReadRxData();
    	
    /* Disable the Slave */
    CS_EEPROM_Write(1);
    	
    return byteRX;
}


/*******************************************************************************
* Function Name: SPI_EEPROM_Interface_Multi_RW
********************************************************************************
*
* Summary:
*   This Function FIRST sends *bytesTX* bytes to the SPI Slave.
*   Then, it reads *bytesRX* bytes from the slave while 
*   transmitting dummy bytes (0x00).
*   Read/write operations are not simultaneous: data may be requested
*   to the slave with the TX operation and then read afterwards.
*
* Parameters:  
*   dataTX: Pointer to the input (TX) data array
*   bytesTX: Number of bytes to transmit
*   bytesRX: Number of bytes to receive
*
* Return:
*   Pointer to the output (RX) data array
*
*******************************************************************************/
void SPI_EEPROM_Interface_Multi_RW(uint8_t* dataTX, uint8_t bytesTX, uint8_t* dataRX, uint8_t bytesRX) 
{
    /* Enable the Slave */
    CS_EEPROM_Write(0);
        
    int8_t count = bytesTX, index = 0;
    	
    /* Transmit Data */
    while ( count > 0 ) 
    {
    	/* Load the TX buffer with Data*/
        SPIM_EEPROM_PutArray(&dataTX[index*SPI_TxBufferSize], (count > SPI_TxBufferSize ? SPI_TxBufferSize : count));
        /* Wait for TX */
        while( !(SPIM_EEPROM_ReadTxStatus() & SPIM_EEPROM_STS_SPI_DONE) );
        
        /* Update count */
        count -= SPI_TxBufferSize;
        index++;
    }
        
    /* Clear the RX Buffer */
    SPIM_EEPROM_ClearFIFO();
    SPIM_EEPROM_ClearRxBuffer();
        
    /* Init the Dummy TX Buffer */
    uint8_t dummyTX[SPI_RxBufferSize];
    memset(dummyTX, SPI_DUMMY_BYTE, SPI_RxBufferSize);
        
    /* Update count for RX */
    count = bytesRX;
    index = 0;
        
    /* Get the RX Data */
    while ( count > 0 ) 
    {
        /* Load the TX buffer with Dummy Bytes*/
        SPIM_EEPROM_PutArray(dummyTX, (count > SPI_TxBufferSize ? SPI_TxBufferSize : count));
        /* Wait for TX */
        while( !(SPIM_EEPROM_ReadTxStatus() & SPIM_EEPROM_STS_SPI_DONE) );
        /* Read the RX Buffer */
        for( uint8_t j = 0; j < (count > SPI_TxBufferSize ? SPI_TxBufferSize : count); j++ ) 
        {
            dataRX[j + index*SPI_RxBufferSize] = SPIM_EEPROM_ReadRxData();   
        }
        
        count -= SPI_RxBufferSize;
        index++;   
    }
        
    /* Disable the Slave */
    CS_EEPROM_Write(1);
    	
    /* Clear */
    SPIM_EEPROM_ClearFIFO();
    SPIM_EEPROM_ClearRxBuffer();
    SPIM_EEPROM_ClearTxBuffer();
}

/* [] END OF FILE */
