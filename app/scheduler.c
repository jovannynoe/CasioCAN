#include "app_bsp.h"
#include "scheduler.h"

#define TRUE 1u
#define FALSE 0u

#define ZERO 0u
#define ONE 1u

void HIL_SCHEDULER_Init( Scheduler_HandleTypeDef *hscheduler )
{
    hscheduler->taskPtr->elapsed = ZERO;
    hscheduler->taskPtr->period = ZERO;
    hscheduler->taskPtr->startTask = ZERO;
    hscheduler->tasks = ZERO;
    hscheduler->tasksCount = ZERO;
}

uint8_t HIL_SCHEDULER_RegisterTask( Scheduler_HandleTypeDef *hscheduler, void (*InitPtr)(void), void (*TaskPtr)(void), uint32_t Period )
{
    uint8_t taskID;

    if( (Period > hscheduler->tick) && (Period % hscheduler->tick == ZERO) ){
        hscheduler->taskPtr[hscheduler->tasksCount].initFunc = InitPtr;
        hscheduler->taskPtr[hscheduler->tasksCount].period = Period;
        hscheduler->taskPtr[hscheduler->tasksCount].taskFunc = TaskPtr;
        hscheduler->tasksCount++;

        taskID = ONE;
    }
    else if( (Period > hscheduler->tick) && (Period % hscheduler->tick == ZERO) && (InitPtr == NULL) ){
        hscheduler->taskPtr[hscheduler->tasksCount].initFunc = InitPtr;
        hscheduler->taskPtr[hscheduler->tasksCount].period = Period;
        hscheduler->taskPtr[hscheduler->tasksCount].taskFunc = TaskPtr;
        hscheduler->tasksCount++;

        taskID = ONE;
    } 
    else{
        taskID = ZERO;
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
        taskStopped = FALSE;
    }
    else{
        taskStopped = TRUE;
    }

    return taskStopped;
}

void HIL_SCHEDULER_Start( Scheduler_HandleTypeDef *hscheduler )
{
    
}