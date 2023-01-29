/**
 * @file    app_clock.c
 * @author  Jovanny Noé Casillas Franco
 * @brief   This file is to can print the time.
 *
 * The driver works through state machines where the state idle is the main state, later, we have the others states called show time,
 * show date and show alarm that this are the states that print the time, date and alarm through of RTC peripheral. After that, we
 * have the others states called change time, change date and change alarm that this states are to change the values through the
 * structure created in the serial header file, since these values are processed in this another file. 
 * 
 * @note    We need take in count that this proyect just show the alarm but haven't an alert when the alarm is active, it just 
 *          keep showing the alarm.
 * 
 */
#include "app_bsp.h"
#include "app_clock.h"
#include "app_serial.h"
#include "app_display.h"

/** 
  * @defgroup Buffer Maximum size to buffer used
  @{ */
#define BUFFER_MAX_SIZE 90u
/**
  @} */

/** 
  * @defgroup Numbers Define of unsigned numbers
  @{ */
#define ONE_HUNDRED 100u    /*!< Used to divide the year in two parts */   
/**
  @} */

/** 
  * @defgroup RTC Defines to configurate RTC
  @{ */
#define RTC_ASYNCH_PREDIV 0x7F  /*!< Specifies the RTC Asynchronous Predivider value */
#define RTC_SYNCH_PREDIV 0x0F9  /*!< Specifies the RTC Synchronous Predivider value */
/**
  @} */

/** 
  * @defgroup States States to we know what is the state without errors
  @{ */
#define STATE_IDLE 0u               /*!< Idle state */
#define STATE_GET_TIME_AND_DATE 1u  /*!< State to save the time in another strcuture */
#define STATE_GET_ALARM 3u          /*!< State to save the alarm in another strcuture */
#define STATE_CHANGE_TIME 4u        /*!< State to change the values in time */
#define STATE_CHANGE_DATE 5u        /*!< State to change the values in date */
#define STATE_CHANGE_ALARM 6u       /*!< State to change the values in alarm */
#define STATE_RECEPTION 7u          /*!< State to read messages */ 
/**
  @} */

/**
 * @brief  APP MSG Structure definition to use the buffer between Clock and Display
 */
APP_MsgTypeDef bufferDisplay[BUFFER_MAX_SIZE];

/**
 * @brief  Variable for QUEUE Handle Structure definition
 */
QUEUE_HandleTypeDef DisplayQueue;

/**
 * @brief  Variable for RTC Handle Structure definition
 */
static RTC_HandleTypeDef hrtc;

/**
 * @brief  Variable for RTC Time structure definition
 */
static RTC_TimeTypeDef sTime = {0};

/**
 * @brief  Variable for RTC Date structure definition
 */
static RTC_DateTypeDef sDate = {0};

/**
 * @brief  Variable for RTC Alarm structure definition
 */
static RTC_AlarmTypeDef sAlarm = {0};

/**
 * @brief  Global variable because is used in two functions
 */
static uint64_t tickstartShowTime;

/**
 * @brief  Global variable to wait a message per 50 ms
 */
static uint64_t tickstartClockFile;

static GPIO_InitTypeDef GPIO_InitStructTest; 

/**
 * @brief   Clock init function is to configurate the RTC peripheral.
 *
 * In this function we go to configurate the RTC peripheral, also the 
 * time, date and alarm to we can use in the clock task function. 
 * Finally we have the tickstart to initialize for the first time to 
 * go to the next state called show time and we go to idle state.
 *
 * @retval  None
 */
void Clock_Init( void ){

    DisplayQueue.Buffer = (void *)bufferDisplay;
    DisplayQueue.Elements = BUFFER_MAX_SIZE;
    DisplayQueue.Size = sizeof( APP_MsgTypeDef );
    HIL_QUEUE_Init( &DisplayQueue );

    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
    hrtc.Init.SynchPrediv = RTC_SYNCH_PREDIV;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    HAL_RTC_Init( &hrtc );

    sTime.Hours = 0x23;
    sTime.Minutes = 0x59;
    sTime.Seconds = 0x30;
    sTime.SubSeconds = 0x00;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    HAL_RTC_SetTime( &hrtc, &sTime, RTC_FORMAT_BCD );

    sDate.WeekDay = RTC_WEEKDAY_SATURDAY;
    sDate.Month = RTC_MONTH_APRIL;
    sDate.Date = 0x30;
    sDate.Year = 0x22;
    HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BCD );

    sAlarm.AlarmTime.Hours = 0x00;
    sAlarm.AlarmTime.Minutes = 0x00;
    sAlarm.AlarmTime.Seconds = 0x0;
    sAlarm.AlarmTime.SubSeconds = 0x0;
    sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
    sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
    sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
    sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    sAlarm.AlarmDateWeekDay = 0x1;
    sAlarm.Alarm = RTC_ALARM_A;
    HAL_RTC_SetAlarm( &hrtc, &sAlarm, RTC_FORMAT_BCD );

    tickstartShowTime = HAL_GetTick();
    tickstartClockFile = HAL_GetTick();

    __GPIOC_CLK_ENABLE();

    GPIO_InitStructTest.Pin  = 0xFF;
    GPIO_InitStructTest.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructTest.Pull = GPIO_NOPULL;
    GPIO_InitStructTest.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init( GPIOC, &GPIO_InitStructTest );
}

