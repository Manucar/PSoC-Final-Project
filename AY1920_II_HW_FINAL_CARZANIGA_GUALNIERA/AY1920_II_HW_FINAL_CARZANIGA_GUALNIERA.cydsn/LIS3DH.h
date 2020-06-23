/* ========================================
 * 
 * This header file contains constants and 
 * function prototypes to interface with the
 * LIS3DH accelerometer.
 *
 * ========================================
*/

/* Header guard */
#ifndef __LIS3DH_H__
    
    #define __LIS3DH_H__

    /* Include required libraries. */
    #include "SPI_Interface.h"
    #include "project.h"
    
    /* IMU Constants */
    #define LIS3DH_FIFO_BYTES_IN_LEVEL 6
    #define LIS3DH_LEVELS_IN_FIFO 32
    #define LIS3DH_BYTES_IN_FIFO 192
    #define LIS3DH_BYTES_IN_FIFO_HIGH_REG 96
    #define LIS3DH_FIFO_STORED 6
    #define LIS3DH_DOWN_SAMPLE 2
    #define LIS3DH_BYTES_IN_FIFO_DOWNSAMPLED LIS3DH_BYTES_IN_FIFO_HIGH_REG/LIS3DH_DOWN_SAMPLE
    #define LIS3DH_BYTES_IN_LOG_BUFFER LIS3DH_BYTES_IN_FIFO_DOWNSAMPLED * LIS3DH_FIFO_STORED // 16 levels * 3 registers * 6 FIFO
    
    /* Buffer that store read data from IMU of one FIFO*/
    uint8_t IMU_DataBuffer[LIS3DH_BYTES_IN_FIFO];
    
    /* Array that acts like a queue storing 6 FIFO at time*/
    uint8_t IMU_log_queue[LIS3DH_BYTES_IN_LOG_BUFFER];
    
    /* Binary mask to set the read bit in the instruction to be sent. */
    #define LIS3DH_READ_BIT 0b10000000
    
    /* Null value to reset registers. */
    #define LIS3DH_RESET_REG  0x00
    
    /* Address of the read address of  WHO AM I REG. */
    #define LIS3DH_WHO_AM_I_REG  0x0F
    
    /* Address of the status registers. */
    #define LIS3DH_STATUS_REG  0x27

    /* Address of the X axis output low register and auto-increment address. */
    #define LIS3DH_READ_OUT_X_L 0xE8

    /* Address of the Control register 1. */
    #define LIS3DH_CTRL_REG1 0x20
    
    /* Hex value to disable x,y,z axis. */
    #define LIS3DH_CTRL_REG1_STOP_XYZ   0x68
      
    /* Hex value to set low power mode to the accelerator and 200Hz data rate. */
    #define LIS3DH_CTRL_REG1_START_XYZ  0x6F

    /* Address of the Control register 3. */
    #define LIS3DH_CTRL_REG3 0x22
    
    /* Hex value to enable interrupt on IA1 and overrun. */
    #define LIS3DH_CTRL_REG3_I1_IA1_OVERRUN 0x42

    /* Hex value to disable interrupt on pin INT1. */
    #define LIS3DH_CTRL_REG3_NULL 0x00

    /* Address of the Control register 4. */
    #define LIS3DH_CTRL_REG4 0x23

    /* Hex value to set output registers not updated until MSB and LSB reading, FSR +-2g and SPI 4 wire interface. */
    #define LIS3DH_CTRL_REG4_BDU_ACTIVE 0x80

    /*Address of the Control register 5. */
    #define LIS3DH_CTRL_REG5 0x24

    /* Hex value to enable FIFO mode. */
    #define LIS3DH_CTRL_REG5_FIFO_ENABLE 0x40

    /* Hex value to disable FIFO mode. */
    #define LIS3DH_CTRL_REG5_FIFO_DISABLE 0x00

    /* Address of the FIFO Control register. */
    #define LIS3DH_FIFO_CTRL_REG 0x2E

    /* Hex value to enable BYPASS mode. */
    #define LIS3DH_FIFO_CTRL_REG_BYPASS_MODE 0x00

    /* Hex value to enable FIFO mode. */
    #define LIS3DH_FIFO_CTRL_REG_FIFO_MODE 0x40
    
    /* Address of the FIFO Control register. */
    #define LIS3DH_FIFO_SRC_REG 0x2F
    
    /* Binary mask to check if FIFO_SRC_REG has overrun bit set to 1. */
    #define LIS3DH_FIFO_SRC_REG_OVR_MASK 0b01000000
    
    /* Address of the INT1 CFG register. */
    #define LIS3DH_INT1_CFG 0x30
    
    /* Hex value to disable isr on events for all the 3 axis. */
    #define LIS3DH_ITN1_CFG_DISABLE_EVENTS 0x00
    
    /* Hex value to set isr on high events for all the 3 axis. */
    #define LIS3DH_INT1_CFG_XYZ_HIGH_EVENTS 0x2A
    
    /* Address of the INT1 SRC register. */
    #define LIS3DH_INT1_SRC 0x31
    
    /* Mask for INT1 SRC register to detect isr occurrences. */
    #define LIS3DH_INT1_SRC_IA_MASK 0b01000000
    
    /* Address of the INT1 THS register. */
    #define LIS3DH_INT1_THS 0x32
    
    /* Hex value for the threshold of each IMU axis. */
    #define LIS3DH_INT1_THS_VALUE 0x64  //@+-2G FSR ---> 1 LSB = 16mG ---> 0x64 ---> 1.6G
    
    /* Address of the INT1 DURATION register. */
    #define LIS3DH_INT1_DURATION 0x33
    
    /* Hex value for the time duration of an overthreshold event. */ 
    #define LIS3DH_INT1_DURATION_VALUE 0x14 // @ 200Hx ODR --> 5 ms every lsb --> 100ms total

    /* LIS3DH function prototype declaration. */

    uint8_t IMU_ReadByte(uint8_t address);
    
    void IMU_Init(void);
    void IMU_Setup(void);
    void IMU_Stop(void);
    void IMU_Start(void);

    void IMU_RegistersSetup(void);
    
    void IMU_ReadFIFO(uint8_t *buffer);
    void IMU_DataSend(uint8_t *buffer);
    void IMU_StoreFIFO(uint8_t *buffer);
    void IMU_getPayload(uint8_t *messagge, uint8_t index);
    void IMU_ResetFIFO(void);
    
#endif


/* [] END OF FILE */