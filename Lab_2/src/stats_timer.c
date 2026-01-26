#include <stdint.h>
#include "AXI_timer.h"

static volatile uint32_t g_stats_counter = 0;
static int g_stats_timer_id = -1;

static void stats_timer_handler(void)
{
    g_stats_counter++;
}

void setup_stats_timer(void)
{
    g_stats_timer_id = AXI_TIMER_allocate();
    if (g_stats_timer_id < 0) {
        return;
    }

    g_stats_counter = 0;

    AXI_TIMER_set_handler((unsigned)g_stats_timer_id, stats_timer_handler);

    int count = (int)AXI_TIMER_US_TO_COUNT(1000);   // 1000 us = 1 ms

    AXI_TIMER_enable_interrupt((unsigned)g_stats_timer_id);
    AXI_TIMER_set_repeating((unsigned)g_stats_timer_id, count);
    AXI_TIMER_enable((unsigned)g_stats_timer_id);
}

uint32_t get_stats_counter(void)
{
    return g_stats_counter;
}
