/**
 * @file    app_msps.c
 * @author  Jovanny Noé Casillas Franco
 * @brief   **Source file of auxiliar functions**
 *
 * File with auxiliary functions of the microcontroller and their initializations of the library
 * 
 * @note    We have only the necessary functions
 *          
 */
#include "app_bsp.h"

void HAL_MspInit( void );
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef *hfdcan);
void HAL_RTC_MspInit( RTC_HandleTypeDef* hrtc );
void HAL_SPI_MspInit( SPI_HandleTypeDef *hspi );
void HEL_LCD_MspInit( LCD_HandleTypeDef *hlcd );

/**
 * @brief   **Function to initialize the osc and clock**
 *
 * Function to configurate the clock to 64MHz where we configurate the main internal regulator 
 * output voltage and we initilize the RCC Oscillators according to the spedified parameters in 
 * the RCC_OscInitTypeDef structure. Also, we initialize the CPU, AHB and APB buses clocks.
 *
 * @retval  None
 */
void HAL_MspInit( void )
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    /** Configure the main internal regulator output voltage*/
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* Initializes the RCC Oscillators according to the specified parameters in the RCC_OscInitTypeDef structure
    The frequency set is 64MHz with the internal 16MHz HSI oscilator. According to the formulas:
    fVCO = fPLLIN x ( N / M ) = 16MHz x (8 / 1) = 128MHz
    fPLLP = fVCO / P = 128MHz / 2 = 64MHz
    fPLLQ = fVCO / Q = 128MHz / 2 = 64MHz
    fPLLR = fVCO / R = 128MHz / 2 = 64MHz
    */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSIDiv              = RCC_HSI_DIV1;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM            = RCC_PLLM_DIV1;
    RCC_OscInitStruct.PLL.PLLN            = 8;
    RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ            = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR            = RCC_PLLR_DIV2;
    HAL_RCC_OscConfig( &RCC_OscInitStruct );

    /** Initializes the CPU, AHB and APB buses clocks*/
    RCC_ClkInitStruct.ClockType       = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource    = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider   = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider  = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_2 ); 
}

/**
 * @brief   **Function to initialize and configurate FDCAN**
 *
 * Function to configurate the pins that we will use to can transmit and receive data through of CAN protocol
 * defining the pin 0 in port D for Rx and pin 1 in port D for Tx in altern mode. Also, we have the NVIC 
 * functions to set the priority and the enable of the interruption.
 *
 * @param   <*hfdcan>[out] Parameter that is used only in the function because it is a requirement
 *
 * @retval  None
 * 
 */
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef *hfdcan)
{
    *hfdcan = *hfdcan;

    GPIO_InitTypeDef GpioCanStruct;

    /* Habilitamos los relojes de los perifericos GPIO y CAN */
    __HAL_RCC_FDCAN_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /* configuramos pin 0(rx) y pin 1(tx) en modo alterno para FDCAN1 */
    GpioCanStruct.Mode = GPIO_MODE_AF_PP;
    GpioCanStruct.Alternate = GPIO_AF3_FDCAN1;
    GpioCanStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GpioCanStruct.Pull = GPIO_NOPULL;
    GpioCanStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init( GPIOD, &GpioCanStruct );

    HAL_NVIC_SetPriority( TIM16_FDCAN_IT0_IRQn, 2, 0 );
    HAL_NVIC_EnableIRQ( TIM16_FDCAN_IT0_IRQn );
}

/**
 * @brief   **Function to initialize and configurate RTC**
 *
 * Function to enable backup domain, reset previous RTC source clock, configurate LSE/LSI as RTC
 * clock source, set LSE as source clock and enable peripheral clock.
 * 
 * @param   <*hfdcan>[out] Parameter that is used only in the function because it is a requirement
 *
 * @retval  None
 * 
 */
void HAL_RTC_MspInit( RTC_HandleTypeDef* hrtc )
{
    *hrtc = *hrtc;
    RCC_OscInitTypeDef        RCC_OscInitStruct = {0};
    RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct = {0};

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    /*Enable backup domain*/
    HAL_PWREx_ControlVoltageScaling( PWR_REGULATOR_VOLTAGE_SCALE1 );
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_LSEDRIVE_CONFIG( RCC_LSEDRIVE_LOW );

    /*reset previous RTC source clock*/
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_NONE;
    HAL_RCCEx_PeriphCLKConfig( &PeriphClkInitStruct );

    /* Configure LSE/LSI as RTC clock source */
    RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
    HAL_RCC_OscConfig( &RCC_OscInitStruct );

    /*Set LSE as source clock*/
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    HAL_RCCEx_PeriphCLKConfig( &PeriphClkInitStruct );

    /* Peripheral clock enable */
    __HAL_RCC_RTC_ENABLE();
    __HAL_RCC_RTCAPB_CLK_ENABLE();
}

void HAL_SPI_MspInit( SPI_HandleTypeDef *hspi )
{
    *hspi = *hspi;

    GPIO_InitTypeDef GPIO_SpiStruct;

    __GPIOB_CLK_ENABLE();
    __SPI2_CLK_ENABLE();

    GPIO_SpiStruct.Pin = GPIO_PIN_11 | GPIO_PIN_13 | GPIO_PIN_14;  /*SCK | MOSI | MISO*/
    GPIO_SpiStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_SpiStruct.Pull = GPIO_PULLUP;
    GPIO_SpiStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_SpiStruct.Alternate = GPIO_AF0_SPI2;
    HAL_GPIO_Init(GPIOB, &GPIO_SpiStruct);
}

void HEL_LCD_MspInit( LCD_HandleTypeDef *hlcd )
{
    *hlcd = *hlcd;

    GPIO_InitTypeDef GPIO_LCDStruct;

    __GPIOB_CLK_ENABLE();   /*Habilitamos reloj del puerto B*/
    __GPIOC_CLK_ENABLE();   /*Habilitamos reloj del puerto C*/

    GPIO_LCDStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_12; /*BKL | RS | RST*/
    GPIO_LCDStruct.Mode  = GPIO_MODE_OUTPUT_PP;    /*Salida tipo push-pull*/
    GPIO_LCDStruct.Pull  = GPIO_NOPULL;    /*Pin sin pull-up ni pull-down*/
    GPIO_LCDStruct.Speed = GPIO_SPEED_FREQ_LOW;    /*Pin a baja velocidad*/
    /*Inicializamos pines con los parametros anteriores*/
    HAL_GPIO_Init( GPIOC, &GPIO_LCDStruct );

    GPIO_LCDStruct.Pin = GPIO_PIN_15; /*CS*/
    GPIO_LCDStruct.Mode  = GPIO_MODE_OUTPUT_PP;    /*Salida tipo push-pull*/
    GPIO_LCDStruct.Pull  = GPIO_NOPULL;    /*Pin sin pull-up ni pull-down*/
    GPIO_LCDStruct.Speed = GPIO_SPEED_FREQ_LOW;    /*Pin a baja velocidad*/
    /*Inicializamos pines con los parametros anteriores*/
    HAL_GPIO_Init( GPIOB, &GPIO_LCDStruct );
}