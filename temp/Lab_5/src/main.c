#include <FreeRTOS.h>
#include <task.h>

#include <UART_16550.h>
#include <hello_task.h>
#include <stats_task.h>
#include <PM_test_task.h>
#include <firework_task.h>
#include <testcurs_task.h>
#include <ninvaders.h>
#include <device_addrs.h>
#include <LDP-001_PM_driver.h>
#include <sound_effects.h>

// "screen /dev/ttyUSB1 57600"

int main(void)
{
  TaskHandle_t hello_handle = NULL;
  TaskHandle_t stats_handle = NULL;
  TaskHandle_t firework_handle = NULL;
  TaskHandle_t testcurs_handle = NULL;
  TaskHandle_t invaders_handle = NULL;
  TaskHandle_t PM_test_handle = NULL;

  /*
   * Interrupt priorities:
   * Lower numerical value = higher priority on Cortex-M.
   *
   * Keep PM interrupt above UART so audio timing is more stable.
   */
  NVIC_SetPriority(UART0_IRQ, 0x6);
  NVIC_SetPriority(UART1_IRQ, 0x5);

  NVIC_SetPriority(PM_IRQ, 0x6);
  NVIC_EnableIRQ(PM_IRQ);

  // Initialize all UARTs
  UART_16550_init();

  // Initialize the Pulse Modulator driver
  PM_init();

  // Configure UARTs
  UART_16550_configure(UART0, 57600, UART_PARITY_NONE, 8, 1);
  UART_16550_configure(UART1, 57600, UART_PARITY_NONE, 8, 1);

  /*
   * Initialize Lab 6 sound effects.
   *
   * This creates:
   * - event group
   * - effect queues
   * - mixer-to-ISR queue
   * - ISR-to-mixer queue
   * - effect tasks
   * - mixer task
   * - optional sound test task
   */
  effect_init();

  /*
   * Lab 6 sound debug mode:
   *
   * For now, leave nInvaders disabled and let the sound test task
   * trigger one sound every 2 seconds.
   *
   * Once sound is working, uncomment nInvaders and disable the
   * sound test task in sound_effects.c.
   */

  invaders_handle = xTaskCreateStatic(ninvaders,
                                      "ninvaders",
                                      NINVADERS_STACK_SIZE,
                                      NULL,
                                      2,
                                      ninvaders_stack,
                                      &ninvaders_TCB);


  /*
   * Do NOT enable PM_test_task while Lab 6 audio is running.
   * PM_test_task and the sound system both use the pulse modulator.
   */

  // PM_test_handle = xTaskCreateStatic(PM_test_task,
  //                                    "PM_test",
  //                                    PM_TEST_STACK_SIZE,
  //                                    NULL,
  //                                    2,
  //                                    PM_test_stack,
  //                                    &PM_test_TCB);


  /*
   * Optional terminal/status tasks.
   * Keep these if they are already working.
   */
  hello_handle = xTaskCreateStatic(hello_task,
                                   "hello",
                                   HELLO_STACK_SIZE,
                                   NULL,
                                   4,
                                   hello_stack,
                                   &hello_TCB);

  stats_handle = xTaskCreateStatic(stats_task,
                                   "stats",
                                   STATS_STACK_SIZE,
                                   NULL,
                                   3,
                                   stats_stack,
                                   &stats_TCB);

  configASSERT(hello_handle != NULL);
  configASSERT(stats_handle != NULL);
  configASSERT(invaders_handle != NULL);
  configASSERT(PM_test_handle != NULL);

  // Start the scheduler
  vTaskStartScheduler();

  while (1);
}

/*-----------------------------------------------------------*/

/*
 * configSUPPORT_STATIC_ALLOCATION is set to 1, so the application
 * must provide an implementation of vApplicationGetIdleTaskMemory()
 * to provide the memory that is used by the Idle task.
 */
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

/*-----------------------------------------------------------*/

/*
 * configSUPPORT_STATIC_ALLOCATION is set to 1, so the application
 * must provide an implementation of vApplicationGetTimerTaskMemory()
 * to provide the memory that is used by the Timer task.
 */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimeTaskTCBBuffer,
                                    StackType_t **ppxTimeTaskStackBuffer,
                                    uint32_t *pulTimeTaskStackSize)
{
  static StaticTask_t xTimeTaskTCB;
  static StackType_t uxTimeTaskStack[configTIMER_TASK_STACK_DEPTH];

  *ppxTimeTaskTCBBuffer = &xTimeTaskTCB;
  *ppxTimeTaskStackBuffer = uxTimeTaskStack;
  *pulTimeTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

/*-----------------------------------------------------------*/

void vAssertCalled(unsigned line, const char * const filename)
{
  unsigned uSetToNonZeroInDebuggerToContinue = 0;

  (void)line;
  (void)filename;

  taskENTER_CRITICAL();
  {
    while (uSetToNonZeroInDebuggerToContinue == 0)
    {
    }
  }
  taskEXIT_CRITICAL();
}

/*-----------------------------------------------------------*/

void malloc_failed(void)
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

/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
  unsigned uSetToNonZeroInDebuggerToContinue = 0;

  (void)pxTask;
  (void)pcTaskName;

  taskENTER_CRITICAL();
  {
    while (uSetToNonZeroInDebuggerToContinue == 0)
    {
    }
  }
  taskEXIT_CRITICAL();
}