#include "app_bsp.h"
#include "app_clock.h"

#define RTC_ASYNCH_PREDIV 0x7F
#define RTC_SYNCH_PREDIV 0x0F9

#define STATE_IDLE 0
#define STATE_SHOW_TIME 1
#define STATE_SHOW_DATE 2
#define STATE_CHANGE_TIME 3
#define STATE_CHANGE_DATE 4

extern void initialise_monitor_handles(void);

RTC_HandleTypeDef hrtc;
RTC_TimeTypeDef sTime = {0};
RTC_DateTypeDef sDate = {0};

uint8_t state = 0;
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
}

void Clock_Task( void ){

    switch (state)
    {
    case STATE_IDLE:
        if( (HAL_GetTick() - tickstartShowTime) >= 1000 ){
            tickstartShowTime = HAL_GetTick();
            state = STATE_SHOW_TIME;
        }
        else if( TimeCAN.msg != SERIAL_MSG_NONE ){
            if( TimeCAN.msg == SERIAL_MSG_TIME ){
                state = STATE_CHANGE_TIME;
            }
            else if( TimeCAN.msg == SERIAL_MSG_DATE ){
                state = STATE_CHANGE_DATE;
            }
        }
        break;
    
    case STATE_SHOW_TIME:

        break;

    case STATE_SHOW_DATE:
        break;

    case STATE_CHANGE_TIME:
        break;

    case STATE_CHANGE_DATE:
        break;
    }
}