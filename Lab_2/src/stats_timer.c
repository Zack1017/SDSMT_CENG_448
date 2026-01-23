#include <stdint.h>

#define DEMCR           (*(volatile uint32_t*)0xE000EDFC)
#define DWT_CTRL        (*(volatile uint32_t*)0xE0001000)
#define DWT_CYCCNT     (*(volatile uint32_t*)0xE0001004)
#define DEMCR_TRCENA_Msk    (1UL << 24)
#define DWT_CTRL_CYCCNTENA_Msk (1UL << 0)

void setup_stats_timer(void)
{
    DEMCR |= DEMCR_TRCENA_Msk;
    
    DWT_CYCCNT =0; 
    DWT_CTRL |= ~DWT_CTRL_CYCCNTENA_Msk; // Disable the cycle counter
}

uint32_t get_stats_counter(void)
{
    return DWT_CYCCNT;
}