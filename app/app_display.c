/**
 * @file    app_display.c
 * @author  Jovanny Noé Casillas Franco
 * @brief   This driver is to can use the LCD.
 *
 * The driver works with a state machines, only three states. The fisrt state is the idle state in where 
 * we go to the print time state in where we print the time in the LCD, later, when we print the time, we
 * go to print date state in where we print the date and the week day in the LCD, finally we go to idle
 * state again.
 * 
 * @note    We need take in count that now we only print the time and date in the LCD.
 * 
 */
#include "app_bsp.h"
#include "app_display.h"
#include "app_serial.h"
#include "app_clock.h"
#include <stdlib.h>

/** 
  * @defgroup Months Months of the year
  @{ */
#define JAN 1u  /*!< January month */
#define FEB 2u  /*!< February month */
#define MAR 3u  /*!< March month */
#define APR 4u  /*!< April month */
#define MAY 5u  /*!< May month */
#define JUN 6u  /*!< June month */
#define JUL 7u  /*!< July month */
#define AUG 8u  /*!< August month */
#define SEP 9u  /*!< September month */
#define OCT 10u /*!< October month */
#define NOV 11u /*!< November month */
#define DEC 12u /*!< December month */
/**
  @} */

/** 
  * @defgroup States States to we know what is the state without errors
  @{ */
#define STATE_IDLE 0        /*!< Idle state */
#define STATE_PRINT_TIME 1  /*!< State to print the time in LCD */
#define STATE_PRINT_DATE 2  /*!< State to print the time in LCD */
#define STATE_RECEPTION 3   /*!< State to read messages */ 
/**
  @} */

/** 
  * @defgroup Commands Commands to can use the LCD
  @{ */
#define CLEAR 0x01u /*!< Command to clear the LCD */ 
/**
  @} */

static void monthNumberToMonthWord( void );
static void weekDayNumberToWeekDayWord( void );

static APP_MsgTypeDef DisplayMsg;

/**
 * @brief  Variable for LCD Handle Structure definition
 */
LCD_HandleTypeDef hlcd;

/**
 * @brief  Global variable because is used in two functions to manage the state machine
 */
static uint8_t stateDisplay;

/**
 * @brief  Global variable because is used in two functions to manipulate the months in array
 */
static char DisplayMsgtm_mon[4];

/**
 * @brief  Global variable because is used in two functions to manipulate the week days in array
 */
static char DisplayMsgtm_wday[3];

/**
 * @brief  Global variable to wait a message per 50 ms
 */
static uint32_t tickstartWaitMessage;

/**
 * @brief   Display init function is to configurate the SPI peripheral to can use the LCD.
 *
 * In this function we go to configurate the SPI peripheral to can use the LCD. We configurate the
 * SPI2 because the pins used to can communicate the LCD through SPI are in SPI2. Later, we
 * configurate in master mode, with a prescaler of 16 because the SCK is 64MHz and the LCD works to
 * 4MHz so, to can transmit the information to LCD we are need the same CLK. The communication is 
 * full-duplex, polarity of clock high and phase on falling edge.
 * The direction is bidirectional with 2 lines, the 8 bits datasize and whether data transfers 
 * start from MSB.
 * 
 * @retval  None
 */
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

    stateDisplay = STATE_RECEPTION;

    tickstartWaitMessage = HAL_GetTick();
}

/**
 * @brief   Display task function is the part to print time and date.
 *
 * In this function, first we stay in the state idle, later we go to state print time in where 
 * we print the time in the LCD, but, first we convert the hours, minutes and seconds to string 
 * with the function itoa to can transmit to LCD the information. After that, we go to state 
 * print date in where we convert the day of the month and the year with te function itoa to 
 * string to can use the function HEL_LCD_String and can print in the LCD, also, we use two 
 * functions to convert the day of the week and the month to word. Finally, we go to state idle.
 *
 * @retval  None
 *
 */
