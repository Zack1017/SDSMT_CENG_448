#include <FreeRTOS.h>
#include <task.h>
#include <device_addrs.h>
#include "AXI_timer.h"

//Register Offsets 
#define TCSR0_OFFSET    0x00u
#define TLR0_OFFSET     0x04u
#define TCR0_OFFSET     0x08u

#define TCSR1_OFFSET    0x10u
#define TLR1_OFFSET     0x14u
#define TCR1_OFFSET     0x18u

//TCSR Bit definitions
#define TCSR_MDT    (1u << 0)   // mode down timer
#define TCSR_UDT    (1u << 1)   // up/down timer
#define TCSR_GENT   (1u << 2)   // generate external trigger
#define TCSR_CAPT   (1u << 3)   // capture mode
#define TCSR_ARHT   (1u << 4)   // auto reload/hold
#define TCSR_LOAD   (1u << 5)   // load timer
#define TCSR_ENIT   (1u << 6)   // enable interrupt
#define TCSR_ENT    (1u << 7)   // enable timer
#define TCSR_TINT   (1u << 8)   // timer interrupt status
#define TCSR_PWMA   (1u << 9)   // pulse width modulation enable
#define TCSR_EXTT   (1u << 10)  // external trigger type
#define TCSR_ENT1   (1u << 11)  // enable timer 1

typedef enum {
    TIMER_MODE_DISABLED = 0,
    TIME_MODE_REPEAT, 
    TIMER_MODE_ONESHOT
} AXI_TIMER_mode_t;

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