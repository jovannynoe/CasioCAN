/**
 * @file    app_serial.c
 * @author  Jovanny Noé Casillas Franco
 * @brief   **This file is to receive messages through CANdo and save the values in structure**
 *
 * The driver works through state machines where the state idle is the main state, later, we have the another state called message,
 * in this state we verifying what is the type of message, if the message is a time, date, alarm or none. We know what type is because
 * in the second byte of the CANdo we have to write 0x01 for time, 0x02 for date and 0x03 for alarm, if this byte is different is a
 * error, so, here is when the error state appears, this state just come back a message to CANdo with a message that indicate an error.
 * When it is a error message we return to CANdo the same CAN-TP but the others bytes are 0xAA, but if the message is ok we return
 * to CANdo the same CANT-TP but the others bytes are 0x55.
 * 
 * The another state is the time state, in this state we validate if the values are in the range of 24 hours, if the values are ok,
 * so, we go to ok state and in this state we save the values in a structure. The date state validate if the values are correct we 
 * go to ok state and we save the values in the same structure. Finally, we have the alarm state, in this state just we validate 
 * the hour and the minutes, if everything is ok, we go to ok state and we save the values in the same structure.
 * 
 * @note    We need take in count that the range of the year in date is just between 1900 to 2100.
 *          
 */
#include "app_bsp.h"
#include "app_serial.h"
#include "app_clock.h"

/** 
  * @defgroup States States to we know what is the state without errors
  @{ */
#define STATE_IDLE 0u
#define STATE_TIME 1u
#define STATE_DATE 2u
#define STATE_ALARM 3u
#define STATE_MESSAGE 4u
#define STATE_OK 5u
#define STATE_ERROR 6u
/**
  @} */

/** 
  * @defgroup Substates Substates to we know what is the substate without errors
  @{ */
#define STATE_HOURS 1u
#define STATE_MINUTES 2u
#define STATE_SECONDS 3u
#define STATE_DAY 1u
#define STATE_MONTH 2u
#define STATE_YEAR 3u
/**
  @} */

/** 
  * @defgroup CAN Values used in the transmission or reception of CAN
  @{ */
#define ERROR_TRANSMIT 0xAAu
#define OK_TRANSMIT 0x55u
#define CAN_TP 0x07u
/**
  @} */

/** 
  * @defgroup Months Months of the year
  @{ */
#define JAN 1u
#define FEB 2u
#define MAR 3u
#define APR 4u
#define MAY 5u
#define JUN 6u
#define JUL 7u
#define AUG 8u
#define SEP 9u
#define OCT 10u
#define NOV 11u
#define DEC 12u
/**
  @} */

FDCAN_HandleTypeDef CANHandler;
FDCAN_TxHeaderTypeDef CANTxHeader;
FDCAN_FilterTypeDef CANFilter;
APP_MsgTypeDef TimeCAN;

uint8_t BCDFormatToDecimalFormat( uint8_t numberBCD );
void HAL_FDCAN_RxFifo0Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs );

/**
 * @brief  Global variable to save the values from the CANdo
 */
static uint8_t RxData[8];

/**
 * @brief  Global variable to we use like a flag
 */
static uint8_t flag = 0;

/**
 * @brief  Global variable to change the state in the state machines
 */
static uint8_t stateSerial = 0;

/**
 * @brief   **Serial init function is to configurate the CAN protocol**
 *
 * In this function we go to configurate the CAN protocol, also the TX header and the filter
 * to receive messages just with the ID 0x122 and 8 Bytes.
 * Finally we have the IT function to when we receive a message we go to callback function.
 *
 * @retval  None
 *
 */
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

