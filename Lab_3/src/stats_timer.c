#include <stdint.h>
#include "AXI_timer.h"
#include <FreeRTOS.h>

 
static volatile uint32_t g_stats_counter = 0;
static int g_stats_timer_id = -1;

static void stats_timer_handler(void)
{
    g_stats_counter++;
}

void setup_stats_timer(void)
{
    g_stats_counter = 0;

    if (g_stats_timer_id >= 0) 
    {
        return;

    }

    g_stats_timer_id = AXI_TIMER_allocate();
    if (g_stats_timer_id < 0) 
    {
        return; //oh no 
    }

    AXI_TIMER_set_handler((unsigned)g_stats_timer_id, stats_timer_handler);

    const uint32_t ticks_per_us = (uint32_t)(configTICK_RATE_HZ / 1000000u);
    const uint32_t ticks_1ms    = ticks_per_us * 1000u;

    if (ticks_1ms == 0u) 
    {
        AXI_TIMER_free((unsigned)g_stats_timer_id);
        g_stats_timer_id = -1;
        return;
    }

    AXI_TIMER_set_repeating((unsigned)g_stats_timer_id, (int)ticks_1ms);
}

int get_stats_counter(void)
{
    return (int)g_stats_counter;
}