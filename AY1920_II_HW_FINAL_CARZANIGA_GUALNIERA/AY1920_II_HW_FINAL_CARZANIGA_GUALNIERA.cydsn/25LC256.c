/* ========================================
 *
 * This file contains a variety of utility
 * functions to handle EEPROM memory reading
 * and writing. On top of basic functions,
 * high-level functions to write and read
 * structured data (such as log messages)
 * are also defined, which incorporates
 * methods for error checking to prevent
 * future usage issues.
 *
 * ========================================
*/


/* Project dependecies. */
#include "25LC256.h"


/*******************************************************************************
* Function Name: EEPROM_readStatus
********************************************************************************
*
* Summary:
*   Read content of 25LC256 EEPROM status register via SPI.
*
* Parameters:  
*   None
*
* Return:
*   8-bit status register.
*
*******************************************************************************/
uint8_t EEPROM_readStatus(void) 
{
	return SPI_EEPROM_Interface_ReadByte(SPI_EEPROM_RDSR);
}


/*******************************************************************************
* Function Name: EEPROM_writeEnable
********************************************************************************
*
* Summary:
*   Set enable bit inside via SPI of 25LC256 EEPROM control register to allow 
*   memory write operations of any kind.
*
* Parameters:  
*   None
*
* Return:
*   None.
*
*******************************************************************************/
void EEPROM_writeEnable(void) 
{
	/* Send 1-byte Instruction */
	SPI_EEPROM_Interface_tradeByte(SPI_EEPROM_WREN);
}


/*******************************************************************************
* Function Name: EEPROM_readByte
********************************************************************************
*
* Summary:
*   Read 1 byte of memory from 25LC256 EEPROM via SPI.
*
* Parameters:  
*   16-bit EEPROM address.
*
* Return:
*   8-bit EEPROM memory data.
*
*******************************************************************************/
uint8_t EEPROM_readByte(uint16_t addr) 
{
	/* Prepare the TX data packet: instruction + address */
	uint8_t dataTX[3] = {SPI_EEPROM_READ, ((addr & 0xFF00) >> 8 ), (addr & 0x00FF)};
	
	/* Prepare the RX byte */
	uint8_t dataRX = 0;
	
	/* Read 1 byte from addr */
	SPI_EEPROM_Interface_Multi_RW(dataTX, 3, &dataRX, 1);
    
    return dataRX;
}


/*******************************************************************************
* Function Name: EEPROM_writeByte
********************************************************************************
*
* Summary:
*   Write 1 byte of 25LC256 EEPROM memory via SPI.
*
* Parameters:  
*   16-bit EEPROM address, 8-bit data content.
*
* Return:
*   None.
*
*******************************************************************************/
void EEPROM_writeByte(uint16_t addr, uint8_t dataByte) 
{
    /* Enable WRITE operations */
    EEPROM_writeEnable();
	
	/* Prepare the TX packet */
    uint8_t dataTX[4] = {SPI_EEPROM_WRITE, ((addr & 0xFF00) >> 8), (addr & 0x00FF), dataByte};
	/* Nothing to RX... */
	uint8_t temp = 0;
	
	/* Write 1 byte to addr */
	SPI_EEPROM_Interface_Multi_RW(dataTX, 4, &temp, 0);
}


/*******************************************************************************
* Function Name: EEPROM_readPage
********************************************************************************
*
* Summary:
*   Read up to 64 bytes of 25LC256 EEPROM memory and place data in a pre-allocated
*   data buffer provided.
*
* Parameters:  
*   16-bit EEPROM address, 8-bit data pointer, number of bytes to read.
*
* Return:
*   None.
*
*******************************************************************************/
void EEPROM_readPage(uint16_t addr, uint8_t* dataRX, uint8_t nBytes) 
{
	/* Prepare the TX data packet: instruction + address */
	uint8_t dataTX[3] = {SPI_EEPROM_READ, ((addr & 0xFF00) >> 8), (addr & 0x00FF)};
	
	/* Read the nBytes */
	SPI_EEPROM_Interface_Multi_RW(dataTX, 3, dataRX, nBytes);
}


