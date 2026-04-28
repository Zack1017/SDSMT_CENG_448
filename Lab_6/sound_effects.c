
#include <sounds.h>
#include <sound_effects.h>
#include <stddef.h>
#include <queue.h>
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

// Each instance of the effect_task will be given a unique sound to
// play, and a unique trigger event using the following structure. The
// pointer to an effect task's structure will be passed to it using
// the "params" mechanism in xTaskCreateStatic.
typedef struct{
  effect_buffer *buffers;
  int num_buffers;
  EventBits_t event;
  QueueHandle_t sendqueue;
}effect_param_t;

// The audio data and events that trigger them are known at compile
// time, but the queue handles must be filled in at run time by the
// effect_init() function.
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


// The interrupt handler for the audio pulse modulator
void audio_handler(BaseType_t *HPTW)
{
  static uint16_t *buffer = NULL; // make it static so that it always exists.

  // may need more static variables here

  // if buffer == null, then get a buffer from the queue.

  // While the PM FIFO is not full,
  //   Transfer a data item from the buffer to the FIFO

  //   You may not have a valid buffer (the mixer may not
  //   have run yet) In that case, write a zero .

  //   If you send the last item in your current buffer, then send the
  //   pointer back to the mixer and get another buffer from the
  //   mixer.
  
}


// define the final audio depth (after the mixer) and sample frequency
#define DEPTH 10
#define FREQ  8000

// Create the audio buffers for communication between the mixer and ISR
// Using 4 buffers is only a suggestion.
#define NUM_MIXER_BUFFERS 4
static uint16_t mixer_buffers[NUM_MIXER_BUFFERS][EFFECT_BUFFER_SIZE];

// The mixer task receives data from the individual effect tasks, and
// mixes the audio data before sending it to the ISR.
static void effect_mixer_task(void *params)
{
  uint16_t *buffer;

  // Initialization:

  // Put the pointers to the NUM_MIXER_BUFFERS mixer_buffers in the PM_to_mixer queue
  
  // configure and enable the pulse modulator

  while (1)
    {
      // Part 1:
      // Pick one of the sound effects.  For each chunk of data in the sound effect:
      //   Get a mixer buffer pointer from the ISR to mixer queue
      //   Copy (making adjustments) the data from the sound effect into it.
      //   Send the mixer buffer pointer to the mixer to ISR queue

      // Part 2: (comment out part 1)
      //   Get a mixer buffer pointer from the ISR to mixer queue
      //   Get incoming data pointers from all of the sound effects queues.
      //   Add all of the incoming data streams and store the results in the mixer buffer. 
      //   Send the mixer buffer pointer to the mixer to ISR queue
    }
}


// Each sound effect is managed by an instance of this task.
static void effect_task(void *params)
{
  // typecast the params pointer so we can access our effect data
  effect_param_t *my_effect = (effect_param_t*)params;

  while(1)
    {
      // Block until my event occurs.
      // loop:
      //   send pointers to my buffers to my send queue
      //   until I have sent all of my buffers.
    }
}



// declare storage for the mixer task
#define MIXER_STACK_SIZE 512
static TaskHandle_t mixer_task_handle;
static StackType_t  mixer_stack[MIXER_STACK_SIZE];
static StaticTask_t mixer_TCB;

// define storage for the ISR to Mixer and Mixer to ISR queues
static StaticQueue_t MixerToISRqueue_QCB, ISRToMixerqueue_QCB;
static uint16_t *MixerToISRqueue_buf[NUM_MIXER_BUFFERS];
static uint16_t *ISRToMixerqueue_buf[NUM_MIXER_BUFFERS];

void effect_init() // main should call this function to set up the sound effects
{
  
  int i;
  
  // create all of the queues that will be used by the effects tasks
  // to send data to the mixer. Store their handles in the
  // effect_to_mixer_queues array

  // create the two queues to communicate between the mixer and the ISR
  
  // create all of the effect tasks, giving them each a unique queue handle and
  // other parameters (effect_params)

  // create the mixer task
}



