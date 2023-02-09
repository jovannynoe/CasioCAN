#include "app_bsp.h"
#include "hil_queue.h"
#include <string.h>

#define ZERO 0u
#define ONE 1u

#define ALL_VECTORS 0xFFu

void HIL_QUEUE_Init( QUEUE_HandleTypeDef *hqueue )
{
    hqueue->Head = ZERO;
    hqueue->Tail = ZERO;
    hqueue->Empty = ONE;
    hqueue->Full = ZERO;
    hqueue->CounterWrite = ZERO;
    hqueue->CounterRead = ZERO;
}

uint8_t HIL_QUEUE_Write( QUEUE_HandleTypeDef *hqueue, void *data )
{
    uint8_t writingIsSuccessful = ZERO;

    if( hqueue->Full == ZERO ){
        memcpy( hqueue->Buffer + hqueue->Head, data, hqueue->Size );  
        hqueue->Head += hqueue->Size % hqueue->Elements;
        hqueue->CounterWrite++;

        if( hqueue->CounterWrite == hqueue->Elements ){
            hqueue->Full = ONE;
            hqueue->Head = ZERO;
            hqueue->CounterWrite = ZERO;
        }
        else{    
        }

        writingIsSuccessful = ONE;
    }
    else{
        writingIsSuccessful = ZERO;
    }

    return writingIsSuccessful;
}

uint8_t HIL_QUEUE_Read( QUEUE_HandleTypeDef *hqueue, void *data )
{
    uint8_t readingIsSuccessful = ZERO;

    if( HIL_QUEUE_IsEmpty( hqueue ) == ZERO ){
        memcpy( data, hqueue->Buffer + hqueue->Tail, hqueue->Size );
        hqueue->Tail += hqueue->Size % hqueue->Elements;
        hqueue->CounterRead++;
        readingIsSuccessful = ONE;

        if( hqueue->CounterRead == hqueue->Elements ){
            hqueue->Full = ZERO;
            hqueue->CounterRead = ZERO;
            hqueue->Tail = ZERO;
        }
    }
    else{
        readingIsSuccessful = ZERO;
    }
    return readingIsSuccessful;
}

uint8_t HIL_QUEUE_IsEmpty( QUEUE_HandleTypeDef *hqueue )
{
    uint8_t queueIsEmpty = ZERO;

    if( (hqueue->Head == hqueue->Tail) && (hqueue->Full == ZERO) ){
        queueIsEmpty = ONE;
    }
    else{
        queueIsEmpty = ZERO;
    }
    return queueIsEmpty;
}

void HIL_QUEUE_Flush( QUEUE_HandleTypeDef *hqueue ) /* cppcheck-suppress misra-c2012-8.7 ; the function isn't used in another file but can be used */
{   
    hqueue->Head = ZERO;
    hqueue->Tail = ZERO;
    hqueue->Empty = ONE;
    hqueue->Full = ZERO;
    hqueue->CounterWrite = ZERO;
    hqueue->CounterRead = ZERO;
    memset( hqueue->Buffer, ' ', strlen(hqueue->Buffer) );
}

uint8_t HIL_QUEUE_WriteISR( QUEUE_HandleTypeDef *hqueue, void *data, uint8_t isr )
{
    uint8_t writingIsSuccessful = ZERO;

    if( isr == ALL_VECTORS ){
        __disable_irq();   
    }
    else{
        HAL_NVIC_DisableIRQ( isr );
    }

    writingIsSuccessful = HIL_QUEUE_Write( hqueue, data );

    if( isr == ALL_VECTORS ){
        __enable_irq();   
    }
    else{
        HAL_NVIC_EnableIRQ( isr );
    }

    return writingIsSuccessful;
}

uint8_t HIL_QUEUE_ReadISR( QUEUE_HandleTypeDef *hqueue, void *data, uint8_t isr )
{
    uint8_t readingIsSuccessful = ZERO;

    if( isr == ALL_VECTORS ){
        __disable_irq(); 
    }
    else{
        HAL_NVIC_DisableIRQ( isr );
    }

    readingIsSuccessful = HIL_QUEUE_Read( hqueue, data );   

    if( isr == ALL_VECTORS ){
        __enable_irq(); 
    }
    else{
        HAL_NVIC_EnableIRQ( isr );
    }

    return readingIsSuccessful;
}

uint8_t HIL_QUEUE_IsEmptyISR( QUEUE_HandleTypeDef *hqueue, uint8_t isr )
{
    uint8_t queueIsEmpty = ZERO;

    if( isr == ALL_VECTORS ){
        __disable_irq();  
    }
    else{
        HAL_NVIC_DisableIRQ( isr );
    }

    queueIsEmpty = HIL_QUEUE_IsEmpty( hqueue );

    if( isr == ALL_VECTORS ){
        __enable_irq();   
    }
    else{
        HAL_NVIC_EnableIRQ( isr );
    }

    return queueIsEmpty;
}

void HIL_QUEUE_FlushISR( QUEUE_HandleTypeDef *hqueue, uint8_t isr )
{
    if( isr == ALL_VECTORS ){
        __disable_irq();   
    }
    else{
        HAL_NVIC_DisableIRQ( isr );
    }

    HIL_QUEUE_Flush( hqueue );

    if( isr == ALL_VECTORS ){
        __enable_irq();  
    }
    else{
        HAL_NVIC_EnableIRQ( isr );
    }
}