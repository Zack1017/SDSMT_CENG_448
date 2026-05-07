#include <sounds.h>
#include <sound_effects.h>

#include <stdint.h>
#include <stddef.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <event_groups.h>

#include <LDP-001_PM_driver.h>
volatile uint32_t sound_debug_step = 0x12345678;
#define CHANNEL 0

EventGroupHandle_t effect_events;
static StaticEventGroup_t effect_events_buffer;
/*
 * Lab audio:
 * - input samples are signed 8-bit
 * - final pulse modulator output is unsigned DEPTH-bit duty
 * - 10-bit output gives range 0..1023
 */
#define DEPTH 10
#define FREQ  8000

#define AUDIO_CENTER      (1U << (DEPTH - 1))      // 512
#define AUDIO_MAX_DUTY    ((1U << DEPTH) - 1U)     // 1023
#define AUDIO_MIN_DUTY    0U

/*
 * Number of mixer buffers passed between mixer and ISR.
 * Each buffer holds 128 samples.
 */
#define NUM_MIXER_BUFFERS 4

/*
 * Per-effect queue length.
 * Small queue gives natural backpressure so an effect task does not dump
 * the entire sound instantly if the mixer is behind.
 */
#define EFFECT_QUEUE_LENGTH 4

/*
 * Optional test task.
 * Set to 1 while debugging Part 2.
 * Set to 0 when enabling nInvaders.
 */
#define ENABLE_SOUND_TEST_TASK 0

/*
 * If set to 1, mixer ignores all effect tasks and repeatedly plays shoot.
 * Useful for Lab 6 Part 1.
 *
 * For Part 2 and Part 3, set this to 0.
 */
#define ENABLE_PART1_MIXER_TEST 1

volatile uint32_t audio_isr_count = 0;
volatile uint32_t mixer_start_count = 0;
volatile uint32_t mixer_loop_count = 0;
volatile uint32_t mixer_send_count = 0;
volatile uint32_t mixer_return_count = 0;




// Each sound effect task sends audio buffer pointers to the mixer.
static QueueHandle_t effect_to_mixer_queues[NUM_EFFECTS];

// The mixer sends mixed uint16_t buffer pointers to the ISR.
static QueueHandle_t MixerToISRqueue;

// The ISR returns consumed uint16_t buffer pointers to the mixer.
static QueueHandle_t ISRToMixerqueue;

typedef struct {
    const effect_buffer *buffers;
    int num_buffers;
    EventBits_t event;
    QueueHandle_t sendqueue;
} effect_param_t;

static effect_param_t effect_task_params[NUM_EFFECTS] = {
    { explosion1,      NUM_explosion1_BUFFERS,      EXPLOSION1_EVENT,    NULL },
    { fastinvader1,   NUM_fastinvader1_BUFFERS,   FASTINVADER1_EVENT,   NULL },
    { fastinvader2,   NUM_fastinvader2_BUFFERS,   FASTINVADER2_EVENT,   NULL },
    { fastinvader3,   NUM_fastinvader3_BUFFERS,   FASTINVADER3_EVENT,   NULL },
    { fastinvader4,   NUM_fastinvader4_BUFFERS,   FASTINVADER4_EVENT,   NULL },
    { invaderkilled,  NUM_invaderkilled_BUFFERS,  INVADERKILLED_EVENT,  NULL },
    { shoot,          NUM_shoot_BUFFERS,          SHOOT_EVENT,          NULL },
    { ufo_highpitch,  NUM_ufo_highpitch_BUFFERS,  UFO_HIGHPITCH_EVENT,  NULL },
    //{ ufo_lowpitch,   NUM_ufo_lowpitch_BUFFERS,   UFO_LOWPITCH_EVENT,   NULL }
};



static uint16_t mixer_buffers[NUM_MIXER_BUFFERS][EFFECT_BUFFER_SIZE];

/*
 * Convert mixed signed audio into unsigned PM duty.
 */
static uint16_t audio_to_duty(int mixed_sample)
{
    /*
     * Input samples are signed 8-bit. Scaling by 2 makes a single
     * effect use a reasonable part of the 10-bit output range.
     */
    int duty = (int)AUDIO_CENTER + (mixed_sample * 2);

    if (duty < (int)AUDIO_MIN_DUTY) {
        duty = AUDIO_MIN_DUTY;
    }

    if (duty > (int)AUDIO_MAX_DUTY) {
        duty = AUDIO_MAX_DUTY;
    }

    return (uint16_t)duty;
}

/*
 * Audio interrupt handler for the pulse modulator.
 *
 * The PM driver calls this through PM_handler().
 * It must not block.
 */
