/* ========================================
 *
 * This file contains utility functions used
 * to drive the RGB LED with different
 * modalities based on the internal state:
 *
 * -> STOP MODE: the RGB LED is completely 
 *    turned off.
 * 
 * -> START MODE: the driving of the LED
 *    is based on IMU data by mapping XYZ
 *    values to RGB values.
 *
 * -> CONFIG MODE only the blue channel
 *    of the LED is driven and it is either
 *    turn on or off based con the logical
 *    value of a flag.
 *
 * ========================================
*/


/* Project dependencies. */
#include "RGB_Driver.h"


/*******************************************************************************
* Function Name: RGB_Init
********************************************************************************
*
* Summary:
*   Initialize and start RGB pulse width modulators with configured settings.
*
* Parameters:  
*   None
*
* Return:
*   None.
*
*******************************************************************************/
void RGB_Init(void)
{
    // Check if enable bit isn't already set
    if (!(PWM_RG_ReadControlRegister() & PWM_RG_CTRL_ENABLE))
    {
        // Enable red/green PWM
        PWM_RG_Start();
    }
    
    // Check if enable bit isn't already set
    if (!(PWM_B_ReadControlRegister() & PWM_B_CTRL_ENABLE))
    {
        // Enable blue PWM
        PWM_B_Start();
    }
    
    // Turn LED off
    RGB_Stop();
}


/*******************************************************************************
* Function Name: RGB_Stop
********************************************************************************
*
* Summary:
*   Stop RGB pulse width modulators if not already paused, do nothing otherwise.
*
* Parameters:  
*   None
*
* Return:
*   None.
*
*******************************************************************************/
void RGB_Stop(void)
{
    // Turn off LED red channel 
    if (PWM_RG_ReadCompare1() != PWM_COMPARE_STOP)
    {
        PWM_RG_WriteCompare1(PWM_COMPARE_STOP);
    }
    
    // Turn off LED green channel 
    if (PWM_RG_ReadCompare2() != PWM_COMPARE_STOP)
    {
        PWM_RG_WriteCompare2(PWM_COMPARE_STOP);
    }
    
    // Turn off LED blue channel 
    if (PWM_B_ReadCompare() != PWM_COMPARE_STOP)
    {
        PWM_B_WriteCompare(PWM_COMPARE_STOP);
    }
}

/*
 * Drive LED RGB by setting PWM duty cycles based
 * on xyz-axes IMU data so that:
 * -> X value drives RED channel
 * -> Y value drives BLUE channel
 * -> Z value drives GREEN channel
 */
/*******************************************************************************
* Function Name: RGB_Driver
********************************************************************************
*
* Summary:
*   High level function that enable LED driving based on IMU data coming from 
*   LIS3DH. Given a single FIFO of IMU data this function first compute the
*   average in order to obtain a stable value, then process the data to map
*   the whole working range of the RGB LED with the absolute value of the
*   inertial measurements and finally drive the two PWMs.
*
* Parameters:  
*   IMU FIFO data pointer.
*
* Return:
*   None.
*
*******************************************************************************/
void RGB_Driver(uint8_t* dataPtr)
{   
    // Apply moving average filter
    Moving_Average(dataPtr, RGB_DataBuffer, 32);
    
    // Process IMU data in place
    RGB_dataProcess(RGB_DataBuffer);
    
    // Set PWM compare values
    PWM_Driver(RGB_DataBuffer);
}


/*******************************************************************************
* Function Name: RGB_sendFlagNotify
********************************************************************************
*
* Summary:
*   Drive RGB LED blue channel only based on a logical flag value:
*   +---------------+
*   | 1 -> turn on  |
*   | 0 -> turn off |
*   +---------------+
*
* Parameters:  
*   Logical flag value.
*
* Return:
*   None.
*
*******************************************************************************/
void RGB_sendFlagNotify(uint8_t flag)
{
    // Drive blue channel only
    uint8_t buffer[3] = {0, PWM_CYCLE_LENGTH * flag, 0};
    PWM_Driver(buffer);
}


/*******************************************************************************
* Function Name: PWM_Driver
********************************************************************************
*
* Summary:
*   Drive LED RGB by setting PWM duty cycles based on xyz-axes IMU data so that:
*   +--------------------------+
*   | X value ->   RED channel |
*   | Y value ->  BLUE channel |
*   | Z value -> GREEN channel |
*   +--------------------------+
*
* Parameters:  
*   Buffer data pointer.
*
* Return:
*   None.
*
*******************************************************************************/
void PWM_Driver(uint8* dataPtr)
{
    // Set red channel PWM compare value
    PWM_RG_WriteCompare1(dataPtr[0]);
    
    // Set green channel PWM compare value
    PWM_RG_WriteCompare2(dataPtr[2]);
    
    //Set blue channel PWM compare value
    PWM_B_WriteCompare(dataPtr[1]);
}


/*******************************************************************************
* Function Name: RGB_dataProcess
********************************************************************************
*
* Summary:
*   Process IMU data in place to map full LED range. First the absolute value is
*   computed and then is scaled by 2 to map the LED working range.
*
* Parameters:  
*   Buffer data pointer.
*
* Return:
*   None.
*
*******************************************************************************/
void RGB_dataProcess(uint8_t* dataPtr)
{   
    // For all 3 channels
    for (uint8_t i=0; i<3; i++)
    {
        // Take absolute value
        dataPtr[i] = Absolute_Value(dataPtr[i]);
        
        // Avoid variable overflow
        if (dataPtr[i] > 127)
        {
            dataPtr[i] = 127;
        }
        
        // Multiply by 2 to use full range of uint8
        dataPtr[i] *= 2;
        
        // Get negated value to drive common anode LED
        dataPtr[i] = ((uint8_t)dataPtr[i]);
    }
}


/*******************************************************************************
* Function Name: RGB_dataProcess
********************************************************************************
*
* Summary:
*   Given a window of IMU data (ordered X->Y->Z->X..) it computes the average of
*   that window.
*
* Parameters:  
*   Buffer data pointer, Empty 3 bytes buffer to store result, Size of window.
*
* Return:
*   None.
*
*******************************************************************************/
void Moving_Average(uint8_t* dataPtr, uint8_t* filtPtr, uint8_t windowSize)
{
    int16_t dataSum[3] = {0,0,0};
    
    // For all sample in window
    for (uint8_t i=0; i<windowSize; i++)
    {
        // For all 3 channels
        for (uint8_t j=0; j<3; j++)
        {
            // Sum the sample
            int8_t tmp = dataPtr[1+i*6+j*2];
            dataSum[j] += (int16_t)tmp;
        }
    }
    
    // For all 3 channels
    for (uint8_t i=0; i<3; i++)
    {
        // Assign window average value
        filtPtr[i] = (uint8_t)(dataSum[i]/windowSize);
    }
}


/*******************************************************************************
* Function Name: Absolute_Value
********************************************************************************
*
* Summary:
*   Light-weight function to get absolute value of int8_t variable without 
*   conditional jumps by setting the sign bit to zero.
*
* Parameters:  
*   8-bit raw value.
*
* Return:
*   8-bit absolute value.
*
*******************************************************************************/
uint8_t Absolute_Value(int8_t value)
{
    // Get sign bit
    const int8_t mask = value >> 7;
    
    // Compute abs value
    return (uint8_t)((value + mask) ^ mask);
}

/* [] END OF FILE */
