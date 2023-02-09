/**
 * @file    app_ints.c
 * @author  Jovanny Noé Casillas Franco
 * @brief   Source file of interrupts.
 *
 * File with the interruption functions from the microcontroller
 * 
 * @note    Review file startup_stm32g0b1.S for more information
 *          
 */
#include "app_bsp.h"
#include "app_serial.h"

void SysTick_Handler( void );
void EXTI4_15_IRQHandler( void );
void TIM16_FDCAN_IT0_IRQHandler( void );
void RTC_TAMP_IRQHandler( void );

/**
 * @brief   Function to can use the Systick timer.
 *
 * Function to use the concurrent processes with the timer SysTick
 *
 * @retval  None
 */
void SysTick_Handler( void )
{
    HAL_IncTick( );
}

/**
 * @brief   EXTI IRQHandler.
 *
 * Function to use external interrupts with the function Handle EXTI interrupt request.
 *
 * @retval  None
 */
void EXTI4_15_IRQHandler( void )
{
    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_9 );
}

/**
 * @brief   FDCAN IRQHandler.
 *
 * Function to when we received or transmit a message through Handles FDCAN interrupt request.
 *
 * @retval  None
 */
void TIM16_FDCAN_IT0_IRQHandler( void )
{
    HAL_FDCAN_IRQHandler( &CANHandler );
}

void RTC_TAMP_IRQHandler( void )
{
    HAL_RTC_AlarmIRQHandler( &RTC_Structure );
}

