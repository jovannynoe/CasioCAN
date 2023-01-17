/**
 * @file    app_serial.h
 * @author  Jovanny Noé Casillas Franco
 * @brief   Header file of Serial
 *
 * The Clock header file is to make the structures and we use in differents files, also, to declarate the functions prototypes
 * that we are using in the source file. 
 * 
 * @note    We need take in count that we have the extern with the declaration of structures because we use MISRA.
 * 
 */
#ifndef APP_SERIAL_H__
#define APP_SERIAL_H__

/**
  * @brief   Structure to save the values to time, date and alarm
  */
typedef struct _APP_TmTypeDef 
{
  uint32_t tm_sec;        /*!< Used to save the seconds between 0 to 59 */

  uint32_t tm_min;        /*!< Used to save the minutes between 0 to 59 */
    
  uint32_t tm_hour;       /*!< Used to save the hour between 0 to 23 */
        
  uint32_t tm_mday;       /*!< Used to save the day of the month between 1 to 31 */
  
  uint32_t tm_mon;        /*!< Used to save the month between 1 to 12 */
        
  uint32_t tm_year;       /*!< Used to save the year between 1900 to 2100 */
        
  uint32_t tm_wday;       /*!< Used to save the day of the week between 0 to 6 */
        
  uint32_t tm_yday;       /*!< Used to save the day in the year between 0 to 365 */
        
  uint32_t tm_isdst;      /*!< Used to save the daylight saving time */            
        
}APP_TmTypeDef;

/**
  * @brief   Structure to know the type of the message and we use another structure inside
  */
typedef struct _APP_MsgTypeDef
{
  uint8_t msg;        /*!< Store the message type to send */

  APP_TmTypeDef tm;   /*!< Time and date in stdlib tm format */

}APP_MsgTypeDef;

extern APP_MsgTypeDef TimeCAN;
extern FDCAN_HandleTypeDef CANHandler;
extern FDCAN_TxHeaderTypeDef CANTxHeader;
extern FDCAN_FilterTypeDef CANFilter;

extern void Serial_Init( void );
extern void Serial_Task( void );
extern uint8_t BCDFormatToDecimalFormat( uint8_t numberBCD );
extern void HAL_FDCAN_RxFifo0Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs );

#endif