void Display_Task( void )
{
    char DisplayMsgtm_hour[3];
    char DisplayMsgtm_min[3];
    char DisplayMsgtm_sec[3]; 
    char DisplayMsgtm_mday[3];
    char DisplayMsgtm_year[5];

    if( (HAL_GetTick() - tickstartWaitMessage) >= 100 ){
        tickstartWaitMessage = HAL_GetTick();

        while( stateDisplay != STATE_IDLE ){

            switch (stateDisplay)
            {
            case STATE_IDLE:

                break;

            case STATE_RECEPTION:
                if( HIL_QUEUE_IsEmpty( &DisplayQueue ) == 0 ){

                    HIL_QUEUE_Read( &DisplayQueue, &DisplayMsg );

                    stateDisplay = STATE_PRINT_TIME;
                }
                else{
                    stateDisplay = STATE_IDLE;
                }
                break;

            case STATE_PRINT_TIME:

                if( DisplayMsg.tm.tm_sec == 0u ){
                    HEL_LCD_Command( &hlcd, CLEAR );
                }

                (void)itoa( DisplayMsg.tm.tm_hour, DisplayMsgtm_hour, 10 );
                (void)itoa( DisplayMsg.tm.tm_min, DisplayMsgtm_min, 10 );
                (void)itoa( DisplayMsg.tm.tm_sec, DisplayMsgtm_sec, 10 );

                HEL_LCD_SetCursor( &hlcd, 1, 3 );
                HEL_LCD_String( &hlcd, DisplayMsgtm_hour ); 
                HEL_LCD_SetCursor( &hlcd, 1, 5 );
                HEL_LCD_Data( &hlcd, ':' );
                HEL_LCD_SetCursor( &hlcd, 1, 6 );
                HEL_LCD_String( &hlcd, DisplayMsgtm_min );
                HEL_LCD_SetCursor( &hlcd, 1, 8 );
                HEL_LCD_Data( &hlcd, ':' );
                HEL_LCD_SetCursor( &hlcd, 1, 9 );
                HEL_LCD_String( &hlcd, DisplayMsgtm_sec );

                stateDisplay = STATE_PRINT_DATE;
        
                break;

            case STATE_PRINT_DATE:
                monthNumberToMonthWord();
                weekDayNumberToWeekDayWord();

                (void)itoa( DisplayMsg.tm.tm_mday, DisplayMsgtm_mday, 10 );
                (void)itoa( DisplayMsg.tm.tm_year, DisplayMsgtm_year, 10 );

                HEL_LCD_SetCursor( &hlcd, 0, 1 );
                HEL_LCD_String( &hlcd, DisplayMsgtm_mon );
                HEL_LCD_SetCursor( &hlcd, 0, 4 );
                HEL_LCD_Data( &hlcd, ',' );
                HEL_LCD_SetCursor( &hlcd, 0, 5 );
                HEL_LCD_String( &hlcd, DisplayMsgtm_mday );
                HEL_LCD_SetCursor( &hlcd, 0, 8 );
                HEL_LCD_String( &hlcd, DisplayMsgtm_year );
                HEL_LCD_SetCursor( &hlcd, 0, 13 );
                HEL_LCD_String( &hlcd, DisplayMsgtm_wday );

                stateDisplay = STATE_IDLE;
                break;
    
            default:
                break;
            }
        }
    }
}

/**
 * @brief   Month number to month word convert function.
 *
 * In this function, we ask the month in number and we convert the month to word of three leters
 * and we save the month in word in a char of four elements. For example, if the month is 2(FEB), 
 * so, we convert the month to word, we save the word FEB in a char called DisplayMsgtm_mon to can
 * print the name in place that of the number.
 *
 * @retval  None
 *
 */
