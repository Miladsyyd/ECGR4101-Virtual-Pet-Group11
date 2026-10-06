/**
 ******************************************************************************
 * @file    anim.h
 * @brief   Non-blocking sprite animation engine for the virtual pet.
 *
 * Animations are registered from outside (frame tables live with the sprite
 * data), then started with Anim_Play(). Anim_Update() must be called every
 * pass of the main loop; it never waits, so input handling stays responsive.
 ******************************************************************************
 */
#ifndef ANIM_H
#define ANIM_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    ANIM_IDLE = 0,
    ANIM_FEED,
    ANIM_PLAY,
    ANIM_ACTION3,
    ANIM_DEATH,
    ANIM_RUNAWAY,
    ANIM_COUNT
} AnimId;

typedef struct
{
    const uint16_t *const *frames;  /* array of pointers to RGB565 frames */
    uint8_t  count;                 /* number of frames */
    uint16_t frame_ms;              /* time each frame is shown */
    bool     loop;                  /* true = repeat, false = play once */
} Animation;

/* w, h = frame size in memory; scale = drawn size multiplier (1 = actual size). */
void   Anim_Init(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t scale);

/* Connects an animation (frame table + timing) to an AnimId. */
void   Anim_Register(AnimId id, const Animation *anim);

/* Starts an animation from its first frame. Ignored if not registered. */
void   Anim_Play(AnimId id);

/* Call every main-loop pass. Advances frames when due and redraws only
 * when the frame actually changes. Never blocks. */
void   Anim_Update(uint32_t now_ms);

/* True when a play-once animation has shown its last frame. */
bool   Anim_IsFinished(void);

/* The animation currently selected. */
AnimId Anim_Current(void);

#endif /* ANIM_H */