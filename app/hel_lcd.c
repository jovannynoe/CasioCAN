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

#define WAKE_UP 0x30u
#define FUNCTION_SET 0x39u
#define INTERNAL_OSC_FREQUENCY 0x14u
#define POWER_CONTROL 0x56u
#define FOLLOWER_CONTROL 0x6Du
#define CONTRAST 0x70u
#define DISPLAY_ON 0x0Cu
#define CURSOR_ON 0x0Eu
#define ENTRY_MODE 0x06u
#define CLEAR 0x01u 

#define LCD_ON 1u
#define LCD_OFF 2u
#define LCD_TOGGLE 3u

void HEL_LCD_Init( LCD_HandleTypeDef *hlcd )
{
    HEL_LCD_MspInit( hlcd );

    HEL_LCD_Backlight( hlcd, LCD_ON );
    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, SET );        //CS = 1
    HAL_GPIO_WritePin( hlcd->RstPort, hlcd->RstPin, RESET );    //RST = 0
    HAL_Delay(2);
    HAL_GPIO_WritePin( hlcd->RstPort, hlcd->RstPin, SET );      //RST = 1
    HAL_Delay(20);
    HEL_LCD_Command( hlcd, WAKE_UP );
    HAL_Delay(2);
    HEL_LCD_Command( hlcd, WAKE_UP );
    HEL_LCD_Command( hlcd, WAKE_UP );
    HEL_LCD_Command( hlcd, FUNCTION_SET );
    HEL_LCD_Command( hlcd, INTERNAL_OSC_FREQUENCY );
    HEL_LCD_Command( hlcd, POWER_CONTROL );
    HEL_LCD_Command( hlcd, FOLLOWER_CONTROL );
    HAL_Delay(200);
    HEL_LCD_Command( hlcd, CONTRAST );
    HEL_LCD_Command( hlcd, DISPLAY_ON );
    HEL_LCD_Command( hlcd, ENTRY_MODE );
    HEL_LCD_Command( hlcd, CLEAR );
    HAL_Delay(1);
}

__weak void HEL_LCD_MspInit( LCD_HandleTypeDef *hlcd )
{
    *hlcd = *hlcd;
}

void HEL_LCD_Command( LCD_HandleTypeDef *hlcd, uint8_t cmd )
{
    HAL_GPIO_WritePin( hlcd->RsPort, hlcd->RsPin, RESET );  /*RS = 0*/
    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, RESET );  /*CS = 0*/
    HAL_SPI_Transmit( hlcd->SpiHandler, &cmd, sizeof(cmd), 5000 );
    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, SET );    /*CS = 1*/
    HAL_Delay(1);
}

void HEL_LCD_Data( LCD_HandleTypeDef *hlcd, uint8_t data )
{
    HAL_GPIO_WritePin( hlcd->RsPort, hlcd->RsPin, SET );    /*RS = 1*/
    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, RESET );  /*CS = 0*/
    HAL_SPI_Transmit( hlcd->SpiHandler, &data, sizeof(data), 5000 );
    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, SET );    /*CS = 1*/
    HAL_Delay(1);
}

void HEL_LCD_String( LCD_HandleTypeDef *hlcd, char *str )
{
    while ( *str != '\0' )
    {
        HEL_LCD_Data( hlcd, *str );
        str++;
    }
}

void HEL_LCD_SetCursor( LCD_HandleTypeDef *hlcd, uint8_t row, uint8_t col )
{
    const uint8_t rowZero[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    const uint8_t rowOne[16] = { 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F };
    uint8_t ddRamAddress;

    if( row == 0u ){
        ddRamAddress = rowZero[col];
    }
    else{
        ddRamAddress = rowOne[col];
    }

    HEL_LCD_Command( hlcd, 0x80u | ddRamAddress );
    /*Row[0:1] & Col[0:15]*/   
}

void HEL_LCD_Backlight( LCD_HandleTypeDef *hlcd, uint8_t state )
{
    switch (state)
    {
    case LCD_ON:
        HAL_GPIO_WritePin( hlcd->BklPort, hlcd->BklPin, SET );
        break;

    case LCD_OFF:
        HAL_GPIO_WritePin( hlcd->BklPort, hlcd->BklPin, RESET );
        break;

    case LCD_TOGGLE:
        HAL_GPIO_TogglePin( hlcd->BklPort, hlcd->BklPin );
        break;
    
    default:
        break;
    }
}

void HEL_LCD_Contrast( LCD_HandleTypeDef *hlcd, uint8_t contrast )
{

}