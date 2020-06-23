/* ========================================
 *
 * This header file defines the ISR function
 * prototypes and the finite state machines
 * variables to enable all project features.
 *
 * ========================================
*/


/* Header guard. */
#ifndef __INTERRUPT_ROUTINES_H__
    
    #define __INTERRUPT_ROUTINES_H__
    
    /* Project dependencies. */
    #include "project.h"
    #include "25LC256.h"
    #include "LIS3DH.h"
    #include "Notifications.h"
    
    /* Remote UART Instruction Set. */
    #define UART_RX_OPERATION_ACK   0x4B
    #define UART_RX_RESET_MEMORY    0x52
    #define UART_RX_NUMBER_OF_LOGS  0x4E
    #define UART_RX_READ_CTRL_REG   0x43
    #define UART_RX_SEND_LOG_ID     0x4C
    
    /* State machine type. */
    typedef enum {
        STOP_MODE,
        START_MODE,
        CONFIG_MODE
    } button_t;

    /* LIS3DH interrupt flags. */
    volatile uint8_t IMU_data_ready_flag;
    volatile uint8_t IMU_over_threshold_flag;
    
    /* Internal state variable. */
    volatile button_t button_state;
    volatile uint8_t send_flag;
    
    /* ISR functions prototype declaration. */
    CY_ISR_PROTO(CUSTOM_ISR_CONFIG);
    CY_ISR_PROTO(CUSTOM_ISR_START);
    CY_ISR_PROTO(CUSTOM_ISR_IMU);
    CY_ISR_PROTO(CUSTOM_ISR_RX);
    
#endif


/* [] END OF FILE */
