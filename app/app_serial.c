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
#define STATE_TIME 1u       /*!< State to validate the parameters of time */
#define STATE_DATE 2u       /*!< State to validate the parameters of date */
#define STATE_ALARM 3u      /*!< State to validate the parameters of alarm */
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
static uint64_t bufferSerial[BUFFER_MAX_SIZE];  /* cppcheck-suppress misra-c2012-8.9 ; the variable is used in two functions but isn't detect */

/**
 * @brief  APP MSG Structure definition to use the buffer between Serial and Clock
 */
APP_MsgTypeDef bufferClock[45]; /* cppcheck-suppress misra-c2012-8.7 ; the variable is used in two files but isn't detect */

/**
 * @brief  Variable for QUEUE Handle Structure definition
 */
static QUEUE_HandleTypeDef SerialQueue;

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

static void Serial_Stmachine( uint8_t *RxData, APP_MsgTypeDef *SerialMsg );
static void errorMsg( void );
static void validationOK( APP_MsgTypeDef *MsgValidation );

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
    uint8_t RxData[8];

    while( HIL_QUEUE_IsEmptyISR( &SerialQueue, TIM16_FDCAN_IT0_IRQn ) == FALSE ){

        (void)HIL_QUEUE_ReadISR( &SerialQueue, &RxData, TIM16_FDCAN_IT0_IRQn );

        Serial_Stmachine( RxData, &SerialMsg );
    }
}

void Serial_Stmachine( uint8_t *RxData, APP_MsgTypeDef *SerialMsgSt )
{
    static uint16_t year;

    (void)HIL_QUEUE_ReadISR( &SerialQueue, RxData, TIM16_FDCAN_IT0_IRQn );

    RxData[1] = BCDFormatToDecimalFormat(RxData[1]); 

    SerialMsgSt->msg = RxData[1];

    switch (SerialMsgSt->msg)
    {

    case STATE_TIME:

        RxData[2] = BCDFormatToDecimalFormat(RxData[2]);
        RxData[3] = BCDFormatToDecimalFormat(RxData[3]);
        RxData[4] = BCDFormatToDecimalFormat(RxData[4]);

        if( RxData[2] <= 23u ){
            if( RxData[3] <= 59u ){
                if( RxData[4] <= 59u ){
                    SerialMsgSt->tm.tm_hour = RxData[2];
                    SerialMsgSt->tm.tm_min = RxData[3];
                    SerialMsgSt->tm.tm_sec = RxData[4];
                    SerialMsgSt->msg = (uint8_t)SERIAL_MSG_TIME;
                    validationOK( SerialMsgSt );
                }
                else{
                    errorMsg();
                }
            }
            else{
                errorMsg();
            }
        }
        else{
            errorMsg();
        }
        break;

    case STATE_DATE:

        RxData[2] = BCDFormatToDecimalFormat(RxData[2]);
        RxData[3] = BCDFormatToDecimalFormat(RxData[3]);
        RxData[4] = BCDFormatToDecimalFormat(RxData[4]);
        RxData[5] = BCDFormatToDecimalFormat(RxData[5]);
            
        if( (RxData[3] <= 12u) && (RxData[3] > 0u) ){
            year = (RxData[4] * 100u) + RxData[5];    

            if( (year >= 1900u) && (year <= 2100u) ){
                if( (RxData[3] == JAN) || (RxData[3] == MAR) || (RxData[3] == MAY) || (RxData[3] == JUL) || (RxData[3] == AUG) || (RxData[3] == OCT) || (RxData[3] == DEC) ){
                    if( (RxData[2] <= 31u) && (RxData[2] > 0u) ){
                        SerialMsgSt->tm.tm_mday = RxData[2];
                        SerialMsgSt->tm.tm_mon = RxData[3];
                        SerialMsgSt->tm.tm_year = year;
                        SerialMsgSt->msg = (uint8_t)SERIAL_MSG_DATE;
                        validationOK( SerialMsgSt );
                    }  
                    else{
                        errorMsg();
                    }
                }
                else if( (RxData[3] == APR) || (RxData[3] == JUN) || (RxData[3] == SEP) || (RxData[3] == NOV) ){
                    if( (RxData[2] <= 30u) && (RxData[2] > 0u) ){
                        SerialMsgSt->tm.tm_mday = RxData[2];
                        SerialMsgSt->tm.tm_mon = RxData[3];
                        SerialMsgSt->tm.tm_year = year;
                        SerialMsgSt->msg = (uint8_t)SERIAL_MSG_DATE;
                        validationOK( SerialMsgSt );
                    }
                    else{
                        errorMsg();
                    }
                }
                else if( RxData[3] == FEB ){
                    if( (year % 4u) == 0u ){
                        if( (RxData[2] <= 29u) && (RxData[2] > 0u) ){
                            SerialMsgSt->tm.tm_mday = RxData[2];
                            SerialMsgSt->tm.tm_mon = RxData[3];
                            SerialMsgSt->tm.tm_year = year;
                            SerialMsgSt->msg = (uint8_t)SERIAL_MSG_DATE;
                            validationOK( SerialMsgSt );
                        } 
                        else{
                            errorMsg();
                        }
                    }
                    else if( (year % 4u) != 0u ){
                        if( (RxData[2] <= 28u) && (RxData[2] > 0u) ){
                            SerialMsgSt->tm.tm_mday = RxData[2];
                            SerialMsgSt->tm.tm_mon = RxData[3];
                            SerialMsgSt->tm.tm_year = year;
                            SerialMsgSt->msg = (uint8_t)SERIAL_MSG_DATE;
                            validationOK( SerialMsgSt );
                        }
                        else{
                            errorMsg();
                        }
                    }
                    else{
                        errorMsg();
                    }
                }   
                else{
                    errorMsg();
                }
            }
            else{
                errorMsg();
            }
        }
        else{
            errorMsg();
        }
        break;

    case STATE_ALARM:

        RxData[2] = BCDFormatToDecimalFormat(RxData[2]);
        RxData[3] = BCDFormatToDecimalFormat(RxData[3]);

        if( RxData[2] <= 23u ){
            if( RxData[3] <= 59u ){
                SerialMsgSt->tm.tm_hour = RxData[2];
                SerialMsgSt->tm.tm_min = RxData[3];
                SerialMsgSt->msg = (uint8_t)SERIAL_MSG_ALARM;
                validationOK( SerialMsgSt );
            }
            else{
                errorMsg();
            }
        }
        else{
            errorMsg();
        }
    
        break;

    default:
        break;
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
            (void)HIL_QUEUE_Write( &SerialQueue, RxData );
        }
        else{
            errorMsg();
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

void errorMsg( void )
{
    uint8_t TxData[8];
    uint8_t i;

    TxData[0] = CAN_TP;

    for( i = 1u; i < 8u; i++ ){
        TxData[i] = ERROR_TRANSMIT;
    }

    HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, TxData );
}

void validationOK( APP_MsgTypeDef *MsgValidation )
{
    uint8_t TxData[8];
    uint8_t i;

    TxData[0] = CAN_TP;

    for( i = 1u; i < 8u; i++ ){
        TxData[i] = OK_TRANSMIT;
    }

    HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, TxData ); 

    (void)HIL_QUEUE_WriteISR( &ClockQueue, MsgValidation, TIM16_FDCAN_IT0_IRQn );  
}