/*******************************************************************************
* Function Name: EEPROM_writePage
********************************************************************************
*
* Summary:
*   Write up to 64 bytes of 25LC256 EEPROM memory from a pre-allocated data
*   buffer provided.
*
* Parameters:  
*   16-bit EEPROM address, 8-bit data pointer, number of bytes to write.
*
* Return:
*   None.
*
* Side effects:
*   All writing attempt across different EEPROM pages will result in overwrite
*   of previous data from top to bottom of current page. Each page is defined
*   between a begin address multiple of 64 and an end address: begin_addr+64-1. 
*
*******************************************************************************/
void EEPROM_writePage(uint16_t addr, uint8_t* data, uint8_t nBytes) 
{
    /* Enable WRITE operations */
    EEPROM_writeEnable();
	
    CyDelayUs(1);
    
	/* Prepare the TX packet of size nBytes+3 
       [ Write Instruction - Address MSB - Address LSB - +++data+++ ]
    */
	uint8_t dataTX[3+nBytes];
    dataTX[0] = SPI_EEPROM_WRITE;
    dataTX[1] = (addr & 0xFF00) >> 8;
    dataTX[2] = addr & 0x00FF;
    /* Copy the input data in the memory */
	memcpy(&dataTX[3], data, nBytes);
	
	/* Nothing to RX: point to a dummy variable */
	uint8_t temp = 0;
	
	SPI_EEPROM_Interface_Multi_RW(dataTX, 3+nBytes, &temp, 0);
}

/*******************************************************************************
* Function Name: EEPROM_waitForWriteComplete
********************************************************************************
*
* Summary:
*   Blocking function thats wait until previous write instruction has been
*   completed by reading WIP bit inside EEPROM status register.
*
* Parameters:  
*   None.
*
* Return:
*   None.
*
*******************************************************************************/
void EEPROM_waitForWriteComplete(void) 
{
    while( EEPROM_readStatus() & SPI_EEPROM_WRITE_IN_PROGRESS );
}


/*******************************************************************************
* Function Name: EEPROM_saveStartStopState
********************************************************************************
*
* Summary:
*   Save start bit (1) bit or stop bit (0) inside EEPROM memory.
*
* Parameters:  
*   State of start/stop flag.
*
* Return:
*   None.
*
*******************************************************************************/
void EEPROM_saveStartStopState(uint8_t state)
{
    // Read current register content
    uint8_t ctrl_reg = EEPROM_readByte(CTRL_REG_PSOC_STATUS);
    
    // Check state and setup mask
    if (state == 1)
    {
        ctrl_reg |= CTRL_REG_PSOC_SET_START;
    }
    else
    {
        ctrl_reg &= ~CTRL_REG_PSOC_SET_START;
    }
    
    // Overwrite register content
    EEPROM_writeByte(CTRL_REG_PSOC_STATUS, ctrl_reg);
    EEPROM_waitForWriteComplete();
    
    // Set reset bit to zero after first write op
    if (ctrl_reg & CTRL_REG_PSOC_SET_RESET_FLAG)
    {
        EEPROM_saveResetFlag(0);
    }
}


/*******************************************************************************
* Function Name: EEPROM_retrieveStartStopState
********************************************************************************
*
* Summary:
*   Retrieve start bit (1) bit or stop bit (0) from EEPROM memory.
*
* Parameters:  
*   None.
*
* Return:
*   State of start/stop flag.
*
*******************************************************************************/
uint8_t EEPROM_retrieveStartStopState(void)
{
    // Read current register content
    uint8_t ctrl_reg = EEPROM_readByte(CTRL_REG_PSOC_STATUS);
    
    // Retrieve bit
    ctrl_reg &= CTRL_REG_PSOC_SET_START;
    ctrl_reg = (ctrl_reg >> CTRL_REG_PSOC_START_STOP_SHIFT) & 0x01;
    return ctrl_reg;
}


