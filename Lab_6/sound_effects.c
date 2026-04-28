#include <sounds.h>
#include <sound_effects.h>
#include <stddef.h>
#include <stdint.h>
#include <queue.h>
#include <task.h>
#include <LDP-001_PM_driver.h>

#define CHANNEL 0

EventGroupHandle_t effect_events;

// Each sound effect task will send audio buffers (actually just
// pointers) to the mixer using a dedicated queue
static QueueHandle_t effect_to_mixer_queues[NUM_EFFECTS];

// The mixer will send buffer pointers to the ISR.
static QueueHandle_t MixerToISRqueue;
// The Interrupt Handler will return the buffer pointers to the mixer
// after transferring the data to the PM device.
static QueueHandle_t ISRToMixerqueue;

typedef struct{
  effect_buffer *buffers;
  int num_buffers;
  EventBits_t event;
  QueueHandle_t sendqueue;
}effect_param_t;

static effect_param_t effect_task_params[NUM_EFFECTS] = {
  {explosion1,NUM_explosion1_BUFFERS,EXPLOSION1_EVENT,NULL},
  {fastinvader1,NUM_fastinvader1_BUFFERS,FASTINVADER1_EVENT,NULL},
  {fastinvader2,NUM_fastinvader2_BUFFERS,FASTINVADER2_EVENT,NULL},
  {fastinvader3,NUM_fastinvader3_BUFFERS,FASTINVADER3_EVENT,NULL},
  {fastinvader4,NUM_fastinvader4_BUFFERS,FASTINVADER4_EVENT,NULL},
  {invaderkilled,NUM_invaderkilled_BUFFERS,INVADERKILLED_EVENT,NULL},
  {shoot,NUM_shoot_BUFFERS,SHOOT_EVENT,NULL},
  {ufo_highpitch,NUM_ufo_highpitch_BUFFERS,UFO_HIGHPITCH_EVENT,NULL},
  {ufo_lowpitch,NUM_ufo_lowpitch_BUFFERS,UFO_LOWPITCH_EVENT,NULL}
};

void audio_handler(BaseType_t *HPTW)
{
  static uint16_t *buffer = NULL;
  static int buffer_index = 0;

  if(buffer == NULL)
    {
      xQueueReceiveFromISR(MixerToISRqueue,&buffer,HPTW);
      buffer_index = 0;
    }

  while(!PM_FIFO_full(CHANNEL))
    {
      if(buffer == NULL)
        {
          PM_set_duty(CHANNEL,0);
        }
      else
        {
          PM_set_duty(CHANNEL,buffer[buffer_index]);
          buffer_index++;

          if(buffer_index >= EFFECT_BUFFER_SIZE)
            {
              xQueueSendFromISR(ISRToMixerqueue,&buffer,HPTW);
              xQueueReceiveFromISR(MixerToISRqueue,&buffer,HPTW);
              buffer_index = 0;
            }
        }
    }
}

#define DEPTH 10
#define FREQ  8000
#define NUM_MIXER_BUFFERS 4
static uint16_t mixer_buffers[NUM_MIXER_BUFFERS][EFFECT_BUFFER_SIZE];

static void effect_mixer_task(void *params)
{
  (void)params;
  uint16_t *buffer;
  effect_buffer *effect_data[NUM_EFFECTS];
  int i, j;

  for(i=0;i<NUM_MIXER_BUFFERS;i++)
    {
      buffer = mixer_buffers[i];
      xQueueSend(ISRToMixerqueue,&buffer,portMAX_DELAY);
    }

  PM_acquire(CHANNEL);
  PM_set_handler(CHANNEL,(void(*)(void))audio_handler);
  PM_set_cycle_time(CHANNEL,(1<<DEPTH),FREQ);
  PM_set_PDM_mode(CHANNEL);
  PM_enable_FIFO(CHANNEL);
  PM_enable(CHANNEL);

  while (1)
    {
      xQueueReceive(ISRToMixerqueue,&buffer,portMAX_DELAY);

      for(i=0;i<NUM_EFFECTS;i++)
        xQueueReceive(effect_to_mixer_queues[i],&effect_data[i],0);

      for(j=0;j<EFFECT_BUFFER_SIZE;j++)
        {
          int mix = 0;
          for(i=0;i<NUM_EFFECTS;i++)
            if(effect_data[i] != NULL)
              mix += effect_data[i]->data[j];

          int sample = mix + (1<<(DEPTH-1));
          if(sample < 0) sample = 0;
          if(sample > ((1<<DEPTH)-1)) sample = ((1<<DEPTH)-1);
          buffer[j] = (uint16_t)sample;
        }

      xQueueSend(MixerToISRqueue,&buffer,portMAX_DELAY);
    }
}

