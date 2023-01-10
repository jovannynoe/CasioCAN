#ifndef _BSP_H_
#define _BSP_H_

    #include "stm32g0xx.h"
    #include <stdint.h>

    typedef struct _APP_TmTypeDef 
    {
        uint32_t tm_sec;         /* seconds,  range 0 to 59          */

        uint32_t tm_min;         /* minutes, range 0 to 59           */
        
        uint32_t tm_hour;        /* hours, range 0 to 23             */
        
        uint32_t tm_mday;        /* day of the month, range 1 to 31  */
        
        uint32_t tm_mon;         /* month, range 0 to 11             */
        
        uint32_t tm_year;        /* years in rage 1900 2100          */
        
        uint32_t tm_wday;        /* day of the week, range 0 to 6    */
        
        uint32_t tm_yday;        /* day in the year, range 0 to 365  */
        
        uint32_t tm_isdst;       /* daylight saving time             */
        
    }APP_TmTypeDef;

    typedef struct _APP_MsgTypeDef
    {
        uint8_t msg;          /*!< Store the message type to send */

        APP_TmTypeDef tm;     /*!< time and date in stdlib tm format */

    }APP_MsgTypeDef;

    extern APP_MsgTypeDef TimeCAN;

    typedef enum
    {
        SERIAL_MSG_NONE = 0,

        SERIAL_MSG_TIME,

        SERIAL_MSG_DATE,

        SERIAL_MSG_ALARM
    
    }APP_Messages;

#endif

