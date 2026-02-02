#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>

#include <device_addrs.h>
#include <AXI_timer.h>
#include "core_cm3.h" 

//define numbs 
#ifndef TIMER0_IRQ_NUM
#define TIMER0_IRQ_NUM 0   
#endif
#ifndef TIMER1_IRQ_NUM
#define TIMER1_IRQ_NUM 0  
#endif


//********** Bitfield structure for TCSR register ************/
struct TCSRbits 
{
  volatile unsigned MDT:1;   // Mode: 0 = generate mode
  volatile unsigned UDT:1;   // Up/Down: 1 = count down
  volatile unsigned GENT:1;  // External generate
  volatile unsigned CAPT:1;  // External capture
  volatile unsigned ARHT:1;  // Auto reload (1 = repeating)
  volatile unsigned LOAD:1;  // Load TLR into counter when toggled
  volatile unsigned ENIT:1;  // Interrupt enable
  volatile unsigned ENT:1;   // Timer enable (run)
  volatile unsigned TINT:1;  // Interrupt occurred (write 1 to clear)
  volatile unsigned PWMA:1;  // PWM enable (not used)
  volatile unsigned ENALL:1; // Enable all timers (not used)
  volatile unsigned CASC:1;  // Cascade (not used)
};

//********** Device structures ************/
typedef union 
{
  volatile struct TCSRbits bits;
  volatile uint32_t TCSR;
} AXI_timer_TCSR_t;

typedef struct 
{
  volatile AXI_timer_TCSR_t TCSR; // Control/status
  volatile uint32_t TLR;          // Load register
  volatile uint32_t TCR;          // Counter register
} AXI_timer_t;

typedef struct 
{
  TaskHandle_t owner[2];
  void (*handler[2])();
  volatile AXI_timer_t *timer[2];
  int NVIC_IRQ_NUM;
} AXI_timer_device_t;



//********** Module variables ************/
static volatile AXI_timer_device_t timer_device[NUM_AXI_TIMERS/2] = 
{
  { {NULL, NULL}, {NULL, NULL}, { (AXI_timer_t*)TIMER0, (AXI_timer_t*)(TIMER0 + 0x10) }, TIMER0_IRQ_NUM },
  { {NULL, NULL}, {NULL, NULL}, { (AXI_timer_t*)TIMER1, (AXI_timer_t*)(TIMER1 + 0x10) }, TIMER1_IRQ_NUM }
};



//*******************Helpers********************** */
static inline int timer_valid(unsigned int timer) 
{
  return (timer < NUM_AXI_TIMERS);
}



static inline int timer_dev(unsigned int timer) 
{
  return (int)(timer >> 1);  // /2
}



static inline int timer_ch(unsigned int timer) 
{
  return (int)(timer & 1);   // %2
}



static inline void clear_timer_interrupt(volatile AXI_timer_t *t) 
{
  t->TCSR.bits.TINT = 1; // Clear interrupt
}



static inline void base_config(volatile AXI_timer_t *t) 
{
  t->TCSR.bits.MDT  = 0; // generate mode
  t->TCSR.bits.UDT  = 1; // count down
  t->TCSR.bits.CASC = 0; // no cascade
  t->TCSR.bits.PWMA = 0; // no pwm
  t->TCSR.bits.ENALL = 0;
  t->TCSR.bits.GENT = 0;
  t->TCSR.bits.CAPT = 0;
}



/* ---- AXI Timer API Implementation ---- */
void AXI_timer_handler(volatile AXI_timer_device_t *device)
{
  for (int ch = 0; ch < 2; ch++) 
  {
    volatile AXI_timer_t *t = device->timer[ch];

    if (t->TCSR.bits.TINT) 
    {
      // Call the per-channel handl 
      if (device->handler[ch] != NULL) 
      {
        device->handler[ch]();
      }

      // auto-reload
      if (t->TCSR.bits.ARHT == 0) 
      {
        t->TCSR.bits.ENT  = 0;  // stop
        t->TCSR.bits.ENIT = 0;  // disable interrupt
      }

      clear_timer_interrupt(t);
    }
  }
  NVIC_ClearPendingIRQ((IRQn_Type)device->NVIC_IRQ_NUM); 
}



void AXI_TIMER_0_ISR()
{
  volatile AXI_timer_device_t *dev = &(timer_device[0]);
  AXI_timer_handler(dev);
}



void AXI_TIMER_1_ISR()
{
  volatile AXI_timer_device_t *dev = &(timer_device[1]);
  AXI_timer_handler(dev);
}



int AXI_TIMER_allocate()
{
  TaskHandle_t me = xTaskGetCurrentTaskHandle();

  taskENTER_CRITICAL();
  for (unsigned int t = 0; t < NUM_AXI_TIMERS; t++) 
  {
    int dev = timer_dev(t);
    int ch  = timer_ch(t);

    if (timer_device[dev].owner[ch] == NULL) 
    {
      timer_device[dev].owner[ch] = me;
      timer_device[dev].handler[ch] = NULL;

      //Stops the tings 
      timer_device[dev].timer[ch]->TCSR.bits.ENT = 0;
      timer_device[dev].timer[ch]->TCSR.bits.ENIT = 0;
      clear_timer_interrupt(timer_device[dev].timer[ch]);

      taskEXIT_CRITICAL();
      return (int)t;
    }
  }
  taskEXIT_CRITICAL();
  return -1;
}