static void effect_task(void *params)
{
  effect_param_t *my_effect = (effect_param_t*)params;

  while(1)
    {
      xEventGroupWaitBits(effect_events,
                          my_effect->event,
                          pdTRUE,
                          pdFALSE,
                          portMAX_DELAY);

      for(int i=0;i<my_effect->num_buffers;i++)
        {
          effect_buffer *next = &my_effect->buffers[i];
          xQueueSend(my_effect->sendqueue,&next,portMAX_DELAY);
        }
    }
}

#define MIXER_STACK_SIZE 512
static TaskHandle_t mixer_task_handle;
static StackType_t  mixer_stack[MIXER_STACK_SIZE];
static StaticTask_t mixer_TCB;

#define EFFECT_STACK_SIZE 256
static TaskHandle_t effect_task_handles[NUM_EFFECTS];
static StackType_t effect_stacks[NUM_EFFECTS][EFFECT_STACK_SIZE];
static StaticTask_t effect_TCBs[NUM_EFFECTS];

static StaticQueue_t MixerToISRqueue_QCB, ISRToMixerqueue_QCB;
static uint16_t *MixerToISRqueue_buf[NUM_MIXER_BUFFERS];
static uint16_t *ISRToMixerqueue_buf[NUM_MIXER_BUFFERS];

static StaticQueue_t effect_to_mixer_QCBs[NUM_EFFECTS];
#define EFFECT_TO_MIXER_QUEUE_LEN 6
static effect_buffer *effect_to_mixer_queue_buf[NUM_EFFECTS][EFFECT_TO_MIXER_QUEUE_LEN];

static StaticEventGroup_t effect_events_storage;

void effect_init()
{
  int i;

  effect_events = xEventGroupCreateStatic(&effect_events_storage);

  for(i=0;i<NUM_EFFECTS;i++)
    {
      effect_to_mixer_queues[i] =
        xQueueCreateStatic(EFFECT_TO_MIXER_QUEUE_LEN,
                           sizeof(effect_buffer *),
                           (uint8_t*)effect_to_mixer_queue_buf[i],
                           &effect_to_mixer_QCBs[i]);
      effect_task_params[i].sendqueue = effect_to_mixer_queues[i];
    }

  MixerToISRqueue = xQueueCreateStatic(NUM_MIXER_BUFFERS,
                                       sizeof(uint16_t *),
                                       (uint8_t*)MixerToISRqueue_buf,
                                       &MixerToISRqueue_QCB);

  ISRToMixerqueue = xQueueCreateStatic(NUM_MIXER_BUFFERS,
                                       sizeof(uint16_t *),
                                       (uint8_t*)ISRToMixerqueue_buf,
                                       &ISRToMixerqueue_QCB);

  for(i=0;i<NUM_EFFECTS;i++)
    {
      effect_task_handles[i] = xTaskCreateStatic(effect_task,
                                                 "effect",
                                                 EFFECT_STACK_SIZE,
                                                 &effect_task_params[i],
                                                 2,
                                                 effect_stacks[i],
                                                 &effect_TCBs[i]);
    }

  mixer_task_handle = xTaskCreateStatic(effect_mixer_task,
                                        "mixer",
                                        MIXER_STACK_SIZE,
                                        NULL,
                                        3,
                                        mixer_stack,
                                        &mixer_TCB);
  (void)mixer_task_handle;
}
