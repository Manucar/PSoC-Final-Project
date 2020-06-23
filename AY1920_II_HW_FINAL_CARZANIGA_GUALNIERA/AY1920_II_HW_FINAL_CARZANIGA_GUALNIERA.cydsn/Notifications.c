/* ========================================
 *
 * This file contains utility functions
 * used for visually notify the user of the
 * internal state of the firmware by driving
 * the PSoC on-board LED, as well as taking 
 * user inputs throught a potentiometer to
 * set a logical flag used inside the main.
 *
 * ========================================
*/


/* Project dependencies. */
#include "Notifications.h"


/*******************************************************************************
* Function Name: LED_Notify_Stop
********************************************************************************
*
* Summary:
*   Notify visually when internal state is set to stop mode by turning PSoC 
*   on-board LED off.
*
* Parameters:  
*   None.
*
* Return:
*   None.
*
*******************************************************************************/
void LED_Notify_Stop(void)
{
    // If LED PWM is enabled
    if (PWM_NOTIFY_ReadControlRegister() & PWM_NOTIFY_CTRL_ENABLE)
    {
        // Disable PWM
        PWM_NOTIFY_Stop();
    }
}


/*******************************************************************************
* Function Name: LED_Notify_Start
********************************************************************************
*
* Summary:
*   Notify visually when internal state is set to stop mode by turning PSoC 
*   on-board LED on.
*
* Parameters:  
*   None.
*
* Return:
*   None.
*
*******************************************************************************/
void LED_Notify_Start(void)
{
    // If LED PWM is not enabled
    if (!(PWM_NOTIFY_ReadControlRegister() & PWM_NOTIFY_CTRL_ENABLE))
    {
        // Enable PWM
        PWM_NOTIFY_Start();
    }
    
    // Change PWM compare value
    PWM_NOTIFY_WriteCompare(255);
}


/*******************************************************************************
* Function Name: LED_Notify_Config
********************************************************************************
*
* Summary:
*   Notify visually when internal state is set to stop mode by making PSoC 
*   on-board LED blink at 1Hz frequency.
*
* Parameters:  
*   None.
*
* Return:
*   None.
*
*******************************************************************************/
void LED_Notify_Config(void)
{
    // If LED PWM is not enabled
    if (!(PWM_NOTIFY_ReadControlRegister() & PWM_NOTIFY_CTRL_ENABLE))
    {
        // Enable PWM
        PWM_NOTIFY_Start();
    }
    
    // Change PWM compare value
    PWM_NOTIFY_WriteCompare(127);
}


/*******************************************************************************
* Function Name: POT_Read_Value
********************************************************************************
*
* Summary:
*   Read user input potentiometer value from ADC delta sigma and process it to 
*   be either 1 or 0 according to the folliwing potentiomenter range mapping:
*   +--------------------------+
*   | 0%->24%   :            0 |
*   | 25%->50%  : don't change |
*   | 51%->100% :            1 |
*   +--------------------------+
*
* Parameters:  
*   Previous flag value.
*
* Return:
*   New flag value.
*
*******************************************************************************/
uint8_t POT_Read_Value(uint8_t oldFlag)
{
    // Start and wait for ADC conversion
    ADC_DELSIG_StartConvert();
    ADC_DELSIG_IsEndConversion(ADC_DELSIG_WAIT_FOR_RESULT);
    
    // Read and stop ADC conversion
    int16_t raw = ADC_DELSIG_Read16();
    ADC_DELSIG_StopConvert();
    
    // Clip outside range values
    if (raw > 255) raw = 255;
    else if (raw < 0) raw = 0;
    
    // Convert to unsigned 8-bit integer
    uint8_t pot_val = (uint8_t)(raw & 0xFF);
    
    // Set flag based on read value
    uint8_t flag;
    if (pot_val < (POT_MID_RANGE - POT_DEAD_BAND))
    {
        flag = 0;
    }
    else if (pot_val > (POT_MID_RANGE + POT_DEAD_BAND))
    {
        flag = 1;
    }
    else
    {
        flag = oldFlag;
    }
    
    // Return new flag
    return flag;
}

/* [] END OF FILE */
