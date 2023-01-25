#include "app_bsp.h"
#include "hil_queue.h"
#include <stdio.h>
#include <string.h>

#define ZERO 0u
#define ONE 1u

#define ALL_VECTORS 0xFF

void HIL_QUEUE_Init( QUEUE_HandleTypeDef *hqueue )
{
    hqueue->Head = ZERO;
    hqueue->Tail = ZERO;
    hqueue->Empty = ONE;
    hqueue->Full = ZERO;
}

uint8_t HIL_QUEUE_Write( QUEUE_HandleTypeDef *hqueue, void *data )
{
    uint8_t writingIsSuccessful = ZERO;

    if( hqueue->Full == ZERO ){
        memcpy( hqueue->Buffer + hqueue->Head, data, hqueue->Size );
        hqueue->Head += hqueue->Size % hqueue->Elements;

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

    if( HIL_QUEUE_IsEmpty( hqueue ) == 0 ){
        memcpy( hqueue->Buffer + hqueue->Tail, data, hqueue->Size );
        hqueue->Tail += hqueue->Size % hqueue->Elements;
        readingIsSuccessful = ONE;
    }
    else{
        readingIsSuccessful = ZERO;
    }
    return readingIsSuccessful;
}

uint8_t HIL_QUEUE_IsEmpty( QUEUE_HandleTypeDef *hqueue )
{
    uint8_t queueIsEmpty = ZERO;

    if( (hqueue->Head == hqueue->Tail) && (hqueue->Full == 0) ){
        queueIsEmpty = ONE;
    }
    else{
        queueIsEmpty = ZERO;
    }
    return queueIsEmpty;
}

void HIL_QUEUE_Flush( QUEUE_HandleTypeDef *hqueue )
{   
    hqueue->Head = ZERO;
    hqueue->Tail = ZERO;
    hqueue->Empty = ONE;
    hqueue->Full = ZERO;
    memset( hqueue->Buffer, ' ', strlen(hqueue->Buffer) );
}

uint8_t HIL_QUEUE_WriteISR( QUEUE_HandleTypeDef *hqueue, void *data, uint8_t isr )
{
    uint8_t writingIsSuccessful = ZERO;
    uint8_t i;

    if( isr == ALL_VECTORS ){
        for( i = 0; i <= 30; i++ ){
            HAL_NVIC_DisableIRQ( i );
        }   
    }
    else{
        HAL_NVIC_DisableIRQ( isr );
    }

    memcpy( hqueue->Buffer, data, hqueue->Size );

    if( memcmp( hqueue->Buffer, data, hqueue->Size ) == ZERO ){
        writingIsSuccessful = ONE;
        hqueue->Head = ( hqueue->Head + ONE ) % hqueue->Elements;
    }
    else{
        writingIsSuccessful = ZERO;
    }

    if( isr == ALL_VECTORS ){
        for( i = 0; i <= 30; i++ ){
            HAL_NVIC_EnableIRQ( i );
        }   
    }
    else{
        HAL_NVIC_EnableIRQ( isr );
    }

    return writingIsSuccessful; 
}

uint8_t HIL_QUEUE_ReadISR( QUEUE_HandleTypeDef *hqueue, void *data, uint8_t isr )
{
    uint8_t readingIsSuccessful = ZERO;
    uint8_t i;

    if( isr == ALL_VECTORS ){
        for( i = 0; i <= 30; i++ ){
            HAL_NVIC_DisableIRQ( i );
        }   
    }
    else{
        HAL_NVIC_DisableIRQ( isr );
    }

    memcpy( data, hqueue->Buffer, hqueue->Size );

    if( memcmp( data, hqueue->Buffer, hqueue->Size ) == ZERO ){
        readingIsSuccessful = ONE;
        hqueue->Tail = ( hqueue->Tail + ONE ) % hqueue->Elements;
    }
    else{
        readingIsSuccessful = ZERO;
    }

    if( isr == ALL_VECTORS ){
        for( i = 0; i <= 30; i++ ){
            HAL_NVIC_EnableIRQ( i );
        }   
    }
    else{
        HAL_NVIC_EnableIRQ( isr );
    }

    return readingIsSuccessful;
}

uint8_t HIL_QUEUE_IsEmptyISR( QUEUE_HandleTypeDef *hqueue, uint8_t isr )
{
    uint8_t queueIsEmpty = ZERO;
    uint8_t i;

    if( isr == ALL_VECTORS ){
        for( i = 0; i <= 30; i++ ){
            HAL_NVIC_DisableIRQ( i );
        }   
    }
    else{
        HAL_NVIC_DisableIRQ( isr );
    }

    if( ( hqueue->Full == ZERO ) || ( hqueue->Full != hqueue->Size ) ){
        queueIsEmpty = ONE;
    }
    else{
        queueIsEmpty = ZERO;
    }

    if( isr == ALL_VECTORS ){
        for( i = 0; i <= 30; i++ ){
            HAL_NVIC_EnableIRQ( i );
        }   
    }
    else{
        HAL_NVIC_EnableIRQ( isr );
    }

    return queueIsEmpty;
}

void HIL_QUEUE_FlushISR( QUEUE_HandleTypeDef *hqueue, uint8_t isr )
{
    uint8_t i;

    if( isr == ALL_VECTORS ){
        for( i = 0; i <= 30; i++ ){
            HAL_NVIC_DisableIRQ( i );
        }   
    }
    else{
        HAL_NVIC_DisableIRQ( isr );
    }

    hqueue->Head = ZERO;
    hqueue->Tail = ZERO;
    hqueue->Empty = ONE;
    hqueue->Full = ZERO;
    memset( hqueue->Buffer, ' ', strlen(hqueue->Buffer) );

    if( isr == ALL_VECTORS ){
        for( i = 0; i <= 30; i++ ){
            HAL_NVIC_EnableIRQ( i );
        }   
    }
    else{
        HAL_NVIC_EnableIRQ( isr );
    }
}