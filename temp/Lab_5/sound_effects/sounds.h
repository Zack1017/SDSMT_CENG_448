#ifndef SOUNDS_H
#define SOUNDS_H

#include <stdint.h>

#define EFFECT_BUFFER_SIZE 128

typedef struct {
  int8_t data[EFFECT_BUFFER_SIZE];
} effect_buffer;

#define NUM_explosion1_BUFFERS 57
extern const effect_buffer explosion1[NUM_explosion1_BUFFERS];

#define NUM_fastinvader1_BUFFERS 7
extern const effect_buffer fastinvader1[NUM_fastinvader1_BUFFERS];

#define NUM_fastinvader2_BUFFERS 7
extern const effect_buffer fastinvader2[NUM_fastinvader2_BUFFERS];

#define NUM_fastinvader3_BUFFERS 7
extern const effect_buffer fastinvader3[NUM_fastinvader3_BUFFERS];

#define NUM_fastinvader4_BUFFERS 8
extern const effect_buffer fastinvader4[NUM_fastinvader4_BUFFERS];

#define NUM_invaderkilled_BUFFERS 22
extern const effect_buffer invaderkilled[NUM_invaderkilled_BUFFERS];

#define NUM_shoot_BUFFERS 27
extern const effect_buffer shoot[NUM_shoot_BUFFERS];

#define NUM_ufo_highpitch_BUFFERS 12
extern const effect_buffer ufo_highpitch[NUM_ufo_highpitch_BUFFERS];

#define NUM_ufo_lowpitch_BUFFERS 168
extern const effect_buffer ufo_lowpitch[NUM_ufo_lowpitch_BUFFERS];

#endif