void AXI_TIMER_free(unsigned int timer)
{
  if (!timer_valid(timer)) 
  {
    return;
  }

  int dev = timer_dev(timer);
  int ch  = timer_ch(timer);

  taskENTER_CRITICAL();

  timer_device[dev].timer[ch]->TCSR.bits.ENT  = 0;
  timer_device[dev].timer[ch]->TCSR.bits.ENIT = 0;
  clear_timer_interrupt(timer_device[dev].timer[ch]);

  timer_device[dev].handler[ch] = NULL;
  timer_device[dev].owner[ch]   = NULL;

  taskEXIT_CRITICAL();
}



void AXI_TIMER_set_handler(unsigned int timer, void (*handler)())
{
  if (!timer_valid(timer)) 
  {
    return;
  }
  int dev = timer_dev(timer);
  int ch  = timer_ch(timer);

  taskENTER_CRITICAL();
  timer_device[dev].handler[ch] = handler;
  taskEXIT_CRITICAL();
}



void AXI_TIMER_enable(unsigned int timer)
{
  if (!timer_valid(timer)) return;

  int dev = timer_dev(timer);
  int ch  = timer_ch(timer);

  volatile AXI_timer_t *t = timer_device[dev].timer[ch];

  t->TCSR.bits.LOAD = 0;

  t->TCSR.bits.ENT = 1; //Run
}



void AXI_TIMER_disable(unsigned int timer, int remove_handler)
{
  if (!timer_valid(timer)) 
  {
    return;
  }

  int dev = timer_dev(timer);
  int ch  = timer_ch(timer);

  volatile AXI_timer_t *t = timer_device[dev].timer[ch];

  t->TCSR.bits.ENT = 0; //Stop    
  t->TCSR.bits.LOAD = 0;

  t->TCSR.bits.ENIT = 0; //turn off 
  clear_timer_interrupt(t);

  if (remove_handler) 
  {
    timer_device[dev].handler[ch] = NULL;
  }

  if (timer_device[dev].timer[0]->TCSR.bits.ENIT == 0 && timer_device[dev].timer[1]->TCSR.bits.ENIT == 0) 
      {
    NVIC_DisableIRQ((IRQn_Type)timer_device[dev].NVIC_IRQ_NUM);
    NVIC_ClearPendingIRQ((IRQn_Type)timer_device[dev].NVIC_IRQ_NUM);
  }
}



void AXI_TIMER_enable_interrupt(unsigned int timer)
{
  if (!timer_valid(timer)) 
  {
    return;
  } 

  int dev = timer_dev(timer);
  int ch  = timer_ch(timer);

  volatile AXI_timer_t *t = timer_device[dev].timer[ch];
  t->TCSR.bits.ENIT = 1;

  NVIC_EnableIRQ((IRQn_Type)timer_device[dev].NVIC_IRQ_NUM);
}



void AXI_TIMER_disable_interrupt(unsigned int timer)
{
  if (!timer_valid(timer)) 
  {
    return;
  }
  int dev = timer_dev(timer);
  int ch  = timer_ch(timer);

  volatile AXI_timer_t *t = timer_device[dev].timer[ch];
  t->TCSR.bits.ENIT = 0;
  clear_timer_interrupt(t);

  if (timer_device[dev].timer[0]->TCSR.bits.ENIT == 0 && timer_device[dev].timer[1]->TCSR.bits.ENIT == 0) 
  {
    NVIC_DisableIRQ((IRQn_Type)timer_device[dev].NVIC_IRQ_NUM);
    NVIC_ClearPendingIRQ((IRQn_Type)timer_device[dev].NVIC_IRQ_NUM);
  }
}



void AXI_TIMER_set_repeating(unsigned int timer, int count)
{
  if (!timer_valid(timer)) 
  {
    return;
  } 
  int dev = timer_dev(timer);
  int ch  = timer_ch(timer);

  volatile AXI_timer_t *t = timer_device[dev].timer[ch];

  t->TCSR.bits.ENT = 0; //stop
  t->TCSR.bits.ENIT = 0;

  base_config(t);

  t->TCSR.bits.ARHT = 1; //auto-reload

  t->TLR = (uint32_t)count; //load

  t->TCSR.bits.LOAD = 1; //load into counter
  t->TCSR.bits.LOAD = 0;

  clear_timer_interrupt(t);

  t->TCSR.bits.ENIT = 1; //interuopt
  t->TCSR.bits.ENT  = 1; //start

  NVIC_EnableIRQ((IRQn_Type)timer_device[dev].NVIC_IRQ_NUM);
}



void AXI_TIMER_set_oneshot(unsigned int timer, int count)
{
  if (!timer_valid(timer)) 
  {
    return;
  }

  int dev = timer_dev(timer);
  int ch  = timer_ch(timer);

  volatile AXI_timer_t *t = timer_device[dev].timer[ch];

  t->TCSR.bits.ENT = 0; //stop 
  t->TCSR.bits.ENIT = 0;

  base_config(t);

  t->TCSR.bits.ARHT = 0; //one 

  t->TLR = (uint32_t)count; //load

  t->TCSR.bits.LOAD = 1; //count load 
  t->TCSR.bits.LOAD = 0;

  clear_timer_interrupt(t);

  t->TCSR.bits.ENIT = 1; //interuopt 
  t->TCSR.bits.ENT  = 1; //start 

  NVIC_EnableIRQ((IRQn_Type)timer_device[dev].NVIC_IRQ_NUM);
}