/*******************************************************************************
* Function Name: EEPROM_saveConfigFlag
********************************************************************************
*
* Summary:
*   Save configuration mode flag inside EEPROM memory.
*
* Parameters:  
*   State of configuration flag.
*
* Return:
*   None.
*
*******************************************************************************/
void EEPROM_saveConfigFlag(uint8_t flag)
{
    // Read current register content
    uint8_t ctrl_reg = EEPROM_readByte(CTRL_REG_PSOC_STATUS);
    
    // Check state and setup mask
    if (flag == 1)
    {
        ctrl_reg |= CTRL_REG_PSOC_SET_CONFIG;
    }
    else
    {
        ctrl_reg &= ~CTRL_REG_PSOC_SET_CONFIG;
    }
    
    // Overwrite register content
    EEPROM_writeByte(CTRL_REG_PSOC_STATUS, ctrl_reg);
    EEPROM_waitForWriteComplete();
    
    // Set reset bit to zero after first write op
    if (ctrl_reg & CTRL_REG_PSOC_SET_RESET_FLAG)
    {
        EEPROM_saveResetFlag(0);
    }
}


/*******************************************************************************
* Function Name: EEPROM_retrieveConfigFlag
********************************************************************************
*
* Summary:
*   Retrieve configuration mode flag from EEPROM memory.
*
* Parameters:  
*   None.
*
* Return:
*   State of configuration flag.
*
*******************************************************************************/
uint8_t EEPROM_retrieveConfigFlag(void)
{
    // Read current register content
    uint8_t ctrl_reg =  EEPROM_readByte(CTRL_REG_PSOC_STATUS);
    
    // Retrieve bit
    ctrl_reg &= CTRL_REG_PSOC_SET_CONFIG;
    ctrl_reg = (ctrl_reg >> CTRL_REG_PSOC_CONFIG_MODE_SHIFT) & 0x01;
    return ctrl_reg;
}


/*******************************************************************************
* Function Name: EEPROM_saveSendFlag
********************************************************************************
*
* Summary:
*   Save send flag state inside EEPROM memory.
*
* Parameters:  
*   State of send flag.
*
* Return:
*   None.
*
*******************************************************************************/
void EEPROM_saveSendFlag(uint8_t flag)
{
    // Read current register content
    uint8_t ctrl_reg = EEPROM_readByte(CTRL_REG_PSOC_STATUS); 
    
    // Check state and setup mask
    if (flag == 1)
    {
        ctrl_reg |= CTRL_REG_PSOC_SET_SEND_FLAG;
    }
    else
    {
        ctrl_reg &= ~CTRL_REG_PSOC_SET_SEND_FLAG;
    }
    
    // Overwrite register content
    EEPROM_writeByte(CTRL_REG_PSOC_STATUS, ctrl_reg);
    EEPROM_waitForWriteComplete();
    
    // Set reset bit to zero after first write op
    if (ctrl_reg & CTRL_REG_PSOC_SET_RESET_FLAG)
    {
        EEPROM_saveResetFlag(0);
    }
}


/*******************************************************************************
* Function Name: EEPROM_retrieveSendFlag
********************************************************************************
*
* Summary:
*   Retrieve send flag state from EEPROM memory.
*
* Parameters:  
*   None.
*
* Return:
*   State of send flag.
*
*******************************************************************************/
uint8_t EEPROM_retrieveSendFlag(void)
{
    // Read current register content
    uint8_t ctrl_reg =  EEPROM_readByte(CTRL_REG_PSOC_STATUS);
    
    // Retrieve bit
    ctrl_reg &= CTRL_REG_PSOC_SET_SEND_FLAG;
    ctrl_reg = (ctrl_reg >> CTRL_REG_PSOC_SEND_FLAG_SHIFT) & 0x01;
    return ctrl_reg;
}


/*******************************************************************************
* Function Name: EEPROM_saveResetFlag
********************************************************************************
*
* Summary:
*   Save reset flag state inside EEPROM memory.
*
* Parameters:  
*   State of reset flag.
*
* Return:
*   None.
*
*******************************************************************************/
void EEPROM_saveResetFlag(uint8_t flag)
{
    // Read current register content
    uint8_t ctrl_reg = EEPROM_readByte(CTRL_REG_PSOC_STATUS);
    
    // Check state and setup mask
    if (flag == 1)
    {
        ctrl_reg |= CTRL_REG_PSOC_SET_RESET_FLAG;
    }
    else
    {
        ctrl_reg &= ~CTRL_REG_PSOC_SET_RESET_FLAG;
    }
    
    // Overwrite register content
    EEPROM_writeByte(CTRL_REG_PSOC_STATUS, ctrl_reg);
    EEPROM_waitForWriteComplete();
}