/**
 * @brief   **Serial task function is the part of the state machines**
 *
 * In this function, first we stay in the state idle, later we check if flag is equal to one
 * we go to message state, in this state we check what is the type of message to go to next 
 * state but, before we use the function to change from BCD format to decimal format.
 * When we are in the next state taking in count the type of message just we validate if the 
 * values are ok, if the values aren't ok we go to error state, in this state we transmit 
 * a error message to cando, but, if the values are ok, so, we go to ok state and we save the 
 * values in a structure to can use with the app_clock file and we transmit a ok message to 
 * cando.
 *
 * @retval  None
 *
 */
void Serial_Task( void )
{
    static uint8_t stateTime; 
    static uint8_t stateDate; 
    static uint8_t stateAlarm; 
    static uint8_t stateOk;
    static uint16_t year;
    static uint8_t TxData[8];
    uint8_t i;

    switch (stateSerial)
    {
    case STATE_IDLE:

        if( flag == 1u ){
            flag = 0u;
            stateSerial = STATE_MESSAGE;
        }
        break;
    
    case STATE_MESSAGE:

        if( RxData[1] == (uint8_t)SERIAL_MSG_TIME ){
            RxData[2] = BCDFormatToDecimalFormat(RxData[2]);
            RxData[3] = BCDFormatToDecimalFormat(RxData[3]);
            RxData[4] = BCDFormatToDecimalFormat(RxData[4]);
            stateTime = STATE_HOURS;
            stateSerial = STATE_TIME;
        }
        else if( RxData[1] == (uint8_t)SERIAL_MSG_DATE ){
            RxData[2] = BCDFormatToDecimalFormat(RxData[2]);
            RxData[3] = BCDFormatToDecimalFormat(RxData[3]);
            RxData[4] = BCDFormatToDecimalFormat(RxData[4]);
            RxData[5] = BCDFormatToDecimalFormat(RxData[5]);
            stateDate = STATE_MONTH;
            stateSerial = STATE_DATE;
        }
        else if( RxData[1] == (uint8_t)SERIAL_MSG_ALARM ){
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
            if( RxData[2] <= 23u ){
                stateTime = STATE_MINUTES;
            }
            else{
                stateSerial = STATE_ERROR;
            }
            break;

        case STATE_MINUTES:
            if( RxData[3] <= 59u ){
                stateTime = STATE_SECONDS;
            }
            else{
                stateSerial = STATE_ERROR;
            }
            break;
        
        case STATE_SECONDS:
            if( RxData[4] <= 59u ){
                stateOk = STATE_TIME;
                stateSerial = STATE_OK;
            }
            else{
                stateSerial = STATE_ERROR;
            }
            break;
        
        default:
            break;
        }
        break;

    case STATE_DATE:

        switch (stateDate)
        {
        case STATE_DAY:
            if( (RxData[3] == JAN) || (RxData[3] == MAR) || (RxData[3] == MAY) || (RxData[3] == JUL) || (RxData[3] == AUG) || (RxData[3] == OCT) || (RxData[3] == DEC) ){
                if( (RxData[2] <= 31u) && (RxData[2] > 0u) ){
                    stateOk = STATE_DATE;
                    stateSerial = STATE_OK;
                }  
                else{
                    stateSerial = STATE_ERROR;
                }
            }
            else if( (RxData[3] == APR) || (RxData[3] == JUN) || (RxData[3] == SEP) || (RxData[3] == NOV) ){
                if( (RxData[2] <= 30u) && (RxData[2] > 0u) ){
                    stateOk = STATE_DATE;
                    stateSerial = STATE_OK;
                }
                else{
                    stateSerial = STATE_ERROR;
                }
            }
            else if( RxData[3] == FEB ){
                if( (year % 4u) == 0u ){
                    if( (RxData[2] <= 29u) && (RxData[2] > 0u) ){
                        stateOk = STATE_DATE;
                        stateSerial = STATE_OK;
                    } 
                    else{
                        stateSerial = STATE_ERROR;
                    }
                }
                else if( (year % 4u) != 0u ){
                    if( (RxData[2] <= 28u) && (RxData[2] > 0u) ){
                        stateOk = STATE_DATE;
                        stateSerial = STATE_OK;
                    }
                    else{
                        stateSerial = STATE_ERROR;
                    }
                }
                else{
                }
            }
            else{
            }
            break;
        
        case STATE_MONTH:
            if( (RxData[3] <= 12u) && (RxData[3] > 0u) ){
                stateDate = STATE_YEAR;
            }
            else{
                stateSerial = STATE_ERROR;
            }
            break;
        
        case STATE_YEAR:
            year = (RxData[4] * 100u) + RxData[5];    

            if( (year >= 1900u) && (year <= 2100u) ){
                stateDate = STATE_DAY;
            }
            else{
                stateSerial = STATE_ERROR;
            }
            break;
        
        default:
            break;
        }
        break;

    case STATE_ALARM:

        switch (stateAlarm)
        {
        case STATE_HOURS:
            if( RxData[2] <= 23u ){
                stateAlarm = STATE_MINUTES;
            }
            else{
                stateSerial = STATE_ERROR;
            }
            break;
        
        case STATE_MINUTES:
            if( RxData[3] <= 59u ){
                stateOk = STATE_ALARM;
                stateSerial = STATE_OK;
            }
            else{
                stateSerial = STATE_ERROR;
            }
            break;
        
        default:
            break;
        }
        break;

    case STATE_OK:
        TxData[0] = RxData[0];

        for( i = 1u; i < 8u; i++ ){
            TxData[i] = OK_TRANSMIT;
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
        
        default:
            break;
        }

        stateSerial = STATE_IDLE;
        break;

    case STATE_ERROR:
        TxData[0] = RxData[0];

        for( i = 1u; i < 8u; i++ ){
            TxData[i] = ERROR_TRANSMIT;
        }

        HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, TxData );

        stateSerial = STATE_IDLE;
        break;
    
    default:
        break;
    }
}

