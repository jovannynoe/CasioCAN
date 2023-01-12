#include "app_bsp.h"
#include "app_serial.h"
#include "app_clock.h"

void Heart_Init( void );
void Heart_Beat( void );

uint32_t tickstartHeartBeat;

int main( void )
{
    HAL_Init();
    Clock_Init();
    Serial_Init();

    while(1){

        Serial_Task();
        Clock_Task();
    }
}

void Heart_Init( void )
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin  = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

    tickstartHeartBeat = HAL_GetTick();
}

void Heart_Beat( void )
{
    if( (HAL_GetTick() - tickstartHeartBeat) >= 300 ){
        tickstartHeartBeat = HAL_GetTick();
        HAL_GPIO_TogglePin( GPIOA, GPIO_PIN_5 );
    }
}
















