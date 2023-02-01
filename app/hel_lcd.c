/**
 * @file    hel_lcd.c
 * @author  Jovanny Noé Casillas Franco
 * @brief   This file is the LCD driver.
 *
 * The driver was made with many functions to every thing to do, we have a function to initialize the LCD,
 * to send a command through SPI, to send a data and string through SPI, to set the cursor in the LCD, to
 * turn on or turn off the light and to set the contrast of 0 to 15 like you want.
 * 
 * @note    We need take in count that this driver is just to use the functions in another source file to
 * can print in the LCD some information. Also, you need configurate the SPI in another file when you want
 * use the functions and this driver was made only to SPI, not works with I2C or another peripheral.
 * 
 */
#include "app_bsp.h"
#include "hel_lcd.h"

/** 
  * @defgroup Commands Commands to every function in the LCD
  @{ */
#define WAKE_UP 0x30u                   /*!< Command to wake up */
#define FUNCTION_SET 0x39u              /*!< Command to set function */
#define INTERNAL_OSC_FREQUENCY 0x14u    /*!< Command to set the osc frequency */
#define POWER_CONTROL 0x56u             /*!< Command to set the power control */
#define FOLLOWER_CONTROL 0x6Du          /*!< Command to follower control */
#define CONTRAST 0x70u                  /*!< Command to set the contrast */
#define DISPLAY_ON 0x0Cu                /*!< Command to turn on the display */
#define ENTRY_MODE 0x06u                /*!< Command to be in entry mode */
#define CLEAR 0x01u                     /*!< Command to clear the LCD */
#define SET_CURSOR 0x80u                /*!< Command to can set the cursor */
#define ROW_ZERO 0x00u                  /*!< Command to set the cursor in position 0,0 */            
#define ROW_ONE 0x40u                   /*!< Command to set the cursor in position 1,0 */
/**
  @} */

/** 
  * @defgroup Contrast Set the contrast
  @{ */
#define CONTRAST_MAX 0x0Fu  /*!< To set the contrast to maximum */
#define CONTRAST_MID 0x07u  /*!< To set the contrast to middle */
#define CONTRAST_MIN 0x00u  /*!< To set the contrast to minimum */
/**
  @} */

/** 
  * @defgroup Backlight Defines to manage the backlight in the LCD
  @{ */
#define LCD_ON 1u       /*!< Turn on the backlight */
#define LCD_OFF 2u      /*!< Turn off the backlight */
#define LCD_TOGGLE 3u   /*!< Toggle the backlight */
/**
  @} */

/**
 * @brief   HEL LCD init function is to configurate the LCD through SPI functions.
 *
 * In this function we turn on the backlight, later we turn on the CS pin, turn off the RESET pin 
 * and turn on the RESET pin after of delay of 2 ms. After of 20 ms, wake up the LCD three times, 
 * and we use the command FUNCTION SET, we set the osc frequency, power control and follower
 * control. After of 200 ms, we set the contrast to MAX, turn on the display, we select the entry
 * mode and finally we clear the LCD.
 * 
 * @param   hlcd[out] Parameter that is used only to can use the LCD structure
 * 
 * @retval  None
 */
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
    HEL_LCD_Contrast( hlcd, CONTRAST_MAX );
    HEL_LCD_Command( hlcd, DISPLAY_ON );
    HEL_LCD_Command( hlcd, ENTRY_MODE );
    HEL_LCD_Command( hlcd, CLEAR );
    HAL_Delay(1);
}

/**
 * @brief   HEL LCD Mpsinit function is to configurate the pins in the LCD.
 *
 * In this function only we declarate the __weak function because is used in msps source file
 * to configurate the pins used to can we have a communication. 
 * 
 * @param   hlcd[out] Parameter that is used only in the function because it is a requirement
 * 
 * @retval  None
 */
__weak void HEL_LCD_MspInit( LCD_HandleTypeDef *hlcd )
{
    *hlcd = *hlcd;
}

/**
 * @brief   HEL LCD Command function is to can transmit commands through SPI.
 *
 * In this function we turn off RS pin, turn off CS pin and we transmit through SPI a command
 * that indicates in cmd variable with 5000 timeout. Finally, We turn on the CS pin and wait
 * 30 us to complete the transmition of the command. 
 * 
 * @param   hlcd[out] Parameter that is used only to can use the LCD structure
 * @param   cmd[out] Parameter that is used to know what is the command and we process
 * 
 * @retval  None
 */
void HEL_LCD_Command( LCD_HandleTypeDef *hlcd, uint8_t cmd )
{
    HAL_GPIO_WritePin( hlcd->RsPort, hlcd->RsPin, RESET );  /*RS = 0*/
    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, RESET );  /*CS = 0*/
    HAL_SPI_Transmit( hlcd->SpiHandler, &cmd, sizeof(cmd), 5000 );
    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, SET );    /*CS = 1*/
    HAL_Delay(1);
}

