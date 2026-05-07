#ifndef SOUND_EFFECTS_H
#define SOUND_EFFECTS_H

#include <FreeRTOS.h>
#include <event_groups.h>

// Set the number of individual sound effects
#define NUM_EFFECTS 8

// Define the event group and events that trigger the sound effects
#define EXPLOSION1_EVENT      (1U << 0)
#define FASTINVADER1_EVENT    (1U << 1)
#define FASTINVADER2_EVENT    (1U << 2)
#define FASTINVADER3_EVENT    (1U << 3)
#define FASTINVADER4_EVENT    (1U << 4)
#define INVADERKILLED_EVENT   (1U << 5)
#define SHOOT_EVENT           (1U << 6)
#define UFO_HIGHPITCH_EVENT   (1U << 7)
//#define UFO_LOWPITCH_EVENT    (1U << 8)

// When it is time to play a sound effect, signal the appropriate
// event on this event group.
extern EventGroupHandle_t effect_events;

// main must call this function to initialize all of the sound effects
void effect_init(void);

#endif