/*******************************************************************************
* Function Name: EEPROM_retrieveResetFlag
********************************************************************************
*
* Summary:
*   Retrieve reset flag state from EEPROM memory.
*
* Parameters:  
*   None.
*
* Return:
*   State of reset flag.
*
*******************************************************************************/
uint8_t EEPROM_retrieveResetFlag(void)
{
    // Read current register content
    uint8_t ctrl_reg =  EEPROM_readByte(CTRL_REG_PSOC_STATUS);
    
    // Retrieve bit
    ctrl_reg &= CTRL_REG_PSOC_SET_RESET_FLAG;
    ctrl_reg = (ctrl_reg >> CTRL_REG_PSOC_RESET_FLAG_SHIFT) & 0x01;
    return ctrl_reg;
}


/*******************************************************************************
* Function Name: EEPROM_retrieveLogPages
********************************************************************************
*
* Summary:
*   Read number of written log pages inside EEPROM memory. 
*
* Parameters:  
*   None.
*
* Return:
*   Number of written log pages (max: 511).
*
*******************************************************************************/
uint16_t EEPROM_retrieveLogPages(void)
{
    // Read current registers and increment counter
    uint16_t reg_count = EEPROM_readByte(CTRL_REG_LOG_PAGES_HIGH) << 8;
    reg_count |= EEPROM_readByte(CTRL_REG_LOG_PAGES_LOW);
    
    // Return log counter
    return reg_count;
}


/*******************************************************************************
* Function Name: EEPROM_retrieveLogCount
********************************************************************************
*
* Summary:
*   Compute number of logs currently store inside EEPROM memory. 
*
* Parameters:  
*   None.
*
* Return:
*   Number of logs (max: 102).
*
*******************************************************************************/
uint8_t EEPROM_retrieveLogCount(void)
{
    // Read log pages count
    uint16_t page_count = EEPROM_retrieveLogPages();
    
    // Return log count
    return (uint8_t)(page_count / LOG_PAGES_PER_EVENT);
}


/*******************************************************************************
* Function Name: EEPROM_incrementLogCounter
********************************************************************************
*
* Summary:
*   Increment by 1 number of logs inside the log counter of the EEPROM. 
*
* Parameters:  
*   None.
*
* Return:
*   None.
*
*******************************************************************************/
void EEPROM_incrementLogCounter(void)
{
    // Read number of already written pages
    uint16_t reg_count = EEPROM_retrieveLogPages();
    
    // Avoid variable overflow
    if (reg_count < SPI_EEPROM_PAGE_COUNT)
    {
        // Increment counter
        reg_count++;
    
        // Store data in buffer
        uint8_t buffer[2];
        buffer[0] = reg_count & 0x00FF;
        buffer[1] = (reg_count >> 8) & 0x00FF;
        
        // Overwrite registers
        EEPROM_writePage(CTRL_REG_LOG_PAGES_LOW, buffer, 2);
        EEPROM_waitForWriteComplete();
    }
}


/*******************************************************************************
* Function Name: EEPROM_storeLogData
********************************************************************************
*
* Summary:
*   Store 64 bytes buffer of log data inside the first available page of EEPROM 
*   memory. 
*
* Parameters:  
*   Log buffer pointer.
*
* Return:
*   None.
*
*******************************************************************************/
void EEPROM_storeLogData(uint8_t* dataPtr)
{
    // Check how many log pages are currently stored
    uint16_t page_count = EEPROM_retrieveLogPages();

    // Compute first available address 
    uint16_t page_addr = LOG_DATA_BASE_ADDR + page_count * SPI_EEPROM_PAGE_SIZE;
    
    // Check if address is valid
    if (page_addr <= SPI_EEPROM_SIZE_BYTE - SPI_EEPROM_PAGE_SIZE)
    {
        // Write EEPROM page
        EEPROM_writePage(page_addr, dataPtr, SPI_EEPROM_PAGE_SIZE);
        EEPROM_waitForWriteComplete();
    } 
}


