#include <FreeRTOS.h>
#include <task.h>
#include <UART_16550.h>
#include <device_addrs.h>

// "screen /dev/ttyUSB1 9600"

#define ECHO_STACK_SIZE 256

static StackType_t echo_stack[ECHO_STACK_SIZE];
static StaticTask_t echo_TCB;

static void echo_task(void *pvParameters);

int main(void)
{
  TaskHandle_t echo_handle = NULL;

  NVIC_SetPriority(UART0_IRQ, 0x6); // priority for UART
  NVIC_SetPriority(UART1_IRQ, 0x6); // priority for UART

  // Initialize all UARTS
  UART_16550_init();

  // Configure UART0 and UART1 for 9600/N/8/2
  UART_16550_configure(UART0, 9600, UART_PARITY_NONE, 8, 2);
  UART_16550_configure(UART1, 9600, UART_PARITY_NONE, 8, 2);

  /* Part 5: Disable hello_task and stats_task.
     Instead, create an echo task. */
  echo_handle = xTaskCreateStatic(
      echo_task,
      "echo",
      ECHO_STACK_SIZE,
      NULL,
      3,
      echo_stack,
      &echo_TCB);

  (void)echo_handle;

  /* start the scheduler */
  vTaskStartScheduler();

  /* should never reach here */
  while (1)
    ;
}

/* Echo task: blocks until a char is received, then writes it back */
static void echo_task(void *pvParameters)
{
  (void)pvParameters;

  // Optional startup banner (safe even if you haven't finished TX IRQ yet,
  // but it may block depending on your implementation)
  UART_16550_write_string(UART0, "\r\nEcho task ready (UART0)\r\n", portMAX_DELAY);

  for (;;)
  {
    char ch;

    // Block until one character is received from ISR->RX stream buffer
    if (UART_16550_get_char(UART0, &ch, portMAX_DELAY) == pdPASS)
    {
      // Echo it back
      UART_16550_put_char(UART0, ch, portMAX_DELAY);

      // Optional: if you want "enter" to produce CRLF nicely in terminals
      // uncomment this:
      /*
      if (ch == '\r')
        UART_16550_put_char(UART0, '\n', portMAX_DELAY);
      */
    }
  }
}

/* ---- Idle task static allocation boilerplate (unchanged) ---- */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
  static StaticTask_t xIdleTaskTCB;
  static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vAssertCalled(unsigned line, const char *const filename)
{
  unsigned uSetToNonZeroInDebuggerToContinue = 0;
  taskENTER_CRITICAL();
  {
    while (uSetToNonZeroInDebuggerToContinue == 0)
    {
    }
  }
  taskEXIT_CRITICAL();
}