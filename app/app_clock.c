/**
 * @file    app_clock.c
 * @author  Jovanny Noé Casillas Franco
 * @brief   **This file is to can print the time**
 *
 * The driver works through state machines where the state idle is the main state, later, we have the others states called show time,
 * show date and show alarm that this are the states that print the time, date and alarm through of RTC peripheral. After that, we
 * have the others states called change time, change date and change alarm that this states are to change the values through the
 * structure created in the serial header file, since these values are processed in this another file. 
 * 
 * @note    We need take in count that this proyect just show the alarm but haven't a alert when the alarm is active, it just 
 *          keep showing the alarm.
 * 
 */
#include "app_bsp.h"
#include "app_clock.h"
#include "app_serial.h"
#include <stdio.h>  /* cppcheck-suppress misra-c2012-21.6 ; The library is only for testing but it wont be included in the last part */

#define RTC_ASYNCH_PREDIV 0x7F
#define RTC_SYNCH_PREDIV 0x0F9

#define STATE_IDLE 0
#define STATE_SHOW_TIME 1
#define STATE_SHOW_DATE 2
#define STATE_SHOW_ALARM 3
#define STATE_CHANGE_TIME 4
#define STATE_CHANGE_DATE 5
#define STATE_CHANGE_ALARM 6

extern void initialise_monitor_handles(void);

static RTC_HandleTypeDef hrtc;
static RTC_TimeTypeDef sTime = {0};
static RTC_DateTypeDef sDate = {0};
static RTC_AlarmTypeDef sAlarm = {0};

static uint8_t stateClock = 0;
static uint32_t tickstartShowTime;

void Clock_Init( void ){

    initialise_monitor_handles();

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

    stateClock = STATE_IDLE;
}

void Clock_Task( void ){
    static uint8_t yearMSB = 20u;

    switch (stateClock)
    {
    case STATE_IDLE:
        if( (HAL_GetTick() - tickstartShowTime) >= 1000 ){
            tickstartShowTime = HAL_GetTick();
            stateClock = STATE_SHOW_TIME;
        }
        else if( TimeCAN.msg != (uint8_t)SERIAL_MSG_NONE ){
            if( TimeCAN.msg == (uint8_t)SERIAL_MSG_TIME ){
                stateClock = STATE_CHANGE_TIME;
                TimeCAN.msg = SERIAL_MSG_NONE;
            }
            else if( TimeCAN.msg == (uint8_t)SERIAL_MSG_DATE ){
                stateClock = STATE_CHANGE_DATE;
                TimeCAN.msg = SERIAL_MSG_NONE;
            }
            else if( TimeCAN.msg == (uint8_t)SERIAL_MSG_ALARM ){
                stateClock = STATE_CHANGE_ALARM;
                TimeCAN.msg = SERIAL_MSG_NONE;
            }
            else{ 
            }
        }
        else{
        }
        break;
    
    case STATE_SHOW_TIME:
        HAL_RTC_GetTime( &hrtc, &sTime, RTC_FORMAT_BIN );

        (void)printf( "Time %d:%d:%d\n\r", sTime.Hours, sTime.Minutes, sTime.Seconds );

        stateClock = STATE_SHOW_DATE;
        break;

    case STATE_SHOW_DATE:
        HAL_RTC_GetDate( &hrtc, &sDate, RTC_FORMAT_BIN );

        if( sDate.Year <= 9u ){
            (void)printf( "Date: %d/%d/%d0%d\n\r", sDate.Date, sDate.Month, yearMSB, sDate.Year );  
        }
        else{
            (void)printf( "Date: %d/%d/%d%d\n\r", sDate.Date, sDate.Month, yearMSB, sDate.Year );
        }

        stateClock = STATE_SHOW_ALARM;
        break;

    case STATE_SHOW_ALARM:
        HAL_RTC_GetAlarm( &hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN );

        (void)printf( "Alarm: %ld:%ld\n\r", TimeCAN.tm.tm_hour, TimeCAN.tm.tm_min );
        (void)printf( "\n\r" );
        
        stateClock = STATE_IDLE;
        break;

    case STATE_CHANGE_TIME:

        sTime.Hours = TimeCAN.tm.tm_hour;
        sTime.Minutes = TimeCAN.tm.tm_min;
        sTime.Seconds = TimeCAN.tm.tm_sec;
        HAL_RTC_SetTime( &hrtc, &sTime, RTC_FORMAT_BIN );

        stateClock = STATE_SHOW_TIME;
        break;

    case STATE_CHANGE_DATE:
        yearMSB = (TimeCAN.tm.tm_year / 100u);

        sDate.Date = TimeCAN.tm.tm_mday;
        sDate.Month = TimeCAN.tm.tm_mon; 
        sDate.Year = (TimeCAN.tm.tm_year % 100u);
        HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BIN );

        stateClock = STATE_SHOW_DATE;
        break;

    case STATE_CHANGE_ALARM:

        sAlarm.AlarmTime.Hours = TimeCAN.tm.tm_hour;
        sAlarm.AlarmTime.Minutes = TimeCAN.tm.tm_min;
        HAL_RTC_SetAlarm( &hrtc, &sAlarm, RTC_FORMAT_BIN );

        stateClock = STATE_SHOW_ALARM;
        break;
    
    default:
        break;
    }
}