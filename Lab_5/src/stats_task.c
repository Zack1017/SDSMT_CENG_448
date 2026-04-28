#include <FreeRTOS.h>
#include <stats_task.h>
#include <task.h>
#include <stdio.h>
#include <UART_16550.h>
#include <AXI_timer.h>
#include <ANSI_terminal.h>
#include <uart_driver_table.h>

static int stats_counter=0;

int get_stats_counter()
{
  return stats_counter;
}
  
void setup_stats_timer()
{
}

static void stats_handler()
{
  stats_counter++;
}

void stats_task(void *pvParameters)
{
  static char stats_buffer[1024];
  static char mem_buffer[64];
  size_t heapsize;
  int timer;
  timer = AXI_TIMER_allocate();
  AXI_TIMER_set_handler(timer,stats_handler);
  AXI_TIMER_set_repeating(timer,AXI_TIMER_HZ_TO_COUNT(20000));

  while(1)
    {
      vTaskGetRunTimeStats(stats_buffer);
      heapsize = xPortGetFreeHeapSize();
      sprintf(mem_buffer,"Heap Used: %u\n",(0xFFFFFFFF)-heapsize);
      ANSI_uart.tx_lock(UART1,portMAX_DELAY);
      ANSI_clear(UART1);
      ANSI_moveTo(UART1,3,0);\
      ANSI_uart.write_string(UART1,mem_buffer,portMAX_DELAY);
      ANSI_moveTo(UART1,5,0);
      ANSI_uart.write_string(UART1,stats_buffer,portMAX_DELAY);
      ANSI_uart.tx_unlock(UART1);
      vTaskDelay(pdMS_TO_TICKS( 5000 ));
    }
}

/* Structure that will hold the TCB of the task being created. */
StaticTask_t stats_TCB;

/* Buffer that the task being created will use as its stack. Note this
is an array of StackType_t variables. The size of StackType_t is
dependent on the RTOS port. */
StackType_t stats_stack[ STATS_STACK_SIZE ];

