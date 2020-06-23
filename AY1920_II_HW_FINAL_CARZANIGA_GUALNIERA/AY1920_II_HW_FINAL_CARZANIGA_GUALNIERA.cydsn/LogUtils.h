/* ========================================
 *
 * This header file contains prototypes of 
 * utility functions used to create, manage
 * and transmit log messages as well as a 
 * custom data type to store log data.
 *
 * ========================================
*/


/* Header guard. */
#ifndef __LOG_UTILS_H__
    
    #define __LOG_UTILS_H__
    
    /* Project dependencies. */
    #include "project.h"
    
    /* Useful constants definition. */
    #define LOG_MESSAGE_HEADER_BYTE 4
    #define LOG_MESSAGE_DATA_BYTE   60
    #define LOG_MESSAGE_TOT_BYTE    64
    #define LOG_TICK_PER_SECOND     1000
    #define LOG_TIMER_OVERFLOW      0xFFFFFFFF
    
    /* Log message type. */
    typedef struct {
        uint8_t logID;
        uint8_t intReg;
        uint16_t timestamp;
        uint8_t data[LOG_MESSAGE_DATA_BYTE];
    } log_t;
    
    /* Function prototype declaration. */
    log_t LOG_createMessage(uint8_t logID, uint8_t intReg, uint16_t time, uint8_t* dataPtr);  
    void LOG_insertPayload(log_t* msg, uint8_t* dataPtr);
    uint16_t LOG_getTimestamp(void); 
    void LOG_unpackMessage(uint8_t* buffer, log_t* message); 
    void LOG_packMessage(log_t* message, uint8_t* buffer);
    void LOG_sendData(log_t* message);
    
#endif

/* [] END OF FILE */
