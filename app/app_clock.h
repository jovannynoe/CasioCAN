#ifndef APP_CLOCK_H__
#define APP_CLOCK_H__

typedef enum
{
    SERIAL_MSG_NONE = 0,
    
    SERIAL_MSG_TIME,

    SERIAL_MSG_DATE,

    SERIAL_MSG_ALARM
    
}APP_Messages;

extern APP_Messages typeMessage;

extern WWDG_HandleTypeDef hwwdg;
extern GPIO_InitTypeDef GPIO_InitStruct;

void Clock_Init( void );
void Clock_Task( void );

#endif