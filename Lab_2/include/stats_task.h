#ifndef STATS_TASK_H
#define STATS_TASK_H

#include "FreeRTOS.h"
#include"task.h"

#define STATS_TASK_STACK_SIZE 512
#define STATS_TASK_PRIORITY 4

extern StaticTask_t stats_task_tcb;
extern StackType_t stats_task_stack[STATS_TASK_STACK_SIZE]

void stats_task(void *params);

#endif