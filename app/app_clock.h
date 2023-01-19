/**
 * @file    app_clock.h
 * @author  Jovanny Noé Casillas Franco
 * @brief   **Header file of Clock**
 *
 * The Clock header file is to make the structures and we use in differents files, also, to declarate the functions prototypes
 * that we are using in the source file. 
 * 
 * @note    We need take in count that we have the extern with the declaration of structures because we use MISRA.
 * 
 */
#ifndef APP_CLOCK_H__
#define APP_CLOCK_H__

/**
  * @brief   Structure to use this words in place of number to more explicit
  */
typedef enum
{
    SERIAL_MSG_NONE = 0,    /*!< Used to initialize the value with 0 and when there isn't a message */
    
    SERIAL_MSG_TIME,        /*!< Used to use a word in place that a number to more explicit */

    SERIAL_MSG_DATE,        /*!< Used to use a word in place that a number to more explicit */

    SERIAL_MSG_ALARM        /*!< Used to use a word in place that a number to more explicit */
    
}APP_Messages;

extern APP_Messages typeMessage;
extern WWDG_HandleTypeDef hwwdg;
extern GPIO_InitTypeDef GPIO_InitStruct;

extern void Clock_Init( void );
extern void Clock_Task( void );
extern void LCD_Init( void );

#endif