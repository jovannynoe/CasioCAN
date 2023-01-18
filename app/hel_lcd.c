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
#include <stdio.h>

#define WAKE_UP 0x30u
#define FUNCTION_SET 0x39u
#define INTERNAL_OSC_FREQUENCY 0x14u
#define POWER_CONTROL 0x56u
#define FOLLOWER_CONTROL 0x6Du
#define CONTRAST 0x70u
#define DISPLAY_ON 0x0Cu
#define ENTRY_MODE 0x06u
#define CLEAR 0x01u

#define LCD_ON
#define LCD_OFF
#define LCD_TOGGLE

void HEL_LCD_Init( LCD_HandleTypeDef *hlcd )
{
    HEL_LCD_MspInit( hlcd );

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
    /*Empty*/
}

void HEL_LCD_Command( LCD_HandleTypeDef *hlcd, uint8_t cmd )
{
    HAL_GPIO_WritePin( hlcd->RsPort, hlcd->RsPin, RESET );  /*RS = 0*/
    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, RESET );  /*CS = 0*/
    HAL_SPI_Transmit( hlcd->SpiHandler, &cmd, sizeof( cmd ), 5000 );
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
    const uint8_t rowZero[16] = { 0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u, 0x08u, 0x09u, 0x0Au, 0x0Bu, 0x0Cu, 0x0Du, 0x0Eu, 0x0Fu };
    const uint8_t rowOne[16] = { 0x40u, 0x41u, 0x42u, 0x43u, 0x44u, 0x45u, 0x46u, 0x47u, 0x48u, 0x49u, 0x4Au, 0x4Bu, 0x4Cu, 0x4Du, 0x4Eu, 0x4Fu };
    uint8_t ddRamAddress;

    if( row == 0 ){
        ddRamAddress = rowZero[col];
    }
    else if( row == 1 ){
        ddRamAddress = rowOne[col];
    }

    HEL_LCD_Command( hlcd, 0x80 | ddRamAddress );
    /*Row[0:1] & Col[0:15]*/   
}

void HEL_LCD_Backlight( LCD_HandleTypeDef *hlcd, uint8_t state )
{

}

void HEL_LCD_Contrast( LCD_HandleTypeDef *hlcd, uint8_t contrast )
{

}