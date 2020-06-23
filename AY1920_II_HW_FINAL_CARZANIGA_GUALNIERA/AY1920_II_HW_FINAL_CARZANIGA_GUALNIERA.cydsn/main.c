/* ========================================
 *
 * This file contains the main function which
 * is designed to enable all ISRs and setup
 * all communication protocols to interface
 * the PSoC board with: 
 * -> 25LC256 EEPROM
 * -> LIS3DH accelerometer
 * -> Serial monitor (UART)
 *
 * ========================================
 *
 * Driving of LED RGB and user inputs:
 *
 * The main logic behind the RGB driver is
 * subdivided into 3 states:
 * -> STOP_MODE: the LED is completely turned 
 *    off.
 * -> START_MODE: the LED is drived by processed
 *    data coming from the IMU.
 * -> CONFIG_MODE: only the blue channel of
 *    the LED is driven and it is either on 
 *    or off based on the send flag value.
 *    When in config mode the user is able,
 *    with the use of a potentiometer, to set
 *    the send flag and allow transmission of
 *    IMU data over UART.
 *
 * ========================================
 *
 * LIS3DH FIFO data reading:
 * 
 * This section is executed when the flag
 * for the fifo data ready is set by a 
 * custom ISR coming from the LIS3DH.
 * The current 32 levels of the FIFO are 
 * stored in a buffer for the drive of the 
 * LED RGB, while a down-sampled copy of the
 * data is stored in a queue. This is done in
 * order to maintain a brief history of the
 * data and be able to log it into the EEPROM
 * when needed.
 *
 * ========================================
 *
 * LIS3DH over threshold event logging:
 *
 * This section is executed when the flag
 * for the over threshold event is set by a
 * custom ISR coming from the LIS3DH.
 * A log type message is generated given the
 * information about the event and the
 * payload that is retrieved from IMU queue.
 * Finally, the message is successfully 
 * stored inside the EEPROM memory.
 *
 * ========================================
*/


/* Project dependencies. */
#include "InterruptRoutines.h"
#include "RGB_Driver.h"
#include "LogUtils.h"
#include "25LC256.h"
#include "LIS3DH.h"


/* Main function definition. */
int main(void)
{   
    // Enable global interrupts
    CyGlobalIntEnable;
    
    // Enable UART communication
    UART_Start();
    
    // Enable all SPI modules
    SPIM_IMU_Start();
    SPIM_EEPROM_Start();
    CyDelay(10);

    // Initialize all timers 
    BUTTON_TIMER_Start();
    CLICK_TIMER_Start();
    MAIN_TIMER_Start();
    
    // Initialize ADC
    ADC_DELSIG_Start();
 
    // Setup all LIS3DH registers
    IMU_Init();
    
    // Initliazide RGB LED
    RGB_Init();
    
    // Enable all ISRs
    ISR_CONFIG_StartEx(CUSTOM_ISR_CONFIG);
    ISR_START_StartEx(CUSTOM_ISR_START);
    ISR_IMU_StartEx(CUSTOM_ISR_IMU);
    ISR_RX_StartEx(CUSTOM_ISR_RX);

    // End of setup
    CyDelay(10);
    
    // Initialize button state
    button_state = STOP_MODE;
    EEPROM_saveStartStopState(0);
    
    // Initialize send flag
    send_flag = 0;
    
    // Initialize IMU flags
    IMU_data_ready_flag = 0;
    IMU_over_threshold_flag = 0;

    // Uncomment this to erase EEPROM memory
    //EEPROM_resetMemory();
    
    // Main loop
    for(;;)
    {   
        // Board state handler
        switch (button_state)
        {
            case STOP_MODE:
                
                // Turn LED off
                RGB_Stop();
                break;
            
            case START_MODE:
                
                // Drive LED based on IMU data
                RGB_Driver(IMU_DataBuffer);
                break;
     
            case CONFIG_MODE:
                
                // Read knob value to set send flag
                send_flag = POT_Read_Value(send_flag);
                   
                // Drive LED blue channel based on flag
                RGB_sendFlagNotify(send_flag);
                break;  
        }

        // IMU ISR FIFO data overrun event
        if (IMU_data_ready_flag == 1)
        {
            // Read data via SPI from IMU
            IMU_ReadFIFO(IMU_DataBuffer);
            
            // Store the read FIFO in the LOG buffer
            IMU_StoreFIFO(IMU_DataBuffer);
            
            // Check EEPROM if send flag is set
            if (EEPROM_retrieveSendFlag() == 1)
            {
                //Send data read from FIFO via UART
                IMU_DataSend(IMU_DataBuffer);
            }

            // Reset the FIFO to enable next ISR occurrences
            IMU_ResetFIFO();
            
            // End of IMU data reading
            IMU_data_ready_flag = 0;
        }
        
        // IMU ISR over threshold event
        if (IMU_over_threshold_flag == 1)
        {   
            // Get sequential ID number
            uint8_t log_id = EEPROM_retrieveLogCount();
            
            // Get interrupt register with info about event
            uint8_t int_reg = IMU_ReadByte(LIS3DH_INT1_SRC);
            
            // Get timestamp in seconds from boot
            uint16_t timestamp = LOG_getTimestamp();
            
            // Capture all over threshold event's interrupts
            while(IMU_ReadByte(LIS3DH_INT1_SRC) & (LIS3DH_INT1_SRC_IA_MASK)){}
  
            for (uint8_t i=0; i<LOG_PAGES_PER_EVENT; i++)
            {        
                // Get payload of 60 bytes from the IMU queue
                uint8_t payload[LOG_MESSAGE_DATA_BYTE];
                IMU_getPayload(payload, i);
                
                // Create log type message
                log_t log_message = LOG_createMessage(log_id, int_reg, timestamp, payload);
                
                // Store message inside EEPROM
                EEPROM_storeLogMessage(log_message);
            }
            
            // Reset the FIFO to enable new ISR occurrences
            IMU_ResetFIFO();
            
            // End of over threshold event
            IMU_over_threshold_flag = 0;
        }   
    }
    
    return 0;
}

/* [] END OF FILE */
