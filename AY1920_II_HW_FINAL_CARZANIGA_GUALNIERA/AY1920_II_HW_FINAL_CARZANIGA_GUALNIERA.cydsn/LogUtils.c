/* ========================================
 *
 * This file contains all function definitions
 * to create, handle and transmit log messages.
 *
 * A log message is used to store information
 * regarding IMU over threshold events and it
 * is composed as follows:
 * 
 * +--------------------+
 * |       Log ID       |   <1 byte>
 * +--------------------+
 * |    INT register    |   <1 byte>
 * +--------------------+
 * |      Timestamp     |   <2 bytes>
 * +--------------------+
 * |                    |
 * |        Data        |   <60 bytes>
 * |                    |
 * +--------------------+
 *
 * Log ID: identification number
 * 
 * INT register: info about external interrupt
 *               triggered by over threshold
 *
 * Timestamp: timestamp in seconds of the event
 *            occurrence
 *
 * Data: inertial measurement data, organized
 *       in row of 3 axis value X, Y, Z for a
 *       total of 20 rows (60 bytes)
 *
 * ========================================
*/


/* Project dependencies. */
#include "LogUtils.h"


/*******************************************************************************
* Function Name: LOG_createMessage
********************************************************************************
*
* Summary:
*   Create and return a 64 bytes log type message which contains header of 4 bytes
*   with all info about log occurrence and 60 bytes payload with IMU data.
*
* Parameters:  
*   Log identification number, interrupt register content, timestamp, data pointer.
*
* Return:
*   64-bit log data structure.
*
*******************************************************************************/
log_t LOG_createMessage(uint8_t logID, uint8_t intReg, uint16_t time, uint8_t* dataPtr)
{
    // Create log type message
    log_t message;
    
    // Assign log id number
    message.logID = logID;
    
    // Assign interrupt register
    message.intReg = intReg;
    
    // Assign logging timestamp
    message.timestamp = time;
    
    // Insert data payload
    LOG_insertPayload(&message, dataPtr);
    
    return message;
}


/*******************************************************************************
* Function Name: LOG_insertPayload
********************************************************************************
*
* Summary:
*   Insert IMU data payload inside data field of a log message type.
*
* Parameters:  
*   Log type message pointer, data payload pointer.
*
* Return:
*   None.
*
*******************************************************************************/
void LOG_insertPayload(log_t* msg, uint8_t* dataPtr)
{
    // Copy payload inside data field
    memcpy(msg->data, dataPtr, LOG_MESSAGE_DATA_BYTE);
}


/*******************************************************************************
* Function Name: LOG_getTimestamp
********************************************************************************
*
* Summary:
*   Get 16-bit timestamp in seconds from PSoC booting.
*
* Parameters:  
*   None
*
* Return:
*   16-bit timestamp.
*
*******************************************************************************/
uint16_t LOG_getTimestamp(void)
{
    // Read time in seconds from boot up
    uint32_t current_time = LOG_TIMER_OVERFLOW - MAIN_TIMER_ReadCounter();
    return (uint16_t)(current_time/LOG_TICK_PER_SECOND);
}


/*******************************************************************************
* Function Name: LOG_unpackMessage
********************************************************************************
*
* Summary:
*   Unpack 64 bytes log type message inside pre-allocated uint8 data buffer 
*   provided.
*
* Parameters:  
*   Data buffer pointer, log type message pointer.
*
* Return:
*   None.
*
*******************************************************************************/
void LOG_unpackMessage(uint8_t* buffer, log_t* message)
{
    // Updack header
    buffer[0] = message->logID;
    buffer[1] = message->intReg;
    buffer[2] = (message->timestamp & 0xFF);
    buffer[3] = ((message->timestamp >> 8) & 0xFF);
    
    // Unpack payload
    memcpy(&buffer[4], message->data, LOG_MESSAGE_DATA_BYTE);
}


/*******************************************************************************
* Function Name: LOG_packMessage
********************************************************************************
*
* Summary:
*   Pack 64 bytes data buffer inside pre-allocated log type message provided.
*
* Parameters:  
*   Log type message pointer, data buffer pointer.
*
* Return:
*   None.
*
*******************************************************************************/
void LOG_packMessage(log_t* message, uint8_t* buffer)
{
    // Pack header
    message->logID = buffer[0];
    message->intReg = buffer[1];
    message->timestamp = ((buffer[3]<<8) | buffer[2]);
    
    // Pack payload
    memcpy(message->data, &buffer[4], LOG_MESSAGE_DATA_BYTE);
}


/*******************************************************************************
* Function Name: LOG_sendData
********************************************************************************
*
* Summary:
*   Send 64 bytes log type message over UART.
*
* Parameters:  
*   Log type message pointer.
*
* Return:
*   None.
*
*******************************************************************************/
void LOG_sendData(log_t* message)
{
    // Unpack message inside buffer
    uint8_t buffer[LOG_MESSAGE_TOT_BYTE];
    LOG_unpackMessage(buffer, message);
    
    // Send buffer via UART
    UART_PutArray(buffer, LOG_MESSAGE_TOT_BYTE);
}

/* [] END OF FILE */
