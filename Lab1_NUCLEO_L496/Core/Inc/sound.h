#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>

typedef enum {
    SOUND_NONE = 0,
    SOUND_HAPPY,
    SOUND_HUNGRY,
    SOUND_DYING
} SoundId;

// Call once after MX_TIM1_Init(). Starts PWM silent on TIM1_CH1 (PE9, Zio D6). 
void Sound_Init(void);

// Start a sound. Returns immediately (non-blocking). 
void Sound_Play(SoundId id);

// Stop the current sound. */
void Sound_Stop(void);

// Call every pass of the main loop. Advances to the next note by time. 
void Sound_Update(void);

// 1 while a sound is playing, 0 otherwise. 
uint8_t Sound_IsPlaying(void);

#endif /* SOUND_H */