/*******************************************************************************
* Function Name: EEPROM_storeLogMessage
********************************************************************************
*
* Summary:
*   Store a log type message inside a single page of EEPROM memory at the first
*   available address. At the end of the writing the fuction also increments
*   the log pages counter. 
*
* Parameters:  
*   Log message.
*
* Return:
*   None.
*
*******************************************************************************/
void EEPROM_storeLogMessage(log_t message)
{
    uint8_t buffer[64];
    
    // Updack struct and place data inside buffer
    LOG_unpackMessage(buffer, &message);
    
    // Store buffer in memory
    EEPROM_storeLogData(buffer);
    
    // Increment number of written pages
    EEPROM_incrementLogCounter();
}


/*******************************************************************************
* Function Name: EEPROM_findLogID
********************************************************************************
*
* Summary:
*   Search inside EEPROM memory for a given log identification number.
*
* Parameters:  
*   Log identification number.
*
* Return:
*   16-bit address of log.
*
* Side effects:
*   If the log is not found, an invalid address of 0xFFFF is returned.
*
*******************************************************************************/
uint16_t EEPROM_findLogID(uint8_t logID)
{
    // Scan every header of each page
    uint16_t addrPtr = LOG_DATA_BASE_ADDR;
    while(EEPROM_readByte(addrPtr) != logID)
    {
        // Increment to next header address
        addrPtr += SPI_EEPROM_PAGE_SIZE;
        
        // Check if address exeeds memory
        if (addrPtr >= SPI_EEPROM_SIZE_BYTE)
        {
            // Return invalid address
            return 0xFFFF;
        }
    }
    
    // If log ID has been found return address
    return addrPtr;
}


/*******************************************************************************
* Function Name: EEPROM_retrieveLogData
********************************************************************************
*
* Summary:
*   Retrieve the log n-th page data given its identification number and fill up
*   a pre-allocated buffer provided in input.
*
* Parameters:  
*   Buffer pointer, log identification number, index of desired page.
*
* Return:
*   None
*
*******************************************************************************/
void EEPROM_retrieveLogData(uint8_t* dataRX, uint8_t logID, uint8_t pageIndex)
{
    // Compute page address
    uint16_t page_addr = EEPROM_findLogID(logID) + pageIndex*SPI_EEPROM_PAGE_SIZE;
    
    // Retrieve 64 byte of data
    EEPROM_readPage(page_addr, dataRX, SPI_EEPROM_PAGE_SIZE);
}


/*******************************************************************************
* Function Name: EEPROM_retrieveLogMessage
********************************************************************************
*
* Summary:
*   Retrieve the n-th log type message given its identification number.
*
* Parameters:  
*   Log identification number, index of desired page.
*
* Return:
*   Log type message.
*
*******************************************************************************/
log_t EEPROM_retrieveLogMessage(uint8_t logID, uint8_t pageIndex)
{
    // Read 64 byte from page
    uint8_t buffer[SPI_EEPROM_PAGE_SIZE];
    EEPROM_retrieveLogData(buffer, logID, pageIndex);
    
    // Pack buffer inside log type message
    log_t msgRX;
    LOG_packMessage(&msgRX, buffer);
    
    // Return log message
    return msgRX;
}


/*******************************************************************************
* Function Name: EEPROM_resetMemory
********************************************************************************
*
* Summary:
*   Overwrite all EEPROM log memory with zeros and set reset flag inside
*   control register psoc status.
*
* Parameters:  
*   None.
*
* Return:
*   None.
*
*******************************************************************************/
void EEPROM_resetMemory(void)
{
    // Fill up buffer with zeros
    uint8_t resetBuffer[SPI_EEPROM_PAGE_SIZE];
    memset(resetBuffer, 0x00, SPI_EEPROM_PAGE_SIZE);
    
    // First available address
    uint16_t page_addr = 0x0000;
    
    // Reset all pages
    for (uint16_t i=1; i<SPI_EEPROM_PAGE_COUNT; i++)
    {
        // Reset page
        EEPROM_writePage(page_addr, resetBuffer, SPI_EEPROM_PAGE_SIZE);
        EEPROM_waitForWriteComplete();
        
        // Compute next page address
        page_addr += SPI_EEPROM_PAGE_SIZE;
    }
    
    // Set reset flag inside control register
    EEPROM_saveResetFlag(1);
}

/* [] END OF FILE */
