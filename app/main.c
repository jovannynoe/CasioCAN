#include "app_bsp.h"
#include "app_serial.h"
#include "app_clock.h"

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
















