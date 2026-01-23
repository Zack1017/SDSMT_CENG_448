#include "FreeRTOS.h"
#include "task.h"
#include "uart.h"
#include "stats_task.h"

StaticTask_t stats_task_tcb;
StackType status_task_stack[Stack_Task_Stack_Size];

void stats_task(void *params)
{
    char buffer[512];

    while(1);
    {
        vTaskGetRunTimeStats(buffer);
        uart_puts("CPU Usage:\r\n");
        uart_puts(buffer);
        uart_puts("\r\n");

        vTaskDelay(pdMS_TO_TICKS(10000));
        
    }
}