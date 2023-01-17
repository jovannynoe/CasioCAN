/**
 * @file    hel_lcd.c
 * @author  Jovanny Noé Casillas Franco
 * @brief   **This file is to receive messages through CANdo and save the values in structure**
 *
 * The driver works through state machines where the state idle is the main state, later, we have the another state called message,
 * in this state we verifying what is the type of message, if the message is a time, date, alarm or none. We know what type is because
 * in the second byte of the CANdo we have to write 0x01 for time, 0x02 for date and 0x03 for alarm, if this byte is different is a
 * error, so, here is when the error state appears, this state just come back a message to CANdo with a message that indicate an error.
 * When it is a error message we return to CANdo the same CAN-TP but the others bytes are 0xAA, but if the message is ok we return
 * to CANdo the same CANT-TP but the others bytes are 0x55.
 * 
 * The another state is the time state, in this state we validate if the values are in the range of 24 hours, if the values are ok,
 * so, we go to ok state and in this state we save the values in a structure. The date state validate if the values are correct we 
 * go to ok state and we save the values in the same structure. Finally, we have the alarm state, in this state just we validate 
 * the hour and the minutes, if everything is ok, we go to ok state and we save the values in the same structure.
 * 
 * @note    We need take in count that the range of the year in date is just between 1900 to 2100.
 *          
 */
#include "app_bsp.h"
#include "hel_lcd.h"

void HEL_LCD_Init( LCD_HandleTypeDef *hlcd )
{

}

void HEL_LCD_MspInit( LCD_HandleTypeDef *hlcd )
{

}

void HEL_LCD_Command( LCD_HandleTypeDef *hlcd, uint8_t cmd )
{

}

void HEL_LCD_Data( LCD_HandleTypeDef *hlcd, uint8_t data )
{

}

void HEL_LCD_String( LCD_HandleTypeDef *hlcd, char *str )
{

}

void HEL_LCD_SetCursor( LCD_HandleTypeDef *hlcd, uint8_t row, uint8_t col )
{
    
}

void HEL_LCD_Backlight( LCD_HandleTypeDef *hlcd, uint8_t state )
{

}

void HEL_LCD_Contrast( LCD_HandleTypeDef *hlcd, uint8_t contrast )
{

}