/**
 * @file    app_serial.c
 * @author  Jovanny Noé Casillas Franco
 * @brief   This file is to receive messages through CANdo and save the values in structure.
 *
 * The driver works through state machines where the state idle is the main state, later, we have the 
 * another state called message, in this state we verifying what is the type of message, if the message 
 * is a time, date, alarm or none. We know what type is because in the second byte of the CANdo we have 
 * to write 0x01 for time, 0x02 for date and 0x03 for alarm, if this byte is different is a error, so, 
 * here is when the error state appears, this state just come back a message to CANdo with a message 
 * that indicate an error.
 * When it is a error message we return to CANdo the same CAN-TP but the others bytes are 0xAA, but if 
 * the message is ok we return to CANdo the same CANT-TP but the others bytes are 0x55.
 * 
 * The another state is the time state, in this state we validate if the values are in the range of 24 
 * hours, if the values are ok, so, we go to ok state and in this state we save the values in a 
 * structure. The date state validate if the values are correct we go to ok state and we save the 
 * values in the same structure. Finally, we have the alarm state, in this state just we validate 
 * the hour and the minutes, if everything is ok, we go to ok state and we save the values in the same 
 * structure.
 * 
 * @note    We need take in count that the range of the year in date is just between 1900 to 2100.
 *          
 */
#include "app_bsp.h"
#include "app_serial.h"
#include "app_clock.h"
#include "app_display.h"

/** 
  * @defgroup Buffer Maximum size to buffer used
  @{ */
#define BUFFER_MAX_SIZE 9u /*!< Size of buffer with elements */
/**
  @} */

/** 
  * @defgroup Buffer Maximum size to buffer used
  @{ */
#define TRUE 1u     /*!< Used to change the one per true with word */
#define FALSE 0u    /*!< Used to change the zero per false with word */ 
/**
  @} */

/** 
  * @defgroup States States to we know what is the state without errors
  @{ */
#define STATE_IDLE 0u       /*!< Idle state */       
#define STATE_TIME 1u       /*!< State to validate the parameters of time */
#define STATE_DATE 2u       /*!< State to validate the parameters of date */
#define STATE_ALARM 3u      /*!< State to validate the parameters of alarm */
#define STATE_MESSAGE 4u    /*!< State to validate and select what type of message */
#define STATE_OK 5u         /*!< State to save the values of the message in structure */
#define STATE_ERROR 6u      /*!< State to send a error message to CANdo */
#define STATE_RECEPTION 7u  /*!< State to read messages */ 
/**
  @} */

/** 
  * @defgroup Substates Substates to we know what is the substate without errors
  @{ */
#define STATE_HOURS 1u      /*!< State to validate the hours */
#define STATE_MINUTES 2u    /*!< State to validate the minutes */
#define STATE_SECONDS 3u    /*!< State to validate the seconds */
#define STATE_DAY 1u        /*!< State to validate the day of the month */
#define STATE_MONTH 2u      /*!< State to validate the month */
#define STATE_YEAR 3u       /*!< State to validate the year */
/**
  @} */

/** 
  * @defgroup CAN Values used in the transmission or reception of CAN
  @{ */
#define ERROR_TRANSMIT 0xAAu    /*!< Value to send when is an error message */
#define OK_TRANSMIT 0x55u       /*!< Value to send when is an OK message */
#define CAN_TP 0x07u            /*!< Value to send in the first byte */
/**
  @} */

/** 
  * @defgroup Months Months of the year
  @{ */
#define JAN 1u  /*!< January month */
#define FEB 2u  /*!< February month */
#define MAR 3u  /*!< March month */
#define APR 4u  /*!< April month */
#define MAY 5u  /*!< May month */
#define JUN 6u  /*!< June month */
#define JUL 7u  /*!< July month */
#define AUG 8u  /*!< August month */
#define SEP 9u  /*!< September month */
#define OCT 10u /*!< October month */
#define NOV 11u /*!< November month */
#define DEC 12u /*!< December month */
/**
  @} */

/**
 * @brief  Global variable to use the buffer to CAN
 */
static uint64_t bufferSerial[BUFFER_MAX_SIZE];

/**
 * @brief  APP MSG Structure definition to use the buffer between Serial and Clock
 */
APP_MsgTypeDef bufferClock[45];

/**
 * @brief  Variable for QUEUE Handle Structure definition
 */
QUEUE_HandleTypeDef SerialQueue;

/**
 * @brief  Variable for QUEUE Handle Structure definition
 */
QUEUE_HandleTypeDef ClockQueue;

/**
 * @brief  Variable for FDCAN Handle Structure definition
 */
FDCAN_HandleTypeDef CANHandler;

/**
 * @brief  Variable for FDCAN Tx header Structure definition
 */
FDCAN_TxHeaderTypeDef CANTxHeader;

/**
 * @brief  Variable for FDCAN filter structure definition
 */
FDCAN_FilterTypeDef CANFilter;

/**
 * @brief  Variable to can save the time, date an alarm through the structure
 */
/*APP_MsgTypeDef SerialMsg;*/

/**
 * @brief  Global variable to we use like a flag
 */
static uint8_t flag = 0u;

/**
 * @brief  Global variable to change the state in the state machines
 */
static uint8_t stateSerial;

