/* ========================================
 *
 * This header file contains macros and 
 * function prototypes to interface with
 * the EEPROM 25LC256.
 *
 * ========================================
*/


/* Header guard. */
#ifndef __25LC256_H__
    
    #define __25LC256_H__

    /* Project dependencies. */
    #include "SPI_Interface.h"
    #include "LogUtils.h"

    /* Instruction Set. */
    #define SPI_EEPROM_READ    0b00000011
    #define SPI_EEPROM_WRITE   0b00000010
    #define SPI_EEPROM_WRDI    0b00000100
    #define SPI_EEPROM_WREN    0b00000110
    #define SPI_EEPROM_RDSR    0b00000101
    #define SPI_EEPROM_WRSR    0b00000001

    /* Write Complete Time. */
    #define SPI_EEPROM_Twc 5

    /* EEPROM Status Registrer Masks */
    #define SPI_EEPROM_WRITE_IN_PROCESS_SHIFT    0
    #define SPI_EEPROM_WRITE_ENABLE_LATCH_SHIFT  1
    #define SPI_EEPROM_BLOCK_PROTECTION_0_SHIFT  2
    #define SPI_EEPROM_BLOCK_PROTECTION_1_SHIFT  3
    #define SPI_EEPROM_WPEN_SHIFT                7

    #define SPI_EEPROM_WRITE_IN_PROGRESS    ((uint8_t) 0x01u << SPI_EEPROM_WRITE_IN_PROCESS_SHIFT)
    #define SPI_EEPROM_WRITE_ENABLE_LATCH   ((uint8_t) 0x01u << SPI_EEPROM_WRITE_ENABLE_LATCH_SHIFT)
    #define SPI_EEPROM_BLOCK_PROTECTION_0   ((uint8_t) 0x01u << SPI_EEPROM_BLOCK_PROTECTION_0_SHIFT)
    #define SPI_EEPROM_BLOCK_PROTECTION_1   ((uint8_t) 0x01u << SPI_EEPROM_BLOCK_PROTECTION_1_SHIFT)
    #define SPI_EEPROM_WPEN                 ((uint8_t) 0x01u << SPI_EEPROM_WPEN_SHIFT)

    #define SPI_EEPROM_PAGE_SIZE    64
    #define SPI_EEPROM_PAGE_COUNT   512
    #define SPI_EEPROM_SIZE_BYTE    0x7FFF

    /* EEPROM User defined regiter masks. */
    #define CTRL_REG_PSOC_STATUS    0x0000
    #define CTRL_REG_LOG_PAGES_LOW  0x0008
    #define CTRL_REG_LOG_PAGES_HIGH 0x0016
    #define LOG_DATA_BASE_ADDR      0x0040
    #define LOG_PAGES_PER_EVENT     5

    #define CTRL_REG_PSOC_START_STOP_SHIFT  0
    #define CTRL_REG_PSOC_CONFIG_MODE_SHIFT 1
    #define CTRL_REG_PSOC_SEND_FLAG_SHIFT   2
    #define CTRL_REG_PSOC_RESET_FLAG_SHIFT  3

    #define CTRL_REG_PSOC_SET_START         ((uint8_t) 0x01u << CTRL_REG_PSOC_START_STOP_SHIFT)
    #define CTRL_REG_PSOC_SET_CONFIG        ((uint8_t) 0x01u << CTRL_REG_PSOC_CONFIG_MODE_SHIFT)
    #define CTRL_REG_PSOC_SET_SEND_FLAG     ((uint8_t) 0x01u << CTRL_REG_PSOC_SEND_FLAG_SHIFT)
    #define CTRL_REG_PSOC_SET_RESET_FLAG    ((uint8_t) 0x01u << CTRL_REG_PSOC_RESET_FLAG_SHIFT)
 
    /* General read/write functions. */
    uint8_t EEPROM_readStatus(void);
    void EEPROM_writeEnable(void);
    uint8_t EEPROM_readByte(uint16_t addr);
    void EEPROM_writeByte(uint16_t addr, uint8_t dataByte);
    void EEPROM_readPage(uint16_t addr, uint8_t* dataRX, uint8_t nBytes);
    void EEPROM_writePage(uint16_t addr, uint8_t* data, uint8_t nBytes); 
    void EEPROM_waitForWriteComplete(void);

    /* Task-specific read/write functions. */
    void EEPROM_saveStartStopState(uint8_t state);
    uint8_t EEPROM_retrieveStartStopState(void);
    void EEPROM_saveConfigFlag(uint8_t flag);
    uint8_t EEPROM_retrieveConfigFlag(void);
    void EEPROM_saveSendFlag(uint8_t flag);
    uint8_t EEPROM_retrieveSendFlag(void);
    void EEPROM_saveResetFlag(uint8_t flag);
    uint8_t EEPROM_retrieveResetFlag(void);
    uint16_t EEPROM_retrieveLogPages(void);
    uint8_t EEPROM_retrieveLogCount(void);
    void EEPROM_incrementLogCounter(void);
    void EEPROM_resetMemory(void);
    
    /* Log type data read/write functions. */
    void EEPROM_storeLogData(uint8_t* dataPtr);
    void EEPROM_storeLogMessage(log_t message);
    uint16_t EEPROM_findLogID(uint8_t logID);
    void EEPROM_retrieveLogData(uint8_t* dataRX, uint8_t logID, uint8_t pageIndex);
    log_t EEPROM_retrieveLogMessage(uint8_t logID, uint8_t pageIndex);

#endif

/* [] END OF FILE */