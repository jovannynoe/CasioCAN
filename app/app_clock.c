#include "app_bsp.h"
#include "app_clock.h"
#include <stdio.h>

#define RTC_ASYNCH_PREDIV 0x7F
#define RTC_SYNCH_PREDIV 0x0F9

#define STATE_IDLE 0
#define STATE_SHOW_TIME 1
#define STATE_SHOW_DATE 2
#define STATE_SHOW_ALARM 3
#define STATE_CHANGE_TIME 4
#define STATE_CHANGE_DATE 5

extern void initialise_monitor_handles(void);

RTC_HandleTypeDef hrtc;
RTC_TimeTypeDef sTime = {0};
RTC_DateTypeDef sDate = {0};

uint8_t stateClock = 0;
uint32_t tickstartShowTime;

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

    tickstartShowTime = HAL_GetTick();

    stateClock = STATE_IDLE;
}

void Clock_Task( void ){
    static uint8_t flagAlarm = 0;

    switch (stateClock)
    {
    case STATE_IDLE:
        if( (HAL_GetTick() - tickstartShowTime) >= 1000 ){
            tickstartShowTime = HAL_GetTick();
            stateClock = STATE_SHOW_TIME;
        }
        else if( TimeCAN.msg != SERIAL_MSG_NONE ){
            if( TimeCAN.msg == SERIAL_MSG_TIME ){
                stateClock = STATE_CHANGE_TIME;
                TimeCAN.msg = SERIAL_MSG_NONE;
            }
            else if( TimeCAN.msg == SERIAL_MSG_DATE ){
                stateClock = STATE_CHANGE_DATE;
                TimeCAN.msg = SERIAL_MSG_NONE;
            }
            else if( TimeCAN.msg == SERIAL_MSG_ALARM ){
                flagAlarm = 1;
                TimeCAN.msg = SERIAL_MSG_NONE;
            }
        }
        break;
    
    case STATE_SHOW_TIME:
        HAL_RTC_GetTime( &hrtc, &sTime, RTC_FORMAT_BIN );

        printf( "Time %d:%d:%d\n\r", sTime.Hours, sTime.Minutes, sTime.Seconds );

        stateClock = STATE_SHOW_DATE;
        break;

    case STATE_SHOW_DATE:
        HAL_RTC_GetDate( &hrtc, &sDate, RTC_FORMAT_BIN );

        if( ( (sDate.Year <= 99) && (sDate.Year >= 0) ) || ( (TimeCAN.tm.tm_year <= 2099) && (TimeCAN.tm.tm_year >= 2000) ) ){
            if( sDate.Year == 0 ){
                printf( "Date: %d/%d/200%d\n\r", sDate.Date, sDate.Month, sDate.Year );   
            }
            else{
                printf( "Date: %d/%d/20%d\n\r", sDate.Date, sDate.Month, sDate.Year );
            } 
        }
        else if( (TimeCAN.tm.tm_year <= 1999) && (TimeCAN.tm.tm_year >= 1900) ){
            if( sDate.Year == 0 ){
                printf( "Date: %d/%d/190%d\n\r", sDate.Date, sDate.Month, sDate.Year );
            }
            else{
                printf( "Date: %d/%d/19%d\n\r", sDate.Date, sDate.Month, sDate.Year );
            }
        }
        else if( TimeCAN.tm.tm_year == 2100 ){
            printf( "Date: %d/%d/210%d\n\r", sDate.Date, sDate.Month, sDate.Year );
        }

        stateClock = STATE_SHOW_ALARM;
        break;

    case STATE_SHOW_ALARM:

        if( flagAlarm == 1 ){
            printf( "Alarm: %ld:%ld\n\r", TimeCAN.tm.tm_hour, TimeCAN.tm.tm_min );
            printf( "\n\r" );
        }
        else{
            printf( "Alarm: 00:00\n\r" );
            printf( "\n\r" );
        }

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

        sDate.Date = TimeCAN.tm.tm_mday;
        sDate.Month = TimeCAN.tm.tm_mon;
        if( (TimeCAN.tm.tm_year >= 1900) && (TimeCAN.tm.tm_year <= 1999) ){
            sDate.Year = TimeCAN.tm.tm_year - 1900;
        }
        else if( (TimeCAN.tm.tm_year >= 2000) && (TimeCAN.tm.tm_year <= 2099) ){
            sDate.Year = TimeCAN.tm.tm_year - 2000;
        }
        else if( TimeCAN.tm.tm_year == 2100 ){
            sDate.Year = 0;
        }
        
        HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BIN );

        stateClock = STATE_SHOW_DATE;
        break;
    }
}