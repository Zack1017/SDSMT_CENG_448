#ifndef FIREWORK_TASK_H
#define FIREWORK_TASK_H

#include <FreeRTOS.h>

void firework_task(void *pvParameters);

#define FIREWORK_STACK_SIZE 512

extern StaticTask_t firework_TCB;
extern StackType_t firework_stack[FIREWORK_STACK_SIZE];

#endif