void monthNumberToMonthWord( void ){
    static uint8_t i;
    const uint8_t months[47] = "JAN FEB MAR APR MAY JUN JUL AUG SEP OCT NOV DEC";

    switch (DisplayMsg.tm.tm_mon)
    {
    case JAN:
        for( i = 0u; i < 3u; i++ ){
            DisplayMsgtm_mon[i] = months[i];
        }
        break;

    case FEB:
        for( i = 0u; i < 3u; i++ ){
            DisplayMsgtm_mon[i] = months[i+4u];
        }
        break;

    case MAR:
        for( i = 0u; i < 3u; i++ ){
            DisplayMsgtm_mon[i] = months[i+8u];
        }
        break;

    case APR:
        for( i = 0u; i < 3u; i++ ){
            DisplayMsgtm_mon[i] = months[i+12u];
        }
        break;

    case MAY:
        for( i = 0u; i < 3u; i++ ){
            DisplayMsgtm_mon[i] = months[i+16u];
        }
        break;

    case JUN:
        for( i = 0u; i < 3u; i++ ){
            DisplayMsgtm_mon[i] = months[i+20u];
        }
        break;

    case JUL:
        for( i = 0u; i < 3u; i++ ){
            DisplayMsgtm_mon[i] = months[i+24u];
        }
        break;

    case AUG:
        for( i = 0u; i < 3u; i++ ){
            DisplayMsgtm_mon[i] = months[i+28u];
        }
        break;

    case SEP:
        for( i = 0u; i < 3u; i++ ){
            DisplayMsgtm_mon[i] = months[i+32u];
        }
        break;

    case OCT:
        for( i = 0u; i < 3u; i++ ){
            DisplayMsgtm_mon[i] = months[i+36u];
        }
        break;

    case NOV:
        for( i = 0u; i < 3u; i++ ){
            DisplayMsgtm_mon[i] = months[i+40u];
        }
        break;

    case DEC:
       for( i = 0u; i < 3u; i++ ){
            DisplayMsgtm_mon[i] = months[i+44u];
        }
        break;

    default:
        break;
    }
}

/**
 * @brief   .Weekday number to weekday word convert function.
 *
 * In this function, we ask the weekday in number and we convert the weekday to word of two leters
 * and we save the weekday in word in a char of three elements. For example, if the weekday is 
 * 1(RTC_WEEKDAY_MONDAY), so, we convert the weekday to word, we save the word Mo in a char called 
 * DisplayMsgtm_wday to can print the name in place that of the number.
 *
 * @retval  None
 *
 */
void weekDayNumberToWeekDayWord( void ){
    static uint8_t i;
    const uint8_t weekDays[21] = "Mo Tu We Th Fr Sa Su";

    switch (DisplayMsg.tm.tm_wday)
    {
    case RTC_WEEKDAY_MONDAY:
        for( i = 0u; i < 2u; i++ ){
            DisplayMsgtm_wday[i] = weekDays[i];
        }
        break;

    case RTC_WEEKDAY_TUESDAY:
        for( i = 0u; i < 2u; i++ ){
            DisplayMsgtm_wday[i] = weekDays[i+3u];
        }
        break;

    case RTC_WEEKDAY_WEDNESDAY:
        for( i = 0u; i < 2u; i++ ){
            DisplayMsgtm_wday[i] = weekDays[i+6u];
        }
        break;

    case RTC_WEEKDAY_THURSDAY:
        for( i = 0u; i < 2u; i++ ){
            DisplayMsgtm_wday[i] = weekDays[i+9u];
        }
        break;

    case RTC_WEEKDAY_FRIDAY:
        for( i = 0u; i < 2u; i++ ){
            DisplayMsgtm_wday[i] = weekDays[i+12u];
        }
        break;

    case RTC_WEEKDAY_SATURDAY:
        for( i = 0u; i < 2u; i++ ){
            DisplayMsgtm_wday[i] = weekDays[i+15u];
        }
        break;

    case RTC_WEEKDAY_SUNDAY:
        for( i = 0u; i < 2u; i++ ){
            DisplayMsgtm_wday[i] = weekDays[i+18u];
        }
        break;

    default:
        break;
    }
}