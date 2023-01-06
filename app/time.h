#ifndef __TIME_H__
#define __TIME_H__

typedef struct _APP_TmTypeDef 
{
    uint32_t tm_sec;         /* seconds,  range 0 to 59          */

    uint32_t tm_min;         /* minutes, range 0 to 59           */

    uint32_t tm_hour;        /* hours, range 0 to 23             */

    uint32_t tm_mday;        /* day of the month, range 1 to 31  */

    uint32_t tm_mon;         /* month, range 0 to 11             */

    uint32_t tm_year;        /* years in range 1900 to 2100      */

    uint32_t tm_wday;        /* day of the week, range 0 to 6    */

    uint32_t tm_yday;        /* day in the year, range 0 to 365  */

    uint32_t tm_isdst;       /* daylight saving time             */
    
}APP_TmTypeDef;

#endif