void audio_handler(void)
{
    audio_isr_count++;

    static uint16_t *buffer = NULL;
    static int buffer_index = 0;

    BaseType_t higher_priority_task_woken = pdFALSE;

    while (!PM_FIFO_full(CHANNEL)) {
        if (buffer == NULL) {
            buffer_index = 0;

            if (xQueueReceiveFromISR(MixerToISRqueue,
                                     &buffer,
                                     &higher_priority_task_woken) != pdPASS) {
                /*
                 * Mixer has not produced data yet.
                 * Output silence.
                 */
                PM_set_duty(CHANNEL, AUDIO_CENTER);
                continue;
            }
        }

        PM_set_duty(CHANNEL, buffer[buffer_index]);
        buffer_index++;

        if (buffer_index >= EFFECT_BUFFER_SIZE) {
            uint16_t *finished_buffer = buffer;
            buffer = NULL;
            buffer_index = 0;

            xQueueSendFromISR(ISRToMixerqueue,
                              &finished_buffer,
                              &higher_priority_task_woken);
        }
    }

    portYIELD_FROM_ISR(higher_priority_task_woken);
}

/*
 * Mixer task:
 * - waits for an available output buffer returned by the ISR
 * - mixes one 128-sample chunk
 * - sends the mixed buffer to the ISR
 */
static void effect_mixer_task(void *params)
{
    (void)params;

    mixer_start_count++;

    uint16_t *out_buffer = NULL;

    /*
     * Give all mixer buffers to the mixer/ISR free-buffer queue first.
     * The mixer will take one, fill it, and send it to the ISR.
     * The ISR returns it after playback.
     */
    for (int i = 0; i < NUM_MIXER_BUFFERS; i++) {
        uint16_t *p = mixer_buffers[i];
        xQueueSend(ISRToMixerqueue, &p, portMAX_DELAY);
        mixer_return_count++;
    }

    /*
     * Configure pulse modulator for audio.
     *
     * divisions = 2^DEPTH gives 10-bit duty range.
     * base_frequency = FREQ gives 8000 output samples/sec.
     */
    PM_acquire(CHANNEL);
    PM_set_cycle_time(CHANNEL, (1 << DEPTH), FREQ);
    PM_set_PDM_mode(CHANNEL);
    PM_enable_FIFO(CHANNEL);
    PM_set_handler(CHANNEL, audio_handler);
    PM_enable(CHANNEL);

    while (1) {
         mixer_loop_count++;
        xQueueReceive(ISRToMixerqueue, &out_buffer, portMAX_DELAY);

#if ENABLE_PART1_MIXER_TEST
        /*
         * Lab 6 Part 1:
         * Repeatedly play one effect without using the effect tasks.
         */
        for (int b = 0; b < NUM_shoot_BUFFERS; b++) {
            for (int i = 0; i < EFFECT_BUFFER_SIZE; i++) {
                out_buffer[i] = audio_to_duty(shoot[b].data[i]);
            }

            xQueueSend(MixerToISRqueue, &out_buffer, portMAX_DELAY);
            mixer_send_count++;
            xQueueReceive(ISRToMixerqueue, &out_buffer, portMAX_DELAY);
            mixer_return_count++;
        }

        for (int i = 0; i < EFFECT_BUFFER_SIZE; i++) {
            out_buffer[i] = AUDIO_CENTER;
        }

        xQueueSend(MixerToISRqueue, &out_buffer, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(500));

#else
        /*
         * Lab 6 Part 2/3:
         * Pull one chunk from each active sound queue and mix them.
         */
        const effect_buffer *active_buffers[NUM_EFFECTS];

        for (int effect = 0; effect < NUM_EFFECTS; effect++) {
            active_buffers[effect] = NULL;

            if (effect_to_mixer_queues[effect] != NULL) {
        xQueueReceive(effect_to_mixer_queues[effect],
                      &active_buffers[effect],
                      0);
    }
        }

        for (int sample = 0; sample < EFFECT_BUFFER_SIZE; sample++) {
            int mixed = 0;

            for (int effect = 0; effect < NUM_EFFECTS; effect++) {
                if (active_buffers[effect] != NULL) {
                    mixed += active_buffers[effect]->data[sample];
                }
            }

            out_buffer[sample] = audio_to_duty(mixed);
        }

        xQueueSend(MixerToISRqueue, &out_buffer, portMAX_DELAY);
#endif
    }
}

/*
 * One instance of this task exists for each sound effect.
 * It waits for its event bit, then streams that sound's buffer pointers
 * to its queue.
 */
static void effect_task(void *params)
{
    effect_param_t *my_effect = (effect_param_t *)params;

    while (1) {
        xEventGroupWaitBits(effect_events,
                            my_effect->event,
                            pdTRUE,      // clear bit on exit
                            pdFALSE,     // wait for any matching bit
                            portMAX_DELAY);

        for (int i = 0; i < my_effect->num_buffers; i++) {
            const effect_buffer *buf = &my_effect->buffers[i];

            xQueueSend(my_effect->sendqueue,
                       &buf,
                       portMAX_DELAY);
        }
    }
}

