#include <stdint.h>

volatile uint32_t hardfault_r0;
volatile uint32_t hardfault_r1;
volatile uint32_t hardfault_r2;
volatile uint32_t hardfault_r3;
volatile uint32_t hardfault_r12;
volatile uint32_t hardfault_lr;
volatile uint32_t hardfault_pc;
volatile uint32_t hardfault_psr;

volatile uint32_t hardfault_cfsr;
volatile uint32_t hardfault_hfsr;
volatile uint32_t hardfault_dfsr;
volatile uint32_t hardfault_afsr;
volatile uint32_t hardfault_bfar;
volatile uint32_t hardfault_mmfar;

void HardFault_Handler_C(uint32_t *stacked_regs)
{
    hardfault_r0  = stacked_regs[0];
    hardfault_r1  = stacked_regs[1];
    hardfault_r2  = stacked_regs[2];
    hardfault_r3  = stacked_regs[3];
    hardfault_r12 = stacked_regs[4];
    hardfault_lr  = stacked_regs[5];
    hardfault_pc  = stacked_regs[6];
    hardfault_psr = stacked_regs[7];

    hardfault_cfsr  = *((volatile uint32_t *)0xE000ED28);
    hardfault_hfsr  = *((volatile uint32_t *)0xE000ED2C);
    hardfault_dfsr  = *((volatile uint32_t *)0xE000ED30);
    hardfault_afsr  = *((volatile uint32_t *)0xE000ED3C);
    hardfault_mmfar = *((volatile uint32_t *)0xE000ED34);
    hardfault_bfar  = *((volatile uint32_t *)0xE000ED38);

    while (1) {
        __asm volatile ("nop");
    }
}