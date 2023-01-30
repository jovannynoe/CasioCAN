#ifndef SCHEDULER_H__
#define SCHEDULER_H__

typedef struct _scheduler
{
    uint32_t tasks;         /*number of task to handle*/

    uint32_t tick;          /*the time base in ms*/

    uint32_t tasksCount;    /*internal task counter*/

    Task_TypeDef *taskPtr;  /*Pointer to buffer for the TCB tasks*/

}Scheduler_HandleTypeDef;

typedef struct _task
{
    uint32_t period;          /*How often the task shopud run in ms*/

    uint32_t elapsed;         /*the cuurent elapsed time*/

    void (*initFunc)(void);   /*pointer to init task function*/

    void (*taskFunc)(void);   /*pointer to task function*/

}Task_TypeDef;

extern void HIL_SCHEDULER_Init( Scheduler_HandleTypeDef *hscheduler );
extern uint8_t HIL_SCHEDULER_RegisterTask( Scheduler_HandleTypeDef *hscheduler, void (*InitPtr)(void), void (*TaskPtr)(void), uint32_t Period );
extern uint8_t HIL_SCHEDULER_StopTask( Scheduler_HandleTypeDef *hscheduler, uint32_t task );
extern uint8_t HIL_SCHEDULER_StartTask( Scheduler_HandleTypeDef *hscheduler, uint32_t task );
extern uint8_t HIL_SCHEDULER_PeriodTask( Scheduler_HandleTypeDef *hscheduler, uint32_t task, uint32_t period );
extern void HIL_SCHEDULER_Start( Scheduler_HandleTypeDef *hscheduler );

#endif