#if ENABLE_SOUND_TEST_TASK
static void sound_test_task(void *params)
{
    (void)params;

    const EventBits_t test_events[] = {
        SHOOT_EVENT,
        INVADERKILLED_EVENT,
        EXPLOSION1_EVENT,
        FASTINVADER1_EVENT,
        FASTINVADER2_EVENT,
        FASTINVADER3_EVENT,
        FASTINVADER4_EVENT,
        UFO_HIGHPITCH_EVENT,
        //UFO_LOWPITCH_EVENT
    };

    int index = 0;

    while (1) {
        xEventGroupSetBits(effect_events, test_events[index]);

        index++;
        if (index >= (int)(sizeof(test_events) / sizeof(test_events[0]))) {
            index = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
#endif

#define MIXER_STACK_SIZE 2048
static TaskHandle_t mixer_task_handle;
static StackType_t  mixer_stack[MIXER_STACK_SIZE];
static StaticTask_t mixer_TCB;

#define EFFECT_STACK_SIZE 1024
static TaskHandle_t effect_task_handles[NUM_EFFECTS];
static StackType_t effect_task_stacks[NUM_EFFECTS][EFFECT_STACK_SIZE];
static StaticTask_t effect_task_TCBs[NUM_EFFECTS];

#if ENABLE_SOUND_TEST_TASK
#define SOUND_TEST_STACK_SIZE 256
static TaskHandle_t sound_test_task_handle;
static StackType_t sound_test_stack[SOUND_TEST_STACK_SIZE];
static StaticTask_t sound_test_TCB;
#endif

static StaticQueue_t MixerToISRqueue_QCB;
static StaticQueue_t ISRToMixerqueue_QCB;

static uint8_t MixerToISRqueue_buf[NUM_MIXER_BUFFERS * sizeof(uint16_t *)];
static uint8_t ISRToMixerqueue_buf[NUM_MIXER_BUFFERS * sizeof(uint16_t *)];

static StaticQueue_t effect_queue_QCBs[NUM_EFFECTS];
static uint8_t effect_queue_bufs[NUM_EFFECTS]
                                [EFFECT_QUEUE_LENGTH * sizeof(const effect_buffer *)];

/*
 * TEMP TEST:
 * Standalone effect queue storage.
 * This bypasses effect_queue_QCBs[0] and effect_queue_bufs[0]
 * so we can test whether the array-based storage is causing the fault.
 */
static StaticQueue_t effect0_queue_QCB;
static uint8_t effect0_queue_storage[EFFECT_QUEUE_LENGTH * sizeof(const effect_buffer *)]
    __attribute__((aligned(4)));

void effect_init(void)
{

    
    /*
     * Create the event group used by nInvaders / sound tasks
     * to trigger individual sound effects.
     */
    effect_events = xEventGroupCreateStatic(&effect_events_buffer);
    configASSERT(effect_events != NULL);

    /*
     * Clear all queue handles before creating them.
     * This makes mixer-side NULL checks safe if debugging later.
     */
    for (int i = 0; i < NUM_EFFECTS; i++) {
        effect_to_mixer_queues[i] = NULL;
    }

    /*
     * Queue of filled mixer buffers going to the PM/audio ISR.
     * Each queue item is a uint16_t* pointing to one mixer buffer.
     */
    MixerToISRqueue = xQueueCreateStatic(NUM_MIXER_BUFFERS,
                                         sizeof(uint16_t *),
                                         MixerToISRqueue_buf,
                                         &MixerToISRqueue_QCB);

    configASSERT(MixerToISRqueue != NULL);

    /*
     * Queue of empty/consumed mixer buffers returned from the ISR
     * back to the mixer task.
     */
    ISRToMixerqueue = xQueueCreateStatic(NUM_MIXER_BUFFERS,
                                         sizeof(uint16_t *),
                                         ISRToMixerqueue_buf,
                                         &ISRToMixerqueue_QCB);

    configASSERT(ISRToMixerqueue != NULL);

    /*
     * Create one queue and one task for each sound effect.
     */
    for (int i = 0; i < NUM_EFFECTS; i++) {
        effect_to_mixer_queues[i] =
            xQueueCreateStatic(EFFECT_QUEUE_LENGTH,
                               sizeof(const effect_buffer *),
                               effect_queue_bufs[i],
                               &effect_queue_QCBs[i]);

        configASSERT(effect_to_mixer_queues[i] != NULL);

        effect_task_params[i].sendqueue = effect_to_mixer_queues[i];

        effect_task_handles[i] =
            xTaskCreateStatic(effect_task,
                              "sound_fx",
                              EFFECT_STACK_SIZE,
                              &effect_task_params[i],
                              tskIDLE_PRIORITY + 2,
                              effect_task_stacks[i],
                              &effect_task_TCBs[i]);

        configASSERT(effect_task_handles[i] != NULL);
    }

    /*
     * Create the mixer task.
     */
    mixer_task_handle =
        xTaskCreateStatic(effect_mixer_task,
                          "mixer",
                          MIXER_STACK_SIZE,
                          NULL,
                          tskIDLE_PRIORITY + 3,
                          mixer_stack,
                          &mixer_TCB);

    configASSERT(mixer_task_handle != NULL);

#if ENABLE_SOUND_TEST_TASK
    /*
     * Optional test task.
     */
    sound_test_task_handle =
        xTaskCreateStatic(sound_test_task,
                          "sound_test",
                          SOUND_TEST_STACK_SIZE,
                          NULL,
                          tskIDLE_PRIORITY + 1,
                          sound_test_stack,
                          &sound_test_TCB);

    configASSERT(sound_test_task_handle != NULL);
#endif
}