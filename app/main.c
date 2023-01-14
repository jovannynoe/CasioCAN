/**
 * @file    main.c
 * @author  Jovanny Noé Casillas Franco
 * @brief   **This file is to run the functions created in files app_serial and app_clock also some others functions created here**
 *
 * On the main file we run the functions from the other files and if we have functions in this file. First we run the HAL function,
 * later the others functions to initialize the drivers and in the while we run the functions with tasks. In this file we created 
 * two more functions, one for a heartbeat and another for the WWDG.
 * 
 * @note    Only the files inside folder app will be take them into account when the
 *          doxygen runs by typing "make docs", index page is generated in
 *          Build/doxigen/html/index.html
 *          
 */
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

/**
 * @brief   **This function is the main where we run everything the functions**
 *
 * In main function we run everything the functions, in the first part just we run the 
 * functions to initilize some peripherals or protocols and in the while is own infinite
 * loop where we run the tasks of externs functions and local functions created in the
 * main file.
 *
 * @retval  None
 *
 */
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

/**
 * @brief   **This heart init function is to configurate the led to heartbeat**
 *
 * In this function we go to configurate like output the led to the heartbeat and
 * initializing for the first time the tickstart for the heartbeat.
 *
 * @retval  None
 *
 */
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

/**
 * @brief   **Provide a brief fucntion description (just one line)**
 *
 * Provide the proper and detailed description for your function, you can use as many
 * lines as you want but keep each line less than 100 characteres
 *
 * @param   <param name>[in] Parameer description based on what does
 * @param   <param name>[out] add one line per paramter and indicates if input or output
 *
 * @retval  Describe the return value and its potential values, 
 *          if void type None
 *
 * @note This is optional, just in case something very special needs to be take into account
 */
void Heart_Beat( void )
{
    if( (HAL_GetTick() - tickstartHeartBeat) >= 300 ){
        tickstartHeartBeat = HAL_GetTick();
        HAL_GPIO_TogglePin( GPIOC, GPIO_PIN_0 );
    }
}

/**
 * @brief   **Provide a brief fucntion description (just one line)**
 *
 * Provide the proper and detailed description for your function, you can use as many
 * lines as you want but keep each line less than 100 characteres
 *
 * @param   <param name>[in] Parameer description based on what does
 * @param   <param name>[out] add one line per paramter and indicates if input or output
 *
 * @retval  Describe the return value and its potential values, 
 *          if void type None
 *
 * @note This is optional, just in case something very special needs to be take into account
 */
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

/**
 * @brief   **Provide a brief fucntion description (just one line)**
 *
 * Provide the proper and detailed description for your function, you can use as many
 * lines as you want but keep each line less than 100 characteres
 *
 * @param   <param name>[in] Parameer description based on what does
 * @param   <param name>[out] add one line per paramter and indicates if input or output
 *
 * @retval  Describe the return value and its potential values, 
 *          if void type None
 *
 * @note This is optional, just in case something very special needs to be take into account
 */
void Peth_The_Dog( void )
{
    if( (HAL_GetTick() - tickstartPethTheDog) >= 30 ){  /*25 - 32*/
        tickstartPethTheDog = HAL_GetTick();
        HAL_WWDG_Refresh( &hwwdg );
    }
}
















