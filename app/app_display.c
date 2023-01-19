#include "app_bsp.h"
#include "app_display.h"

/**
 * @brief  Variable for LCD Handle Structure definition
 */
LCD_HandleTypeDef hlcd;

/**
 * @brief  Variable for SPI Handle Structure definition
 */
static SPI_HandleTypeDef SpiHandle;

void Display_Init( void )
{
    SpiHandle.Instance                  = SPI2;
    SpiHandle.Init.Mode                 = SPI_MODE_MASTER;
    SpiHandle.Init.BaudRatePrescaler    = SPI_BAUDRATEPRESCALER_16;
    SpiHandle.Init.Direction            = SPI_DIRECTION_2LINES;
    SpiHandle.Init.CLKPhase             = SPI_PHASE_2EDGE;
    SpiHandle.Init.CLKPolarity          = SPI_POLARITY_HIGH;
    SpiHandle.Init.DataSize             = SPI_DATASIZE_8BIT;
    SpiHandle.Init.FirstBit             = SPI_FIRSTBIT_MSB;
    SpiHandle.Init.NSS                  = SPI_NSS_SOFT;
    SpiHandle.Init.CRCCalculation       = SPI_CRCCALCULATION_DISABLED;
    SpiHandle.Init.TIMode               = SPI_TIMODE_DISABLED;
    HAL_SPI_Init( &SpiHandle );

    hlcd.BklPin = GPIO_PIN_12;
    hlcd.BklPort = GPIOC;
    hlcd.CsPin = GPIO_PIN_15;
    hlcd.CsPort = GPIOB;
    hlcd.RsPin = GPIO_PIN_9;
    hlcd.RsPort = GPIOC;
    hlcd.RstPin = GPIO_PIN_8;
    hlcd.RstPort = GPIOC;
    hlcd.SpiHandler = &SpiHandle;
    HEL_LCD_Init( &hlcd );
}

void Display_Task( void )
{

}