#ifndef PM_TEST_TASK_H
#define PM_TEST_TASK_H

#include <FreeRTOSConfig.h>
#include <FreeRTOS.h>

void PM_test_task(void *pvParameters);

/* Dimensions the buffer that the task being created will use as its
stack. NOTE: This is the number of words the stack will hold, not the
number of bytes. For example, if each stack item is 32-bits, and this
is set to 100, then 400 bytes (100 * 32-bits) will be allocated. */
#define PM_TEST_STACK_SIZE 8192

/* Structure that will hold the TCB of the task being created. */
extern StaticTask_t PM_test_TCB;

/* Buffer that the task being created will use as its stack. Note this
is an array of StackType_t variables. The size of StackType_t is
dependent on the RTOS port. */
extern StackType_t PM_test_stack[ PM_TEST_STACK_SIZE ];

#endif
