#include "app_bsp.h"
#include "app_serial.h"

#define STATE_IDLE 0
#define STATE_TIME 1
#define STATE_DATE 2
#define STATE_ALARM 3
#define STATE_MESSAGE 4
#define STATE_OK 5
#define STATE_ERROR 6

#define STATE_HOURS 1
#define STATE_MINUTES 2
#define STATE_SECONDS 3
#define STATE_DAY 1
#define STATE_MONTH 2
#define STATE_YEAR 3

#define JAN 1
#define FEB 2
#define MAR 3
#define APR 4
#define MAY 5
#define JUN 6
#define JUL 7
#define AUG 8
#define SEP 9
#define OCT 10
#define NOV 11
#define DEC 12

FDCAN_HandleTypeDef CANHandler;
FDCAN_RxHeaderTypeDef CANRxHeader;
APP_TmTypeDef TimeCAN;
GPIO_InitTypeDef GPIO_InitStruct;
FDCAN_TxHeaderTypeDef CANTxHeader;
FDCAN_FilterTypeDef CANFilter;

uint8_t RxData[8];
uint8_t TxData[8];
uint8_t flag = 0;
uint8_t state = 0;

void Serial_Init( void )
{
     /* Declaramos las opciones para configurar el modulo FDCAN1 para transmitir al bus CAN a 100Kbps
     y sample point de 75% */
    CANHandler.Instance                 = FDCAN1;
    CANHandler.Init.Mode                = FDCAN_MODE_NORMAL;
    CANHandler.Init.FrameFormat         = FDCAN_FRAME_CLASSIC;
    CANHandler.Init.ClockDivider        = FDCAN_CLOCK_DIV1;
    CANHandler.Init.TxFifoQueueMode     = FDCAN_TX_FIFO_OPERATION;
    CANHandler.Init.AutoRetransmission  = DISABLE;
    CANHandler.Init.TransmitPause       = DISABLE;
    CANHandler.Init.ProtocolException   = DISABLE;
    CANHandler.Init.ExtFiltersNbr       = 0;
    CANHandler.Init.StdFiltersNbr       = 1;
    CANHandler.Init.NominalPrescaler    = 10;
    CANHandler.Init.NominalSyncJumpWidth = 1;
    CANHandler.Init.NominalTimeSeg1     = 11;
    CANHandler.Init.NominalTimeSeg2     = 4;
    HAL_FDCAN_Init( &CANHandler );
    /* Change FDCAN instance from initialization mode to normal mode */
    
    CANTxHeader.IdType      = FDCAN_STANDARD_ID;
    CANTxHeader.FDFormat    = FDCAN_CLASSIC_CAN;
    CANTxHeader.TxFrameType = FDCAN_DATA_FRAME;
    CANTxHeader.Identifier  = 0x122;
    CANTxHeader.DataLength  = FDCAN_DLC_BYTES_8;

    /* Configure reception filter to Rx FIFO 0, este filtro solo aceptara mensajes con el ID 0x1FE */
    CANFilter.IdType = FDCAN_STANDARD_ID;
    CANFilter.FilterIndex = 0;
    CANFilter.FilterType = FDCAN_FILTER_MASK;
    CANFilter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    CANFilter.FilterID1 = 0x111;
    HAL_FDCAN_ConfigFilter( &CANHandler, &CANFilter );
    /*indicamos que los mensajes que no vengan con el filtro indicado sean rechazados*/
    HAL_FDCAN_ConfigGlobalFilter(&CANHandler, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE);

    HAL_FDCAN_Start( &CANHandler);

    /*activamos la interrupcion por recepcion en el fifo0 cuando llega algun mensaje*/
    HAL_FDCAN_ActivateNotification( &CANHandler, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0 );   
}

