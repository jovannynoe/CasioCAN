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
APP_MsgTypeDef TimeCAN;
GPIO_InitTypeDef GPIO_InitStruct;
FDCAN_TxHeaderTypeDef CANTxHeader;
FDCAN_FilterTypeDef CANFilter;

uint8_t BCDFormatToDecimalFormat( uint8_t numberBCD );

uint8_t RxData[8];
uint8_t TxData[8];
uint8_t flag = 0;
uint8_t stateSerial = 0;

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
    static uint16_t year;
    uint8_t i;

    switch (stateSerial)
    {
    case STATE_IDLE:

        if( flag == 1 ){
            flag = 0;
            stateSerial = STATE_MESSAGE;
        }
        break;
    
    case STATE_MESSAGE:

        if( RxData[1] == 0x01 ){
            RxData[2] = BCDFormatToDecimalFormat(RxData[2]);
            RxData[3] = BCDFormatToDecimalFormat(RxData[3]);
            RxData[4] = BCDFormatToDecimalFormat(RxData[4]);
            stateTime = STATE_HOURS;
            stateSerial = STATE_TIME;
        }
        else if( RxData[1] == 0x02 ){
            RxData[2] = BCDFormatToDecimalFormat(RxData[2]);
            RxData[3] = BCDFormatToDecimalFormat(RxData[3]);
            RxData[4] = BCDFormatToDecimalFormat(RxData[4]);
            RxData[5] = BCDFormatToDecimalFormat(RxData[5]);
            stateDate = STATE_MONTH;
            stateSerial = STATE_DATE;
        }
        else if( RxData[1] == 0x03 ){
            RxData[2] = BCDFormatToDecimalFormat(RxData[2]);
            RxData[3] = BCDFormatToDecimalFormat(RxData[3]);
            stateAlarm = STATE_HOURS;
            stateSerial = STATE_ALARM;
        }
        else{
            stateSerial = STATE_ERROR;
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
                stateSerial = STATE_ERROR;
            }
            break;

        case STATE_MINUTES:
            if( RxData[3] <= 59 ){
                stateTime = STATE_SECONDS;
            }
            else{
                stateSerial = STATE_ERROR;
            }
            break;
        
        case STATE_SECONDS:
            if( RxData[4] <= 59 ){
                stateOk = STATE_TIME;
                stateSerial = STATE_OK;
            }
            else{
                stateSerial = STATE_ERROR;
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
                    stateSerial = STATE_OK;
                }  
                else{
                    stateSerial = STATE_ERROR;
                }
            }
            else if( (RxData[3] == APR) || (RxData[3] == JUN) || (RxData[3] == SEP) || (RxData[3] == NOV) ){
                if( (RxData[2] <= 30) && (RxData[2] > 0) ){
                    stateOk = STATE_DATE;
                    stateSerial = STATE_OK;
                }
                else{
                    stateSerial = STATE_ERROR;
                }
            }
            else if( RxData[3] == FEB ){
                if( (year % 4) == 0 ){
                    if( (RxData[2] <= 29) && (RxData[2] > 0) ){
                        stateOk = STATE_DATE;
                        stateSerial = STATE_OK;
                    } 
                    else{
                        stateSerial = STATE_ERROR;
                    }
                }
                else if( (year % 4) != 0 ){
                    if( (RxData[2] <= 28) && (RxData[2] > 0) ){
                        stateOk = STATE_DATE;
                        stateSerial = STATE_OK;
                    }
                    else{
                        stateSerial = STATE_ERROR;
                    }
                }
            }
            break;
        
        case STATE_MONTH:
            if( (RxData[3] <= 12) && (RxData[3] > 0) ){
                stateDate = STATE_YEAR;
            }
            else{
                stateSerial = STATE_ERROR;
            }
            break;
        
        case STATE_YEAR:
            year = (RxData[4] * 100) + RxData[5];    /*year = 0x2023*/

            if( (year >= 1900) && (year <= 2100) ){
                stateDate = STATE_DAY;
            }
            else{
                stateSerial = STATE_ERROR;
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
                stateSerial = STATE_ERROR;
            }
            break;
        
        case STATE_MINUTES:
            if( RxData[3] <= 59 ){
                stateOk = STATE_ALARM;
                stateSerial = STATE_OK;
            }
            else{
                stateSerial = STATE_ERROR;
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
            TimeCAN.tm.tm_hour = RxData[2];
            TimeCAN.tm.tm_min = RxData[3];
            TimeCAN.tm.tm_sec = RxData[4];
            TimeCAN.msg = STATE_TIME;
            break;
        
        case STATE_DATE:
            TimeCAN.tm.tm_mday = RxData[2];
            TimeCAN.tm.tm_mon = RxData[3];
            TimeCAN.tm.tm_year = year;
            
            TimeCAN.msg = STATE_DATE;
            break;

        case STATE_ALARM:
            TimeCAN.tm.tm_hour = RxData[2];
            TimeCAN.tm.tm_min = RxData[3];
            TimeCAN.msg = STATE_ALARM;
            break;
        }
        stateSerial = STATE_IDLE;
        break;

    case STATE_ERROR:
        TxData[0] = RxData[0];

        for( i = 1; i < 8; i++ ){
            TxData[i] = 0xAA;
        }

        HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, TxData );

        stateSerial = STATE_IDLE;
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
            stateSerial = STATE_IDLE;
        }
        else{
            stateSerial = STATE_ERROR;
        }   
    }
}

uint8_t BCDFormatToDecimalFormat( uint8_t numberBCD )
{
    uint8_t valueDecimal;
    /*RxData[2] = 0x34  ->  0011 0100    ->  RxData[2] = ( (0011 0100 >> 4) * 10)    ->     0000 0011 * 10  ->  0001 1110

    (RxData[2] & 0xF)   ->  0011 0100   ->  0100*/
    valueDecimal = ( (numberBCD >> 4) * 10 ) + (numberBCD & 0xF);

    return valueDecimal;
}