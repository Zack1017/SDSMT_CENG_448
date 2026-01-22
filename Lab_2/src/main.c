#include <FreeRTOSConfig.h>
#include <FreeRTOS.h>
#include <task.h>
#include <uart.h>
#include "hello_task.h"

int main( void )
{
  uart_init( 115200 );
  xTaskCreateStatic( hello_task,
                     "HelloTask",
                     STACK_SIZE,
                     NULL,
                     TASK_PRIORITY,
                     hello_stack,
                     &hello_TCB 
  );
  vTaskStartScheduler();
  while(1);//bad
  
}

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}