void Serial_Task( void )
{
    static uint8_t stateTime, stateDate, stateAlarm, stateOk;
    uint16_t year = 0;
    uint8_t i;

    switch (state)
    {
    case STATE_IDLE:

        if( flag == 1 ){
            flag = 0;
            state = STATE_MESSAGE;
        }
        break;
    
    case STATE_MESSAGE:

        if( RxData[1] == 0x01 ){
            stateTime = STATE_HOURS;
            state = STATE_TIME;
        }
        else if( RxData[1] == 0x02 ){
            stateDate = STATE_MONTH;
            state = STATE_DATE;
        }
        else if( RxData[1] == 0x03 ){
            stateAlarm = STATE_HOURS;
            state = STATE_ALARM;
        }
        else{
            state = STATE_ERROR;
        }
        break;

    case STATE_TIME:

        switch (stateTime)
        {
        case STATE_HOURS:
            if( RxData[2] <= 23 ){
                stateTime = STATE_MINUTES;
            }
            else{
                state = STATE_ERROR;
            }
            break;

        case STATE_MINUTES:
            if( RxData[3] <= 59 ){
                stateTime = STATE_SECONDS;
            }
            else{
                state = STATE_ERROR;
            }
            break;
        
        case STATE_SECONDS:
            if( RxData[4] <= 59 ){
                stateOk = STATE_TIME;
                state = STATE_OK;
            }
            else{
                state = STATE_ERROR;
            }
            break;
        }
        break;

    case STATE_DATE:

        switch (stateDate)
        {
        case STATE_DAY:
            if( (RxData[3] == JAN) || (RxData[3] == MAR) || (RxData[3] == MAY) || (RxData[3] == JUL) || (RxData[3] == AUG) || (RxData[3] == OCT) || (RxData[3] == DEC) ){
                if( (RxData[2] <= 31) && (RxData[2] > 0) ){
                    stateOk = STATE_DATE;
                    state = STATE_OK;
                }  
                else{
                    state = STATE_ERROR;
                }
            }
            else if( (RxData[3] == APR) || (RxData[3] == JUN) || (RxData[3] == SEP) || (RxData[3] == NOV) ){
                if( (RxData[2] <= 30) && (RxData[2] > 0) ){
                    stateOk = STATE_DATE;
                    state = STATE_OK;
                }
                else{
                    state = STATE_ERROR;
                }
            }
            else if( RxData[3] == FEB ){
                if( (year % 4) == 0 ){
                    if( (RxData[2] <= 29) && (RxData[2] > 0) ){
                        stateOk = STATE_DATE;
                        state = STATE_OK;
                    } 
                    else{
                        state = STATE_ERROR;
                    }
                }
                else if( (year % 4) != 0 ){
                    if( (RxData[2] <= 28) && (RxData[2] > 0) ){
                        stateOk = STATE_DATE;
                        state = STATE_OK;
                    }
                    else{
                        state = STATE_ERROR;
                    }
                }
            }
            break;
        
        case STATE_MONTH:
            if( (RxData[3] <= 12) && (RxData[3] > 0) ){
                stateDate = STATE_YEAR;
            }
            else{
                state = STATE_ERROR;
            }
            break;
        
        case STATE_YEAR:
            year = (RxData[4] << 8) + RxData[5];    /*year = 0x07D4*/

            if( (year >= 1900) && (year <= 2100) ){
                stateDate = STATE_DAY;
            }
            else{
                state = STATE_ERROR;
            }
            break;
        }
        break;

    case STATE_ALARM:

        switch (stateAlarm)
        {
        case STATE_HOURS:
            if( RxData[2] <= 23 ){
                stateAlarm = STATE_MINUTES;
            }
            else{
                state = STATE_ERROR;
            }
            break;
        
        case STATE_MINUTES:
            if( RxData[3] <= 59 ){
                stateOk = STATE_ALARM;
                state = STATE_OK;
            }
            else{
                state = STATE_ERROR;
            }
            break;
        }
        break;

    case STATE_OK:
        TxData[0] = RxData[0];

        for( i = 1; i < 8; i++ ){
            TxData[i] = 0x55;
        }

        HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, TxData );

        switch (stateOk)
        {
        case STATE_TIME:
            TimeCAN.tm_hour = RxData[2];
            TimeCAN.tm_min = RxData[3];
            TimeCAN.tm_sec = RxData[4];
            break;
        
        case STATE_DATE:
            TimeCAN.tm_mday = RxData[2];
            TimeCAN.tm_mon = RxData[3];
            TimeCAN.tm_year = year;
            break;

        case STATE_ALARM:
            TimeCAN.tm_hour = RxData[2];
            TimeCAN.tm_min = RxData[3];
            break;
        }
        state = STATE_IDLE;
        break;

    case STATE_ERROR:
        TxData[0] = RxData[0];

        for( i = 1; i < 8; i++ ){
            TxData[i] = 0xAA;
        }

        HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, TxData );

        state = STATE_IDLE;
        break;
    }
}

void HAL_FDCAN_RxFifo0Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs )
{
    /*A llegado un mensaje via CAN, interrogamos si fue un solo mensaje*/
    if( (RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0 ){
        /* Retrieve Rx messages from RX FIFO0 */
        HAL_FDCAN_GetRxMessage( hfdcan, FDCAN_RX_FIFO0, &CANRxHeader, RxData );

        if( RxData[0] == 0x07 ){
            flag = 1;
            state = STATE_IDLE;
        }
        else{
            state = STATE_ERROR;
        }
        
    }
}