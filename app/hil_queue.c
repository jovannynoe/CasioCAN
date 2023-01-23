#include "app_bsp.h"
#include "hil_queue.h"

void HIL_QUEUE_Init( QUEUE_HandleTypeDef *hqueue )
{
    hqueue->Head = 0u;
    hqueue->Tail = 0u;
    hqueue->Empty = 1u;
    hqueue->Full = 0u;
}

uint8_t HIL_QUEUE_Write( QUEUE_HandleTypeDef *hqueue, void *data )
{
    uint8_t writingIsSuccessful = 0u;

    memcpy( hqueue->Buffer, data, hqueue->Size );

    if( memcmp( hqueue->Buffer, data, hqueue->Size ) == 0 ){
        writingIsSuccessful = 1u;
        hqueue->Head = ( hqueue->Head + 1 ) % hqueue->Elements;
    }
    else{
        writingIsSuccessful = 0u;
    }

    return writingIsSuccessful;
}

uint8_t HIL_QUEUE_Read( QUEUE_HandleTypeDef *hqueue, void *data )
{
    uint8_t readingIsSuccessful = 0u;

    memcpy( data, hqueue->Buffer, hqueue->Size );

    if( memcmp( data, hqueue->Buffer, hqueue->Size ) == 0 ){
        readingIsSuccessful = 1u;
        hqueue->Tail = ( hqueue->Tail + 1u ) % hqueue->Elements;
    }
    else{
        readingIsSuccessful = 0u;
    }
    return readingIsSuccessful;
}

uint8_t HIL_QUEUE_IsEmpty( QUEUE_HandleTypeDef *hqueue )
{
    uint8_t queueIsEmpty = 0u;

    if( ( hqueue->Full == 0u ) || ( hqueue->Full != hqueue->Size ) ){
        queueIsEmpty = 1u;
    }
    else{
        queueIsEmpty = 0u;
    }
    return queueIsEmpty;
}

void HIL_QUEUE_Flush( QUEUE_HandleTypeDef *hqueue )
{   
    hqueue->Head = 0u;
    hqueue->Tail = 0u;
    hqueue->Empty = 1u;
    hqueue->Full = 0u;
    hqueue->Buffer = 0;   
}

uint8_t HIL_QUEUE_WriteISR( QUEUE_HandleTypeDef *hqueue, void *data, uint8_t isr )
{

}

uint8_t HIL_QUEUE_ReadISR( QUEUE_HandleTypeDef *hqueue, void *data, uint8_t isr )
{

}

uint8_t HIL_QUEUE_IsEmptyISR( QUEUE_HandleTypeDef *hqueue, uint8_t isr )
{

}

uint8_t HIL_QUEUE_FlushISR( QUEUE_HandleTypeDef *hqueue, uint8_t isr )
{

}