/**
 * @brief   HEL LCD Data function is to can transmit data through SPI.
 *
 * In this function we turn on RS pin, turn off CS pin and we transmit through SPI a data
 * that indicates in data variable with 5000 timeout. Finally, We turn on the CS pin and wait
 * 30 us to complete the transmition of the data. 
 * 
 * @param   hlcd[out] Parameter that is used only to can use the LCD structure
 * @param   data[out] Parameter that is used to know what is the data and we process
 * 
 * @retval  None
 */
void HEL_LCD_Data( LCD_HandleTypeDef *hlcd, uint8_t data )
{
    HAL_GPIO_WritePin( hlcd->RsPort, hlcd->RsPin, SET );    /*RS = 1*/
    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, RESET );  /*CS = 0*/
    HAL_SPI_Transmit( hlcd->SpiHandler, &data, sizeof(data), 5000 );
    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, SET );    /*CS = 1*/
    HAL_Delay(1);
}

/**
 * @brief   HEL LCD String function is to can transmit string through SPI.
 *
 * In this function we resume the HEL_LCD_Data function to transmit the complete string through 
 * a while, when the pointer detect '\0' caracter not transmit another data, only before the '\0'
 * caracter.
 * 
 * @param   hlcd[out] Parameter that is used only to can use the LCD structure
 * @param   str[out] Parameter that is used to pointer to string and we process
 * 
 * @retval  None
 */
void HEL_LCD_String( LCD_HandleTypeDef *hlcd, char *str )
{
    uint8_t i = 0u;

    while ( str[i] != '\0' )
    {
        HEL_LCD_Data( hlcd, str[i] );
        i++;
    }
}

/**
 * @brief   HEL LCD Set Cursor function is to set the cursor in the LCD.
 *
 * In this function we need to know what are the DD Ram Address in the LCD, and we know,
 * for this, we initialize the variable rowZero with the first position of the LCD, this 
 * position is the 0 in row and 0 in column, and the next row is the position 1 in row and 
 * 0 in column, but in DD Ram Address is 0x40, so, if the row is 0 we need to make a sum 
 * with the const rowZero + the column and if the row is 1 or upper, we make a sum with 
 * the const rowOne + column and with the HEL_LCD_Command we set the cursor in the correct
 * position, also, we use an OR between SET CURSOR(0x80) and ddRamAddreess variable.
 * The OR is very important because wihout OR we not set the cursor, SET CURSOR(0x80) is
 * very important because is the command that the LCD understand that will set the cursor.
 * 
 * @param   hlcd[out] Parameter that is used only to can use the LCD structure
 * @param   row[out] Parameter that is used to know the row to set
 * @param   col[out] Parameter that is used to know the column to set
 * 
 * @retval  None
 * 
 * @note    The parameter row only accept 0 and 1 and parameter col only accept values between 
 *          0 to 15
 */
void HEL_LCD_SetCursor( LCD_HandleTypeDef *hlcd, uint8_t row, uint8_t col )
{
    uint8_t ddRamAddress;

    if( row == 0u ){
        ddRamAddress = (ROW_ZERO + col);
    }
    else{
        ddRamAddress = (ROW_ONE + col);
    }

    HEL_LCD_Command( hlcd, SET_CURSOR | ddRamAddress );   
}

/**
 * @brief   HEL LCD Backlight function is to can manage the backlight.
 *
 * In this function we have three states, the LCD_ON state is to turn on the backlight
 * the LCD_OFF state is to turn off the backlight and the LCD_TOGGLE state is to 
 * toggle the backlight.
 * 
 * @param   hlcd[out] Parameter that is used only to can use the LCD structure
 * @param   state[out] Parameter that is used to select the state in backlight
 * 
 * @retval  None
 * 
 * @note    The values that accept the state variable is betwen 1 to 3, 1 to turn on the
 *          backlight, 2 to turn off the backlight and 3 to toggle the backlight.
 */
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

/**
 * @brief   HEL LCD Contrast function is to can manage the contrast.
 *
 * In this function we use the HEL_LCD_Command to set the contrast with the command
 * CONTRAST(0x70u) that this is very important because is the command that the LCD 
 * understand that is the contrast, so, with the contrast variable we adjust the contrast
 * to taste.
 * Also, we use a OR because is to can manipulate the contrast, in fact, the contrast in 
 * default mode is 0x70 | 0x00, but with this function, we can change the contrast between
 * 0 to 15, because only we have 4 bits for the resolution.
 * 
 * @param   hlcd[out] Parameter that is used only to can use the LCD structure
 * @param   contrast[out] Parameter that is used to select the contrast between 0 to 15
 * 
 * @retval  None
 * 
 */
void HEL_LCD_Contrast( LCD_HandleTypeDef *hlcd, uint8_t contrast )
{
    HEL_LCD_Command( hlcd, CONTRAST | contrast );  
}