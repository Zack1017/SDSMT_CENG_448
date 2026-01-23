#include <FreeRTOSConfig.h>
#include "FreeRTOS.h"
#include "task.h"
#include "uart.h"
#include "stats_task.h"

#if(configUSE_STATISTICS_FORMATTING_FUNCTIONS != 0)
#error configUSE_STATISTICS_FORMATTING_FUNCTIONS is not 1
#endif

StaticTask_t stats_task_tcb;
StackType_t stats_task_stack[STATS_TASK_STACK_SIZE];

void stats_task(void *params)
{
    char buffer[512];
    while(1)
    {
        vTaskGetRunTimeStats(buffer);
        uart_write_string("CPU Usage:\r\n");
        uart_write_string(buffer);
        uart_write_string("\r\n");

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}