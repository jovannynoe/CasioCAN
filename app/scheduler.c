#include "app_bsp.h"
#include "scheduler.h"

#define TRUE 1u
#define FALSE 0u

#define ZERO 0u
#define ONE 1u

void HIL_SCHEDULER_Init( Scheduler_HandleTypeDef *hscheduler )
{
    hscheduler->tasksCount = ZERO;
    hscheduler->taskPtr->elapsed = ZERO;
    hscheduler->taskPtr->period = ZERO;
    hscheduler->taskPtr->startTask = ZERO;
    hscheduler->timersCount = ZERO;
    hscheduler->timerPtr->Count = ZERO;
    hscheduler->timerPtr->StartFlag = ZERO;
    hscheduler->timerPtr->Timeout = ZERO;
}

uint8_t HIL_SCHEDULER_RegisterTask( Scheduler_HandleTypeDef *hscheduler, void (*InitPtr)(void), void (*TaskPtr)(void), uint32_t Period )
{
    uint8_t taskID = ZERO;

    if( (Period > ZERO) && ( (Period % hscheduler->tick) == ZERO) && (TaskPtr != NULL) ){
        hscheduler->taskPtr[hscheduler->tasksCount].initFunc = InitPtr;
        hscheduler->taskPtr[hscheduler->tasksCount].period = Period;
        hscheduler->taskPtr[hscheduler->tasksCount].taskFunc = TaskPtr;
        hscheduler->tasksCount++;

        taskID = ONE;
    }

    return taskID;
}

uint8_t HIL_SCHEDULER_StopTask( Scheduler_HandleTypeDef *hscheduler, uint32_t task )
{
    uint8_t taskStopped;

    if( task > ZERO ){
        hscheduler->taskPtr[task - ONE].startTask = ZERO;
        taskStopped = TRUE;
    }
    else{
        taskStopped = FALSE;
    }

    return taskStopped;
}

uint8_t HIL_SCHEDULER_StartTask( Scheduler_HandleTypeDef *hscheduler, uint32_t task )
{
    uint8_t taskStarted;

    if( task > ZERO ){
        hscheduler->taskPtr[task - ONE].startTask = ONE;
        taskStarted = TRUE;
    }
    else{
        taskStarted = FALSE;
    }

    return taskStarted;
}

uint8_t HIL_SCHEDULER_PeriodTask( Scheduler_HandleTypeDef *hscheduler, uint32_t task, uint32_t period )
{
    uint8_t taskStopped;

    if( ( (period % hscheduler->tick) == ZERO) && (task > ZERO) ){
        hscheduler->taskPtr[task - ONE].period = period;
        taskStopped = TRUE;
    }
    else{
        taskStopped = FALSE;
    }

    return taskStopped;
}

void HIL_SCHEDULER_Start( Scheduler_HandleTypeDef *hscheduler )
{
    static uint32_t tickstart;

    /*We initialize the Init functions.*/
    for( uint32_t i = ZERO; (i < hscheduler->tasks); i++ ){
        if( hscheduler->taskPtr[i].initFunc != NULL ){
            hscheduler->taskPtr[i].initFunc();
        }
    }

    tickstart = HAL_GetTick();
    
    while(1){
        if( (HAL_GetTick() - tickstart) >= (hscheduler->tick) ){
            tickstart = HAL_GetTick();

            /*We run all the timers*/
            for( uint32_t j = ZERO; (j < hscheduler->timers); j++ ){
                if( (hscheduler->timerPtr) != NULL ){
                    if( (hscheduler->timerPtr[j].Count == ZERO) && ( (hscheduler->timerPtr[j].StartFlag) == ONE) ){
                        hscheduler->timerPtr[j].StartFlag = ZERO;
                        hscheduler->timerPtr[j].callbackPtr();
                    }
                    else if( (hscheduler->timerPtr[j].StartFlag) == ONE ){
                        hscheduler->timerPtr[j].Count-=10;
                    }
                    else{
                    }    
                }
                else{
                }
            }

            /*We run all the tasks*/
            for( uint32_t i = ZERO; (i < hscheduler->tasks); i++ ){
                if( (hscheduler->taskPtr[i].elapsed) >= (hscheduler->taskPtr[i].period) ){
                    if( hscheduler->taskPtr[i].taskFunc != NULL ){
                        hscheduler->taskPtr[i].taskFunc();
                        hscheduler->taskPtr[i].elapsed = ZERO;
                    }
                    else{
                    }
                }
                else{
                }
                hscheduler->taskPtr[i].elapsed += hscheduler->tick;
            }
        }
    }  
}

uint8_t HIL_SCHEDULER_RegisterTimer( Scheduler_HandleTypeDef *hscheduler, uint32_t Timeout, void (*CallbackPtr)(void) )
{
    uint8_t timerID;

    if( (Timeout > ZERO) && ( (Timeout % hscheduler->tick) == ZERO) ){
        hscheduler->timerPtr[hscheduler->timersCount].callbackPtr = CallbackPtr;
        hscheduler->timerPtr[hscheduler->timersCount].Timeout = Timeout;
        hscheduler->timersCount++;

        timerID = ONE;
    }
    else{
        timerID = ZERO;
    }

    return timerID;
}

uint32_t HIL_SCHEDULER_GetTimer( Scheduler_HandleTypeDef *hscheduler, uint32_t Timer )
{
    uint32_t currentTimer;

    if( (Timer > ZERO) && (Timer <= hscheduler->timersCount) ){
        currentTimer = (hscheduler->timerPtr[Timer - ONE].Timeout) - (hscheduler->tick); 
    }
    else{
        currentTimer = ZERO;
    }

    return currentTimer;
}

uint8_t HIL_SCHEDULER_ReloadTimer( Scheduler_HandleTypeDef *hscheduler, uint32_t Timer, uint32_t Timeout )
{
    uint8_t itIsRegistered;

    if( /*( (Timeout % hscheduler->tick) == ZERO) &&*/ (Timer > ZERO) && (Timer <= hscheduler->timersCount) ){
        hscheduler->timerPtr[Timer - ONE].Count = Timeout;
        itIsRegistered = TRUE;
    }
    else{
        itIsRegistered = FALSE;
    }

    return itIsRegistered;
}

uint8_t HIL_SCHEDULER_StartTimer( Scheduler_HandleTypeDef *hscheduler, uint32_t Timer )
{
    uint8_t timerStarted;

    if( (Timer > ZERO) && (Timer <= hscheduler->timersCount) ){
        hscheduler->timerPtr[Timer - ONE].Count = hscheduler->timerPtr[Timer - ONE].Timeout;
        hscheduler->timerPtr[Timer - ONE].StartFlag = ONE;
        timerStarted = ONE;
    }
    else{
        timerStarted = ZERO;
    }

    return timerStarted;
}

uint8_t HIL_SCHEDULER_StopTimer( Scheduler_HandleTypeDef *hscheduler, uint32_t Timer )
{
    uint8_t timerStarted;

    if( (Timer > ZERO) && (Timer <= hscheduler->timersCount) ){
        hscheduler->timerPtr[Timer - ONE].StartFlag = ZERO;
        timerStarted = ONE;
    }
    else{
        timerStarted = ZERO;
    }

    return timerStarted;
}
