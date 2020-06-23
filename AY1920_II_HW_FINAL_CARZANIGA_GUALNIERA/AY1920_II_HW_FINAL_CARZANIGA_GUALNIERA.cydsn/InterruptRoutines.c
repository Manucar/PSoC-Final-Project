/* ========================================
 *
 * This file contains ISR function definitions.
 * In this application ISRs are used for
 * various purposes:
 *
 * 1) Link internal finite states with 
 *    hardware events.
 *
 * 2) Handle LIS3DH external interrupts.
 *
 * 3) Execution of remote commands sent over
 *    UART to read/write to the EEPROM memory.
 *
 * ========================================
*/


/* Project dependencies. */
#include "InterruptRoutines.h"


/*******************************************************************************
* Function Name: CUSTOM_ISR_CONFIG
********************************************************************************
*
* Summary:
*   Triggered with a long press of 2 seconds of the on-board button, 
*   it toggles the configuration mode state and save config flag inside EEPROM.
*   
* Priotity level: 5
*
* Parameters:  
*   None
*
* Return:
*   None
*
*******************************************************************************/
CY_ISR(CUSTOM_ISR_CONFIG)
{   
    // Toggle configuration state
    if (button_state != CONFIG_MODE)
    {
        // Stop IMU interrupt events if was in START_MODE
        if (button_state == START_MODE)
        {
            IMU_Stop();
        }

        // Enter configuration mode
        button_state = CONFIG_MODE;
        
        // Save config flag inside EEPROM
        EEPROM_saveConfigFlag(1);
        
        // Blink on-board LED
        LED_Notify_Config();
    }
    else
    {
        // Resume start/stop mode
        button_state = EEPROM_retrieveStartStopState();
        
        // Reset config flag inside EEPROM
        EEPROM_saveConfigFlag(0);
        
        if (button_state == STOP_MODE)
        {
            // Turn on-board LED off
            LED_Notify_Stop();
        }
        else if (button_state == START_MODE)
        {
            // Turn on-board LED on
            LED_Notify_Start();
            
            // Start IMU interrupt events
            IMU_Start();
        }

        // Save send flag inside EEPROM
        EEPROM_saveSendFlag(send_flag);
    }
}


/*******************************************************************************
* Function Name: CUSTOM_ISR_START
********************************************************************************
*
* Summary:
*   Triggered with a double click event (with time delta between clicks less 
*   than 1s), it toggles the start/stop state and save it inside the EEPROM.
* 
* Priority level: 6
*
* Parameters:  
*   None
*
* Return:
*   None
*
*******************************************************************************/
CY_ISR(CUSTOM_ISR_START)
{   
    // Toggle start/stop state
    if (button_state == START_MODE)
    {
        // Stop IMU interrupt events
        IMU_Stop();
        
        // Save stop bit inside EEPROM
        EEPROM_saveStartStopState(0);

        // Turn on-board LED off
        LED_Notify_Stop();
                
        // Update button state
        button_state = STOP_MODE;
    }
    else if (button_state == STOP_MODE)
    {
        // Start IMU interrupt events
        IMU_Start();
        
        // Save start bit inside EEPROM
        EEPROM_saveStartStopState(1);

        // Turn on-board LED on
        LED_Notify_Start();
                
        // Update button state
        button_state = START_MODE;
    }
}


/*******************************************************************************
* Function Name: CUSTOM_ISR_IMU
********************************************************************************
*
* Summary:
*   Set the flags based on which external interrupt occurs.
* 
* Priority level: 7
*
* Parameters:  
*   None
*
* Return:
*   None
*
*******************************************************************************/
CY_ISR(CUSTOM_ISR_IMU)
{
    // IMU interrupt data overrun event handler
    if (IMU_ReadByte(LIS3DH_FIFO_SRC_REG) & LIS3DH_FIFO_SRC_REG_OVR_MASK)
    {
        IMU_data_ready_flag = 1;
    }
    
    // IMU interrupt over threshold event handler
    if (IMU_ReadByte(LIS3DH_INT1_SRC) & LIS3DH_INT1_SRC_IA_MASK)
    {
        IMU_over_threshold_flag = 1;
    }
}


/*******************************************************************************
* Function Name: CUSTOM_ISR_RX
********************************************************************************
*
* Summary:
*   Execute valid operation codes sent remotely via UART in order to read/write
*   from/to the EEPROM memory:
*   +--------------------------------------------------------------------------+
*   | Operation codes:                                                         |
*   | -> UART_RX_RESET_MEMORY     :   Erase all 512 memory pages               |
*   | -> UART_RX_NUMBER_OF_LOGS   :   Send number of logs currently stored     |
*   | -> UART_RX_READ_CTRL_REG    :   Send psoc control register content       |
*   | -> UART_RX_SEND_LOG_ID      :   Send log message corresponding to ID     |
*   +--------------------------------------------------------------------------+
*   
* Priority level: 7
*
* Parameters:  
*   None
*
* Return:
*   None
*
*******************************************************************************/
CY_ISR(CUSTOM_ISR_RX)
{
    // Read operation code received over UART
    uint8_t op_code = UART_GetChar();
    CyDelay(1);
    
    // Execute intruction
    switch (op_code)
    {   
        case (UART_RX_RESET_MEMORY):
        {
            // Erase all EEPROM log pages
            EEPROM_resetMemory();
            
            // Notify that operation is complete
            UART_PutChar(UART_RX_OPERATION_ACK);
            break;
        }
        
        case (UART_RX_NUMBER_OF_LOGS):
        {
            // Read current log counter value
            uint8_t log_count = EEPROM_retrieveLogCount();
            
            // Send byte over UART
            UART_PutChar(log_count);
            break;
        }   
        
        case (UART_RX_READ_CTRL_REG):
        {
            // Read control register content
            uint8_t ctrl_reg = EEPROM_readByte(CTRL_REG_PSOC_STATUS);
            
            // Send byte over UART
            UART_PutChar(ctrl_reg);
            break;
        }
        
        case (UART_RX_SEND_LOG_ID):
        {
            // Get desired log ID
            uint8_t log_id = UART_GetChar();

            // Allocate memory for log type
            log_t log_page;
                
            // Cycle over all log pages
            for (uint8_t i=0; i<LOG_PAGES_PER_EVENT; i++)
            {   
                // Retrieve log page from EEPROM
                log_page = EEPROM_retrieveLogMessage(log_id, i);
                
                // Send log page over UART
                LOG_sendData(&log_page);
            }
            break;
        }
    }
}

/* [] END OF FILE */
