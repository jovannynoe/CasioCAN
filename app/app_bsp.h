#ifndef _BSP_H_
#define _BSP_H_

    #include "stm32g0xx.h"
    #include "time.h"
    #include <stdint.h>

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

