#include <FreeRTOS.h>
#include <task.h>
#include <device_addrs.h>
#include "AXI_timer.h"

#define TCSR0_OFFSET   0x00u
#define TCSR1_OFFSET   0x10u
#define TCSR_ENIT      (1u << 6)   // Enable interrupt

static void (*timer_handlers[4])(void) = {0};

void AXI_TIMER_set_handler(unsigned int timer, void (*handler)())
{
    if (timer < 4) 
    {
        timer_handlers[timer] = handler;
    }
}


void AXI_TIMER_0_ISR(void)
{

    if (timer_handlers[0]) 
    {
        timer_handlers[0]();
    }
    if (timer_handlers[1]) 
    {
        timer_handlers[1]();
    }
}

void AXI_TIMER_1_ISR(void)
{

    if (timer_handlers[2]) 
    {
        timer_handlers[2]();
    }
    if (timer_handlers[3]) 
    {
        timer_handlers[3]();
    }
}