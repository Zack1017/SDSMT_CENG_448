#include <FreeRTOSConfig.h>
#include <FreeRTOS.h>
#include <task.h>
#include <uart.h>
#include "hello_task.h"
#include "stats_task.h"

int main( void )
{
  uart_init( 115200 );
  xTaskCreateStatic(hello_task,
                    "HelloTask",
                    STACK_SIZE,
                    NULL,
                    TASK_PRIORITY,
                    hello_stack,
                    &hello_TCB 
  );
  
  xTaskCreateStatic(stats_task,
                    "Stats",
                    STATS_TASK_STACK_SIZE,
                    NULL,
                    STATS_TASK_PRIORITY,
                    stats_task_stack, 
                    &stats_task_tcb
  );
  vTaskStartScheduler();
  while(1);//bad
  
}

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside
this function then they must be declared static - otherwise they will
be allocated on the stack and so not exists after this function
exits. */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the
    Idle task's state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vConfigureTimerForRunTimeStats( void )
{
    /* Nothing needed as timer is configured in setup_stats_timer() */
}