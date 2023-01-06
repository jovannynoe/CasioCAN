#include "app_bsp.h"
#include "app_clock.h"

#define RTC_ASYNCH_PREDIV 0x7F
#define RTC_SYNCH_PREDIV 0x0F9

RTC_HandleTypeDef hrtc;

RTC_TimeTypeDef sTime = {0};
RTC_DateTypeDef sDate = {0};

void Clock_Init( void ){

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
}

void Clock_Task( void ){

}