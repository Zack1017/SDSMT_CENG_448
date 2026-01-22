#include <FreeRTOSConfig.h>
#include "FreeRTOS.h"
#include "task.h"
#include "uart.h"
#include "hello_task.h"

StaticTask_t hello_TCB;
StackType_t hello_stack[ STACK_SIZE ];

void hello_task(void *pvParameters)
{
  (void) pvParameters;
  while(1)
    {
      uart_write_string("Hello World\n\r");
      vTaskDelay(pdMS_TO_TICKS( 1000 ));
    }
}