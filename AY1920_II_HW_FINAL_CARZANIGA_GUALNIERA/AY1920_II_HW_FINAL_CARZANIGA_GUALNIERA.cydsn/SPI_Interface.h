/* ========================================
 * 
 * This header file contains macros and 
 * function prototypes to interface with 
 * the SPI Master of the PSoC.
 *
 * ========================================
*/


/* Header guard */
#ifndef __SPI_INTERFACE_H__
    
    #define __SPI_INTERFACE_H__

    /* Project dependencies. */
    #include "project.h"

    /* SPI Constants */
    #define SPI_DUMMY_BYTE  0x00
    #define SPI_TxBufferSize 8
    #define SPI_RxBufferSize 8
    
    /* SPI IMU function prototype declaration. */
    uint8_t SPI_IMU_Interface_tradeByte(uint8_t byte);
    uint8_t SPI_IMU_Interface_ReadByte(uint8_t addr);
    void SPI_IMU_Interface_Multi_RW(uint8_t* dataTX, uint8_t bytesTX, uint8_t* dataRX, uint8_t bytesRX);
    
    /* SPI EEPROM function prototype declaration. */
    uint8_t SPI_EEPROM_Interface_tradeByte(uint8_t byte);
    uint8_t SPI_EEPROM_Interface_ReadByte(uint8_t addr);
    void SPI_EEPROM_Interface_Multi_RW(uint8_t* dataTX, uint8_t bytesTX, uint8_t* dataRX, uint8_t bytesRX);
    
#endif

/* [] END OF FILE */
