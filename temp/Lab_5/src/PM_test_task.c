#include <PM_test_task.h>
#include <LDP-001_PM_driver.h>
#include <curses.h>
#include <task.h>
#include <math.h>
#include <sincos.h>

// For 10 bits of depth, we can get a frequency of
// 150 MHz/1<<10 146484.375 Hz
// 

// For 11 bits of depth, we can get a frequency of 73242.1875

// For 12 bits of depth, we can get a frequency of 36621.09375

// For 13 bits of depth, we can get a frequency of 18310.546875

// For 14 bits of depth, we can get a frequency of 9155.2734375

/* Structure that will hold the TCB of the task being created. */
StaticTask_t PM_test_TCB;

/* Buffer that the task being created will use as its stack. Note this
is an array of StackType_t variables. The size of StackType_t is
dependent on the RTOS port. */
StackType_t PM_test_stack[ PM_TEST_STACK_SIZE ];


#define CHANNEL 0

//#define FREQ 146484
//#define DEPTH 10

//#define FREQ 73242
//#define DEPTH 11

//#define FREQ 36621
//#define DEPTH 12

//#define FREQ 18310
//#define DEPTH 13

#define FREQ 9155
#define DEPTH 14

static uint16_t data[1<<DEPTH];
int nsamples;

void fill_data()
{
  int x;
  int y;
  uint32_t yu;
  int SPS;  // Samples per second
  double xd,yd;
  nsamples = round((double)FREQ/440);
  for(x=0;x<nsamples;x++)
    {

      xd = ((double)x)* 2 * M_PI / nsamples;
      yd = sin(xd);
      yd += 1.0;
      y = yd * (1<<(DEPTH-1));
      data[x] = y;
    }
}


void handler()
{
  static int x = 0;
  unsigned y;
  int loops=0;
  while(!PM_FIFO_full(CHANNEL))
    {
      loops++;
      y = data[x++];
      PM_set_duty(CHANNEL,y);
      x %= (nsamples);
    }
}

void handler2()
{
  static int x = 0;
  unsigned y;
  int loops=0;
  while(!PM_FIFO_full(CHANNEL+2))
    {
      loops++;
      y = data[x++];
      PM_set_duty(CHANNEL+2,y);
      x %= (nsamples);
    }
}



// Part 4
void PM_test_task(void *params)
{
  
  int DC = 0;
  int x = 0;

  fill_data();
  
  PM_acquire(CHANNEL);
  PM_set_cycle_time(CHANNEL,1<<DEPTH,FREQ);
  PM_set_PDM_mode(CHANNEL);
  PM_enable_FIFO(CHANNEL);
  PM_enable(CHANNEL);
  PM_set_handler(CHANNEL,handler);

  PM_acquire(CHANNEL+2);
  PM_set_cycle_time(CHANNEL+2,1<<DEPTH,FREQ);
  PM_set_PWM_mode(CHANNEL+2);
  PM_enable_FIFO(CHANNEL+2);
  PM_enable(CHANNEL+2);
  PM_set_handler(CHANNEL+2,handler2);

  /* PM_acquire(CHANNEL+1); */
  /* PM_set_cycle_time(CHANNEL+1,1000,100); */
  /* PM_set_PWM_mode(CHANNEL+1); */
  /* PM_disable_FIFO(CHANNEL+1); */
  /* PM_set_duty(CHANNEL+1,250); */
  /* PM_enable(CHANNEL+1); */


  while(1)
    {
      // PM_set_duty(CHANNEL+1,500);
      vTaskDelay(5);
    }
  
}


/*
// Parts 1, 2, and 3

#define FREQ 100

void PM_test_task(void *params)
{
  
  int DC = 0;
  
  PM_acquire(CHANNEL);
  PM_set_cycle_time(CHANNEL,1000,FREQ);
  PM_set_PDM_mode(CHANNEL);
  PM_disable_FIFO(CHANNEL);
  PM_enable(CHANNEL);

  PM_acquire(CHANNEL+1);
  PM_set_cycle_time(CHANNEL+1,1000,FREQ);
  PM_set_PDM_mode(CHANNEL+1);
  PM_disable_FIFO(CHANNEL+1);
  PM_enable(CHANNEL+1);

  PM_acquire(CHANNEL+2);
  PM_set_cycle_time(CHANNEL+2,1000,FREQ);
  PM_set_PWM_mode(CHANNEL+2);
  PM_disable_FIFO(CHANNEL+2);
  PM_enable(CHANNEL+2);

  while(1)
    {
      PM_set_duty(CHANNEL,DC);
      PM_set_duty(CHANNEL+1,DC);
      PM_set_duty(CHANNEL+2,DC);
      if((DC = DC + 250)>1000)
	DC=0;
      vTaskDelay(5000);
    }
  
}

*/
