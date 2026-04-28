
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

// "screen /dev/ttyUSB1 9600"


int main( void )
{
  TaskHandle_t hello_handle = NULL;
  TaskHandle_t stats_handle = NULL;
  TaskHandle_t firework_handle = NULL;
  TaskHandle_t testcurs_handle = NULL;
  TaskHandle_t invaders_handle = NULL;
  TaskHandle_t PM_test_handle = NULL;

  NVIC_SetPriority(UART0_IRQ,0x6); // priority for UART
  NVIC_SetPriority(UART1_IRQ,0x5); // priority for UART

  NVIC_SetPriority(PM_IRQ,0x4);
  NVIC_EnableIRQ(PM_IRQ);

  // Intitialize all UARTS
  UART_16550_init();

  // Initialize the Pulse Modulator
  PM_init();

  // Configure UARTs
  UART_16550_configure(UART0,57600,UART_PARITY_NONE,8,1);
  UART_16550_configure(UART1,57600,UART_PARITY_NONE,8,1);


  /* Create the task without using any dynamic memory allocation. */
  //firework_handle = xTaskCreateStatic(firework_task,"firework",
  //				      FIREWORK_STACK_SIZE,
  // 				      NULL,2,firework_stack,&firework_TCB);
			      
  /* Create the task without using any dynamic memory allocation. */
  //testcurs_handle = xTaskCreateStatic(testcurs_task,"testcurs",
  //				      TESTCURS_STACK_SIZE,
  //				      NULL,3,testcurs_stack,&testcurs_TCB);

  /* Create the task without using any dynamic memory allocation. */
  //invaders_handle = xTaskCreateStatic(ninvaders,"ninvaders",
  //				      NINVADERS_STACK_SIZE,
  //				      NULL,2,ninvaders_stack,&ninvaders_TCB);
			      
  /* Create the task without using any dynamic memory allocation. */
  PM_test_handle = xTaskCreateStatic(PM_test_task,"PM_test",
  				      PM_TEST_STACK_SIZE,
  				      NULL,2,PM_test_stack,&PM_test_TCB);
			      
  /* Create the task without using any dynamic memory allocation. */
  hello_handle = xTaskCreateStatic(hello_task,"hello",HELLO_STACK_SIZE,
  				   NULL,4,hello_stack,&hello_TCB);
			      
  /* Create the task without using any dynamic memory allocation. */
  stats_handle = xTaskCreateStatic(stats_task,"stats",STATS_STACK_SIZE,
				   NULL,3,stats_stack,&stats_TCB);
			      
  /* start the scheduler */
  vTaskStartScheduler();

  /* we should never get to this point, but if we do, go into infinite
     loop */
  while(1);
}



/* Blatantly stolen from
   https://www.freertos.org/a00110.html#include_parameters
   and I really don't understand it yet.
*/

/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application
must provide an implementation of vApplicationGetIdleTaskMemory() to
provide the memory that is used by the Idle task. */
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
/*-----------------------------------------------------------*/



/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application
must provide an implementation of vApplicationGetTimerTaskMemory() to
provide the memory that is used by the Timer task. */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimeTaskTCBBuffer,
                                    StackType_t **ppxTimeTaskStackBuffer,
                                    uint32_t *pulTimeTaskStackSize )
{
/* If the buffers to be provided to the Time task are declared inside
this function then they must be declared static - otherwise they will
be allocated on the stack and so not exists after this function
exits. */
static StaticTask_t xTimeTaskTCB;
static StackType_t uxTimeTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the
    Time task's state will be stored. */
    *ppxTimeTaskTCBBuffer = &xTimeTaskTCB;

    /* Pass out the array that will be used as the Time task's stack. */
    *ppxTimeTaskStackBuffer = uxTimeTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimeTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimeTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
/*-----------------------------------------------------------*/

void vAssertCalled( unsigned line, const char * const filename )
{
  unsigned uSetToNonZeroInDebuggerToContinue=0;
    taskENTER_CRITICAL();
    {
        /* You can step out of this function to debug the assertion by using
        the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
        value. */
        while(uSetToNonZeroInDebuggerToContinue == 0)
        {
        }
    }
    taskEXIT_CRITICAL();
}

/*-----------------------------------------------------------*/


void malloc_failed()
{
  unsigned uSetToNonZeroInDebuggerToContinue=0;
    taskENTER_CRITICAL();
    {
        /* You can step out of this function to debug the assertion by using
        the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
        value. */
        while(uSetToNonZeroInDebuggerToContinue == 0)
        {
        }
    }
    taskEXIT_CRITICAL();
}



/*-----------------------------------------------------------*/


void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName )
{
  unsigned uSetToNonZeroInDebuggerToContinue=0;
    taskENTER_CRITICAL();
    {
        /* You can step out of this function to debug the assertion by using
        the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
        value. */
        while(uSetToNonZeroInDebuggerToContinue == 0)
        {
        }
    }
    taskEXIT_CRITICAL();
}



