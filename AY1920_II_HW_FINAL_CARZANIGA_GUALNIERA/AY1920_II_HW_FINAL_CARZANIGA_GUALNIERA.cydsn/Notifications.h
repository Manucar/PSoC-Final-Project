/* ========================================
 *
 * This header file contains the utility
 * function prototypes that enable user
 * notifications using the on-board LED
 * as well as taking user inputs through
 * the position of a potentiometer knob.
 *
 * ========================================
*/


/* Header guard. */
#ifndef __NOTIFICATIONS_H__
    
    #define __NOTIFICATIONS_H__
    
    /* Project dependencies. */
    #include "project.h"

    /* Useful constants. */
    #define POT_MID_RANGE   127
    #define POT_DEAD_BAND   POT_MID_RANGE/2

    /* Potentiometer flag. */
    uint8_t POT_Read_Value(uint8_t oldFlag);
    
    /* Function prototypes. */
    void LED_Notify_Stop(void);
    void LED_Notify_Start(void);
    void LED_Notify_Config(void);
    uint8_t POT_Read_Value(uint8_t oldFlag);

#endif


/* [] END OF FILE */
