#include "app_bsp.h"
#include "app_serial.h"
#include "app_clock.h"

#define WWDG_WINDOW 0x50
#define WWDG_COUNTER 0x7F

static void Heart_Init( void );
static void Heart_Beat( void );
static void Dog_Init( void );
static void Peth_The_Dog( void );

WWDG_HandleTypeDef hwwdg;
GPIO_InitTypeDef GPIO_InitStruct;

static uint32_t tickstartHeartBeat;
static uint32_t tickstartPethTheDog;

int main( void )
{
    HAL_Init();
    Heart_Init();
    Dog_Init();
    Clock_Init();
    Serial_Init();

    while(1){

        Serial_Task();
        Clock_Task();
        Heart_Beat();
        Peth_The_Dog();
    }
}

void Heart_Init( void )
{
    __GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin  = 0xFF;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );

    tickstartHeartBeat = HAL_GetTick();
}

void Heart_Beat( void )
{
    if( (HAL_GetTick() - tickstartHeartBeat) >= 300 ){
        tickstartHeartBeat = HAL_GetTick();
        HAL_GPIO_TogglePin( GPIOC, GPIO_PIN_0 );
    }
}

void Dog_Init( void )
{
    if( __HAL_RCC_GET_FLAG( RCC_FLAG_WWDGRST ) != RESET ){
        HAL_GPIO_WritePin( GPIOC, GPIO_PIN_7, SET );
        HAL_Delay(3000);
        HAL_GPIO_WritePin( GPIOC, GPIO_PIN_7, RESET );
    }

    __HAL_RCC_CLEAR_RESET_FLAGS();
    __HAL_RCC_WWDG_CLK_ENABLE();

    hwwdg.Instance = WWDG;
    hwwdg.Init.Prescaler = WWDG_PRESCALER_8;
    hwwdg.Init.Window = WWDG_WINDOW;    /*WWDG_WINDOW; 80 = 25ms*/
    hwwdg.Init.Counter = WWDG_COUNTER;  /*WWDG_COUNTER; 127 = 32ms*/
    hwwdg.Init.EWIMode = WWDG_EWI_DISABLE;
    HAL_WWDG_Init( &hwwdg );

    tickstartPethTheDog = HAL_GetTick();
}

void Peth_The_Dog( void )
{
    if( (HAL_GetTick() - tickstartPethTheDog) >= 30 ){  /*25 - 32*/
        tickstartPethTheDog = HAL_GetTick();
        HAL_WWDG_Refresh( &hwwdg );
    }
}
















