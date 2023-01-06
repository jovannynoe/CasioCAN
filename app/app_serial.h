#ifndef __APP_SERIAL_H__
#define __APP_SERIAL_H__

#include "time.h"

typedef struct _APP_MsgTypeDef
{
    uint8_t msg;          /*!< Store the message type to send */

    APP_TmTypeDef tm;     /*!< time and date in stdlib tm format */

}APP_MsgTypeDef;

typedef enum
{
    SERIAL_MSG_NONE = 0,

    SERIAL_MSG_TIME,

    SERIAL_MSG_DATE,

    SERIAL_MSG_ALARM
    
}APP_Messages;

void Serial_Init( void );
void Serial_Task( void );

#endif