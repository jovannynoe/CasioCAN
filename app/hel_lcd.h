/**
 * @file    hel_lcd.h
 * @author  Jovanny Noé Casillas Franco
 * @brief   Header file of LCD
 *
 * The Clock header file is to make the structures and we use in differents files, also, to declarate the functions prototypes
 * that we are using in the source file. 
 * 
 * @note    We need take in count that we have the extern with the declaration of structures because we use MISRA.
 * 
 */
#ifndef HEL_LCD_H__
#define HEL_LCD_H__

typedef struct
{
    SPI_HandleTypeDef   *SpiHandler; /* SPI handler address of the spi to use with the LCD*/

    GPIO_TypeDef        *RstPort;   /*port where the pin to control the LCD reset pin is*/

    uint32_t            RstPin; /*pin to control the LCD reset pin */

    GPIO_TypeDef        *RsPort; /*port where the pin to control the LCD RS pin*/

    uint32_t            RsPin;  /*pin to control the LCD RS pin */

    GPIO_TypeDef        *CsPort; /*port where the pin to control the LCD chip select is*/

    uint32_t            CsPin;  /*pin to control the LCD chip select pin */

    GPIO_TypeDef        *BklPort; /*port where the pin to control the LCD backlight is*/

    uint32_t            BklPin;  /*pin to control the LCD backlight pin */
    //agregar más elementos si se requieren
} LCD_HandleTypeDef;

extern void HEL_LCD_Init( LCD_HandleTypeDef *hlcd );
extern void HEL_LCD_MspInit( LCD_HandleTypeDef *hlcd );
extern void HEL_LCD_Command( LCD_HandleTypeDef *hlcd, uint8_t cmd );
extern void HEL_LCD_Data( LCD_HandleTypeDef *hlcd, uint8_t data );
extern void HEL_LCD_String( LCD_HandleTypeDef *hlcd, char *str );
extern void HEL_LCD_SetCursor( LCD_HandleTypeDef *hlcd, uint8_t row, uint8_t col );
extern void HEL_LCD_Backlight( LCD_HandleTypeDef *hlcd, uint8_t state );
extern void HEL_LCD_Contrast( LCD_HandleTypeDef *hlcd, uint8_t contrast );

#endif