/**
 * @brief  Global variable to wait a message per 10 ms
 */
static uint32_t tickstartWaitMessage;

/**
 * @brief   Serial init function is to configurate the CAN protocol.
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
    SerialQueue.Buffer = (void *)bufferSerial;
    SerialQueue.Elements = BUFFER_MAX_SIZE;
    SerialQueue.Size = sizeof( uint64_t );
    HIL_QUEUE_Init( &SerialQueue );

    ClockQueue.Buffer = (void *)bufferClock;
    ClockQueue.Elements = 45;
    ClockQueue.Size = sizeof( APP_MsgTypeDef );
    HIL_QUEUE_Init( &ClockQueue );

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

    tickstartWaitMessage = HAL_GetTick();  
}

/**
 * @brief   Serial task function is the part of the state machines.
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
    APP_MsgTypeDef SerialMsg;
    static uint8_t stateTime; 
    static uint8_t stateDate; 
    static uint8_t stateAlarm; 
    static uint8_t stateOk;
    static uint16_t year;
    static uint8_t TxData[8];
    uint8_t RxData[8];
    uint8_t i;

    if( (HAL_GetTick() - tickstartWaitMessage) >= 10 ){
        tickstartWaitMessage = HAL_GetTick();

        while( stateSerial != STATE_IDLE ){
            switch (stateSerial)
            {
            case STATE_IDLE:
                /*STATE EMPTY*/
                break;

            case STATE_RECEPTION:
                if( HIL_QUEUE_IsEmptyISR( &SerialQueue, TIM16_FDCAN_IT0_IRQn ) == FALSE ){

                    HIL_QUEUE_ReadISR( &SerialQueue, RxData, TIM16_FDCAN_IT0_IRQn );
                
                    if( flag == 1u ){
                        flag = 0u;
                        stateSerial = STATE_MESSAGE;
                    }
                }
                else{
                    stateSerial = STATE_IDLE;
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
                    SerialMsg.tm.tm_hour = RxData[2];
                    SerialMsg.tm.tm_min = RxData[3];
                    SerialMsg.tm.tm_sec = RxData[4];
                    SerialMsg.msg = STATE_TIME;
                    HIL_QUEUE_WriteISR( &ClockQueue, &SerialMsg, TIM16_FDCAN_IT0_IRQn );
                    break;
        
                case STATE_DATE:
                    SerialMsg.tm.tm_mday = RxData[2];
                    SerialMsg.tm.tm_mon = RxData[3];
                    SerialMsg.tm.tm_year = year;
                    SerialMsg.msg = STATE_DATE;
                    HIL_QUEUE_WriteISR( &ClockQueue, &SerialMsg, TIM16_FDCAN_IT0_IRQn );
                    break;

                case STATE_ALARM:
                    SerialMsg.tm.tm_hour = RxData[2];
                    SerialMsg.tm.tm_min = RxData[3];
                    SerialMsg.msg = STATE_ALARM;
                    HIL_QUEUE_WriteISR( &ClockQueue, &SerialMsg, TIM16_FDCAN_IT0_IRQn );
                    break;
        
                default:
                    break;
                }

                stateSerial = STATE_RECEPTION;
                break;

            case STATE_ERROR:
                TxData[0] = RxData[0];

                for( i = 1u; i < 8u; i++ ){
                    TxData[i] = ERROR_TRANSMIT;
                }

                HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, TxData );

                stateSerial = STATE_RECEPTION;
                break;
    
            default:
                break;
            }
        }
    }
}

/**
 * @brief   CAN Callback function is to active the flag.
 *
 * In this function, we get the RX message and save the values in RxData to we can validate
 * the values in the state machine, but before, we validate the first byte in RxData 
 * because this byte is CAN-TP, so, if this byte is correct, flag receive 1 and stateSerial
 * receive idle state if this byte isn't correct, so we go to error state.  
 * 
 * @param   hfdcan Is a parameter that the function needs to work.
 * @param   RxFifo0ITs Is a parameter that the function needs to work.
 *
 * @retval  None
 *
 */
void HAL_FDCAN_RxFifo0Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs )
{
    FDCAN_RxHeaderTypeDef CANRxHeader;
    uint8_t RxData[8];

    /*A llegado un mensaje via CAN, interrogamos si fue un solo mensaje*/
    if( (RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != FALSE ){
        /* Retrieve Rx messages from RX FIFO0 */
        HAL_FDCAN_GetRxMessage( hfdcan, FDCAN_RX_FIFO0, &CANRxHeader, RxData );

        if( RxData[0] == CAN_TP ){
            HIL_QUEUE_Write( &SerialQueue, RxData );
            flag = TRUE;
            stateSerial = STATE_RECEPTION;
        }
        else{
            stateSerial = STATE_ERROR;
        }   
    }
}

/**
 * @brief   BCD to Decimal function is to convert values to decimal.
 *
 * In this function, we have a parameter in BCD format, so, we will change the format
 * to decimal format with a formula.
 * First we move four bits the value in BCD and we multiply by 10, later the value in 
 * BCD format we make a operation with the logic operator AND only to the 4 LSB and 
 * finally we make a sum between this two values and we return the value in decimal
 * format. 
 * 
 * @param   numberBCD[out] Is a parameter in BCD format of 8 bits.
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