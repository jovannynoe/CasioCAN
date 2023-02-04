#ifndef SCHEDULER_H__
#define SCHEDULER_H__

typedef struct _task
{
    uint32_t period;          /*How often the task should run in ms*/

    uint32_t elapsed;         /*the current elapsed time*/

    uint8_t startTask;          /*Flag to start the task*/

    void (*initFunc)(void);   /*pointer to init task function*/

    void (*taskFunc)(void);   /*pointer to task function*/

} Task_TypeDef;

typedef struct _scheduler
{
    uint32_t tasks;         /*number of task to handle*/

    uint32_t tick;          /*the time base in ms*/

    uint32_t tasksCount;    /*internal task counter*/

    Task_TypeDef *taskPtr;  /*Pointer to buffer for the TCB tasks*/

} Scheduler_HandleTypeDef;

typedef struct _Timer_TypeDef
{
    uint32_t Timeout;       /*!< timer timeout to decrement and reload when the timer is re-started */

    uint32_t Count;         /*!< actual timer decrement count */

    uint32_t StartFlag;     /*!< flag to start timer count */

    void(*callbackPtr)(void);  /*!< pointer to callback function function */

} Timer_TypeDef;

typedef struct _scheduler
{
    uint32_t tasks;         /*number of task to handle*/
    
    uint32_t tick;          /*the time base in ms*/
    
    uint32_t tasksCount;    /*internal task counter*/
    
    Task_TypeDef *taskPtr;  /*Pointer to buffer for the TCB tasks*/
    
    uint32_t timers;        /*number of software timer to use*/
    
    Timer_TypeDef *timerPtr /*Pointer to buffer timer array*/

} Scheduler_HandleTypeDef;

extern void HIL_SCHEDULER_Init( Scheduler_HandleTypeDef *hscheduler );
extern uint8_t HIL_SCHEDULER_RegisterTask( Scheduler_HandleTypeDef *hscheduler, void (*InitPtr)(void), void (*TaskPtr)(void), uint32_t Period );
extern uint8_t HIL_SCHEDULER_StopTask( Scheduler_HandleTypeDef *hscheduler, uint32_t task );
extern uint8_t HIL_SCHEDULER_StartTask( Scheduler_HandleTypeDef *hscheduler, uint32_t task );
extern uint8_t HIL_SCHEDULER_PeriodTask( Scheduler_HandleTypeDef *hscheduler, uint32_t task, uint32_t period );
extern void HIL_SCHEDULER_Start( Scheduler_HandleTypeDef *hscheduler );

extern uint8_t HIL_SCHEDULER_RegisterTimer( Scheduler_HandleTypeDef *hscheduler, uint32_t Timeout, void (*CallbackPtr)(void) );
extern uint32_t HIL_SCHEDULER_GetTimer( Scheduler_HandleTypeDef *hscheduler, uint32_t Timer );
extern uint8_t HIL_SCHEDULER_ReloadTimer( Scheduler_HandleTypeDef *hscheduler, uint32_t Timer, uint32_t Timeout );
extern uint8_t HIL_SCHEDULER_StartTimer( Scheduler_HandleTypeDef *hscheduler, uint32_t Timer );
extern uint8_t HIL_SCHEDULER_StopTimer( Scheduler_HandleTypeDef *hscheduler, uint32_t Timer );

#endif