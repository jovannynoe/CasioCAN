/**------------------------------------------------------------------------------------------------
 * Archivo con la funcion principal del programa main, coloque aqui el codigo de su aplicacion
-------------------------------------------------------------------------------------------------*/
#include "app_bsp.h"
#include <stdio.h>

/**------------------------------------------------------------------------------------------------
Brief.- Punto de entrada del programa
-------------------------------------------------------------------------------------------------*/
int main( void )
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    uint32_t i = 0;


    HAL_Init( );

    __HAL_RCC_GPIOA_CLK_ENABLE( );

    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pin   = GPIO_PIN_5;
    HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

    printf("Hola semihosting\n\r");

    for( ; ; )
    {
        HAL_GPIO_TogglePin( GPIOA, GPIO_PIN_5 );
        HAL_Delay( 1000u );
        printf("la variable i = %d\n\r", i);
        i++;
    }

    return 0u;
}

