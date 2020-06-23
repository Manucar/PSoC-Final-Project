/* ========================================
 *
 * This header file defines an interface to 
 * enable the driving of the RGB LED.  
 * The LED interface can either turn the LED
 * on/off or enable the driving of the LED
 * to happen based on raw data incoming from
 * an inertial measurement unit.
 *
 * ========================================
*/


/* Header guard. */
#ifndef __RGB_DRIVER_H__
    
    #define __RGB_DRIVER_H__
    
    /* Project dependendcies. */
    #include "project.h"
    #include "stdio.h"
    
    /* Useful constants. */
    #define PWM_CYCLE_LENGTH    255
    #define PWM_COMPARE_STOP    0
    
    /* LED driver value. */
    uint8_t RGB_DataBuffer[3];
    
    /* Function prototype declaration. */
    void RGB_Init(void);
    void RGB_Stop(void);
    void RGB_Driver(uint8_t* dataPtr);
    void RGB_sendFlagNotify(uint8_t flag);
    void PWM_Driver(uint8* dataPtr);
    void RGB_dataProcess(uint8_t* dataPtr);
    void Moving_Average(uint8_t* dataPtr, uint8_t* filtPtr, uint8_t windowSize);
    uint8_t Absolute_Value(int8_t value);
    
#endif    

/* [] END OF FILE */
