/*
 * freertos_irq_wrappers.c
 *
 * Strong wrappers that map the startup vector handler names
 * to the FreeRTOS Cortex-M port handler names.
 */

void vPortSVCHandler(void);
void xPortPendSVHandler(void);
void xPortSysTickHandler(void);

void __attribute__((used, externally_visible)) SVC_Handler(void)
{
    vPortSVCHandler();
}

void __attribute__((used, externally_visible)) PendSV_Handler(void)
{
    xPortPendSVHandler();
}

void __attribute__((used, externally_visible)) SysTick_Handler(void)
{
    xPortSysTickHandler();
}