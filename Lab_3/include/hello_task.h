#ifndef HELLO_TASK_H
#define HELLO_TASK_H

#include "FreeRTOS.h"
#include "task.h"

#define STACK_SIZE 256
#define TASK_PRIORITY 4

extern StaticTask_t hello_TCB;
extern StackType_t hello_stack[ STACK_SIZE ];

void hello_task(void *pvParameters);

#endif // HELLO_TASK_H