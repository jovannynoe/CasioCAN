#include "app_bsp.h"
#include "app_display.h"
#include "app_serial.h"
#include "app_clock.h"
#include <stdlib.h>

#define STATE_IDLE 0
#define STATE_PRINT_TIME 1
#define STATE_PRINT_DATE 2

#define CLEAR 0x01u 

static void monthNumberToMonthWord( void );
static void weekDayNumberToWeekDayWord( void );

/**
 * @brief  Variable for LCD Handle Structure definition
 */
LCD_HandleTypeDef hlcd;

/**
 * @brief  Variable
 */
static uint8_t stateDisplay;

static char ClockMsgtm_mon[4];
static char ClockMsgtm_wday[3];

void Display_Init( void )
{
    static SPI_HandleTypeDef SpiHandle;

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

    stateDisplay = STATE_IDLE;
}

void Display_Task( void )
{
    char ClockMsgtm_hour[3];
    char ClockMsgtm_min[3];
    char ClockMsgtm_sec[3]; 
    char ClockMsgtm_mday[3];
    char ClockMsgtm_year[5];

    switch (stateDisplay)
    {
    case STATE_IDLE:

        stateDisplay = STATE_PRINT_TIME;
        break;

    case STATE_PRINT_TIME:

        if( ClockMsg.tm.tm_sec == 0u ){
            HEL_LCD_Command( &hlcd, CLEAR );
        }

        (void)itoa( ClockMsg.tm.tm_hour, ClockMsgtm_hour, 10 );
        (void)itoa( ClockMsg.tm.tm_min, ClockMsgtm_min, 10 );
        (void)itoa( ClockMsg.tm.tm_sec, ClockMsgtm_sec, 10 );

        HEL_LCD_SetCursor( &hlcd, 1, 3 );
        HEL_LCD_String( &hlcd, ClockMsgtm_hour ); 
        HEL_LCD_SetCursor( &hlcd, 1, 5 );
        HEL_LCD_Data( &hlcd, ':' );
        HEL_LCD_SetCursor( &hlcd, 1, 6 );
        HEL_LCD_String( &hlcd, ClockMsgtm_min );
        HEL_LCD_SetCursor( &hlcd, 1, 8 );
        HEL_LCD_Data( &hlcd, ':' );
        HEL_LCD_SetCursor( &hlcd, 1, 9 );
        HEL_LCD_String( &hlcd, ClockMsgtm_sec );

        stateDisplay = STATE_PRINT_DATE;
        
        break;

    case STATE_PRINT_DATE:
        monthNumberToMonthWord();
        weekDayNumberToWeekDayWord();

        (void)itoa( ClockMsg.tm.tm_mday, ClockMsgtm_mday, 10 );
        (void)itoa( ClockMsg.tm.tm_year, ClockMsgtm_year, 10 );

        HEL_LCD_SetCursor( &hlcd, 0, 1 );
        HEL_LCD_String( &hlcd, ClockMsgtm_mon );
        HEL_LCD_SetCursor( &hlcd, 0, 4 );
        HEL_LCD_Data( &hlcd, ',' );
        HEL_LCD_SetCursor( &hlcd, 0, 5 );
        HEL_LCD_String( &hlcd, ClockMsgtm_mday );
        HEL_LCD_SetCursor( &hlcd, 0, 8 );
        HEL_LCD_String( &hlcd, ClockMsgtm_year );
        HEL_LCD_SetCursor( &hlcd, 0, 13 );
        HEL_LCD_String( &hlcd, ClockMsgtm_wday );

        stateDisplay = STATE_IDLE;
        break;
    
    default:
        break;
    }
}

void monthNumberToMonthWord( void ){
    static uint8_t i;
    const uint8_t months[47] = "JAN FEB MAR APR MAY JUN JUL AUG SEP OCT NOV DEC";

    switch (ClockMsg.tm.tm_mon)
    {
    case 1:
        for( i = 0u; i < 3u; i++ ){
            ClockMsgtm_mon[i] = months[i];
        }
        break;

    case 2:
        for( i = 0u; i < 3u; i++ ){
            ClockMsgtm_mon[i] = months[i+4u];
        }
        break;

    case 3:
        for( i = 0u; i < 3u; i++ ){
            ClockMsgtm_mon[i] = months[i+8u];
        }
        break;

    case 4:
        for( i = 0u; i < 3u; i++ ){
            ClockMsgtm_mon[i] = months[i+12u];
        }
        break;

    case 5:
        for( i = 0u; i < 3u; i++ ){
            ClockMsgtm_mon[i] = months[i+16u];
        }
        break;

    case 6:
        for( i = 0u; i < 3u; i++ ){
            ClockMsgtm_mon[i] = months[i+20u];
        }
        break;

    case 7:
        for( i = 0u; i < 3u; i++ ){
            ClockMsgtm_mon[i] = months[i+24u];
        }
        break;

    case 8:
        for( i = 0u; i < 3u; i++ ){
            ClockMsgtm_mon[i] = months[i+28u];
        }
        break;

    case 9:
        for( i = 0u; i < 3u; i++ ){
            ClockMsgtm_mon[i] = months[i+32u];
        }
        break;

    case 10:
        for( i = 0u; i < 3u; i++ ){
            ClockMsgtm_mon[i] = months[i+36u];
        }
        break;

    case 11:
        for( i = 0u; i < 3u; i++ ){
            ClockMsgtm_mon[i] = months[i+40u];
        }
        break;

    case 12:
       for( i = 0u; i < 3u; i++ ){
            ClockMsgtm_mon[i] = months[i+44u];
        }
        break;

    default:
        break;
    }
}

void weekDayNumberToWeekDayWord( void ){
    static uint8_t i;
    const uint8_t weekDays[21] = "Mo Tu We Th Fr Sa Su";

    switch (ClockMsg.tm.tm_wday)
    {
    case RTC_WEEKDAY_MONDAY:
        for( i = 0u; i < 2u; i++ ){
            ClockMsgtm_wday[i] = weekDays[i];
        }
        break;

    case RTC_WEEKDAY_TUESDAY:
        for( i = 0u; i < 2u; i++ ){
            ClockMsgtm_wday[i] = weekDays[i+3u];
        }
        break;

    case RTC_WEEKDAY_WEDNESDAY:
        for( i = 0u; i < 2u; i++ ){
            ClockMsgtm_wday[i] = weekDays[i+6u];
        }
        break;

    case RTC_WEEKDAY_THURSDAY:
        for( i = 0u; i < 2u; i++ ){
            ClockMsgtm_wday[i] = weekDays[i+9u];
        }
        break;

    case RTC_WEEKDAY_FRIDAY:
        for( i = 0u; i < 2u; i++ ){
            ClockMsgtm_wday[i] = weekDays[i+12u];
        }
        break;

    case RTC_WEEKDAY_SATURDAY:
        for( i = 0u; i < 2u; i++ ){
            ClockMsgtm_wday[i] = weekDays[i+15u];
        }
        break;

    case RTC_WEEKDAY_SUNDAY:
        for( i = 0u; i < 2u; i++ ){
            ClockMsgtm_wday[i] = weekDays[i+18u];
        }
        break;

    default:
        break;
    }
}