/**
 * @brief   **CAN Callback function is to active the flag**
 *
 * In this function, we get the RX message and save the values in RxData to we can validate
 * the values in the state machine, but before, we validate the first byte in RxData 
 * because this byte is CAN-TP, so, if this byte is correct, flag receive 1 and stateSerial
 * receive idle state if this byte isn't correct, so we go to error state.  
 * 
 * @param   <*hfdcan>[out] Is a parameter that the function needs to work.
 * @param   <RxFifo0ITs>[out] Is a parameter that the function needs to work.
 *
 * @retval  None
 *
 */
void HAL_FDCAN_RxFifo0Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs )
{
    FDCAN_RxHeaderTypeDef CANRxHeader;
    /*A llegado un mensaje via CAN, interrogamos si fue un solo mensaje*/
    if( (RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0u ){
        /* Retrieve Rx messages from RX FIFO0 */
        HAL_FDCAN_GetRxMessage( hfdcan, FDCAN_RX_FIFO0, &CANRxHeader, RxData );

        if( RxData[0] == CAN_TP ){
            flag = 1u;
            stateSerial = STATE_IDLE;
        }
        else{
            stateSerial = STATE_ERROR;
        }   
    }
}

/**
 * @brief   **BCD to Decimal function is to convert values to decimal**
 *
 * In this function, we have a parameter in BCD format, so, we will change the format
 * to decimal format with a formula.
 * First we move four bits the value in BCD and we multiply by 10, later the value in 
 * BCD format we make a operation with the logic operator AND only to the 4 LSB and 
 * finally we make a sum between this two values and we return the value in decimal
 * format. 
 * 
 * @param   <numberBCD>[out] Is a parameter in BCD format of 8 bits.
 *
 * @retval  We return the value in decimal format with values between 0 to 99
 *
 */
uint8_t BCDFormatToDecimalFormat( uint8_t numberBCD )
{
    uint8_t valueDecimal;

    valueDecimal = ( (numberBCD >> 4u) * 10u ) + (numberBCD & 0xFu);

    return valueDecimal;
}