/**
 * @brief   Clock task function is the part of the state machines.
 *
 * In this function, first we stay in the state idle, later we check if passed a second
 * and if pass a second we go to the state show time, in this state we print the time, 
 * later, we go to state show date where we print the date, after that we go to state
 * show alarm where we print the alarm and we come back to state idle.
 * If we have a message to change the time, date or alarm, so, in the state idle we check 
 * what is the type of message and we go to that state and this state has to change the
 * values in the RTC with the new values in own structure.
 *
 * @retval  None
 *
 */
void Clock_Task( void ){
    APP_MsgTypeDef ClockMsg;
    static uint8_t yearMSB = 20u;
    uint8_t stateClock;

    stateClock = STATE_RECEPTION;

    if( (HAL_GetTick() - tickstartClockFile) >= 50 ){
        tickstartClockFile = HAL_GetTick();

        while( stateClock != STATE_IDLE ){

            switch (stateClock)
            {
            case STATE_IDLE:
                /*STATE EMPTY*/
                break;

            case STATE_RECEPTION:
                if( HIL_QUEUE_IsEmpty( &ClockQueue ) == 0u ){

                    HIL_QUEUE_Read( &ClockQueue, &ClockMsg );

                    if( ClockMsg.msg != (uint8_t)SERIAL_MSG_NONE ){
                        if( ClockMsg.msg == (uint8_t)SERIAL_MSG_TIME ){
                            stateClock = STATE_CHANGE_TIME;
                            ClockMsg.msg = SERIAL_MSG_NONE;
                        }
                        else if( ClockMsg.msg == (uint8_t)SERIAL_MSG_DATE ){
                            stateClock = STATE_CHANGE_DATE;
                            ClockMsg.msg = SERIAL_MSG_NONE;
                        }
                        else if( ClockMsg.msg == (uint8_t)SERIAL_MSG_ALARM ){
                            stateClock = STATE_CHANGE_ALARM;
                            ClockMsg.msg = SERIAL_MSG_NONE;
                        }
                        else{ 
                        }
                    }
                    else{
                    }
                }
                else{
                    stateClock = STATE_IDLE;
                }

                if( (HAL_GetTick() - tickstartShowTime) >= 1000 ){
                    tickstartShowTime = HAL_GetTick();
                    stateClock = STATE_GET_TIME_AND_DATE;
                }
                break;
    
            case STATE_GET_TIME_AND_DATE:
                HAL_RTC_GetTime( &hrtc, &sTime, RTC_FORMAT_BIN );
                HAL_RTC_GetDate( &hrtc, &sDate, RTC_FORMAT_BIN );

                ClockMsg.tm.tm_hour = sTime.Hours;
                ClockMsg.tm.tm_min = sTime.Minutes;
                ClockMsg.tm.tm_sec = sTime.Seconds;

                ClockMsg.tm.tm_mday = sDate.Date;
                ClockMsg.tm.tm_mon = sDate.Month;
                ClockMsg.tm.tm_wday = sDate.WeekDay;
                ClockMsg.tm.tm_year = (yearMSB * ONE_HUNDRED) + sDate.Year;
                HIL_QUEUE_Write( &DisplayQueue, &ClockMsg );

                ClockMsg.msg = (uint8_t)SERIAL_MSG_NONE;

                stateClock = STATE_RECEPTION;
                break;

            case STATE_GET_ALARM:
                HAL_RTC_GetAlarm( &hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN );

                /*ClockMsg.msg = SERIAL_MSG_ALARM;
                ClockMsg.tm.tm_hour = sAlarm.AlarmTime.Hours;
                ClockMsg.tm.tm_min = sAlarm.AlarmTime.Minutes;*/
                
                stateClock = STATE_RECEPTION;
                break;

            case STATE_CHANGE_TIME:

                sTime.Hours = ClockMsg.tm.tm_hour;
                sTime.Minutes = ClockMsg.tm.tm_min;
                sTime.Seconds = ClockMsg.tm.tm_sec;
                HAL_RTC_SetTime( &hrtc, &sTime, RTC_FORMAT_BIN );

                ClockMsg.msg = (uint8_t)SERIAL_MSG_TIME;

                stateClock = STATE_GET_TIME_AND_DATE;
                break;

            case STATE_CHANGE_DATE:
                yearMSB = (ClockMsg.tm.tm_year / ONE_HUNDRED);

                sDate.Date = ClockMsg.tm.tm_mday;
                sDate.Month = ClockMsg.tm.tm_mon; 
                sDate.Year = (ClockMsg.tm.tm_year % ONE_HUNDRED);
                HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BIN );

                ClockMsg.msg = (uint8_t)SERIAL_MSG_DATE;

                stateClock = STATE_GET_TIME_AND_DATE;
                break;

            case STATE_CHANGE_ALARM:

                sAlarm.AlarmTime.Hours = ClockMsg.tm.tm_hour;
                sAlarm.AlarmTime.Minutes = ClockMsg.tm.tm_min;
                HAL_RTC_SetAlarm( &hrtc, &sAlarm, RTC_FORMAT_BIN );

                stateClock = STATE_GET_ALARM;
                break;
    
            default:
              break;
            }
        }
    }
}