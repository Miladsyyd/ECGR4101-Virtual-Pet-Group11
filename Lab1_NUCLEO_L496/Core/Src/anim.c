/**
 ******************************************************************************
 * @file    anim.c
 * @brief   Non-blocking sprite animation engine (see anim.h).
 ******************************************************************************
 */
#include "anim.h"
#include "ili9341.h"
#include "stm32l4xx_hal.h"
#include <stddef.h>

static const Animation *anims[ANIM_COUNT];
static uint16_t pos_x, pos_y, spr_w, spr_h;
static AnimId   cur = ANIM_IDLE;
static uint8_t  idx;
static uint32_t last_ms;
static bool     finished = true;
static bool     dirty;

void Anim_Init(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    uint8_t i;

    pos_x = x; pos_y = y; spr_w = w; spr_h = h;
    for (i = 0; i < (uint8_t)ANIM_COUNT; i++)
    {
        anims[i] = NULL;
    }
    cur = ANIM_IDLE; idx = 0; finished = true; dirty = false;
}

void Anim_Register(AnimId id, const Animation *anim)
{
    if (id < ANIM_COUNT)
    {
        anims[id] = anim;
    }
}

void Anim_Play(AnimId id)
{
    /* Invalid or empty animations are ignored instead of crashing */
    if ((id >= ANIM_COUNT) || (anims[id] == NULL) || (anims[id]->count == 0U))
    {
        return;
    }
    cur = id;
    idx = 0;
    finished = false;
    dirty = true;                 /* draw the first frame immediately */
    last_ms = HAL_GetTick();
}

void Anim_Update(uint32_t now_ms)
{
    const Animation *a = anims[cur];

    if ((a == NULL) || (a->count == 0U))
    {
        return;
    }

    /* Unsigned subtraction stays correct even when the tick counter wraps */
    if (!finished && ((now_ms - last_ms) >= a->frame_ms))
    {
        last_ms = now_ms;
        if ((uint8_t)(idx + 1U) < a->count)
        {
            idx++;
            dirty = true;
        }
        else if (a->loop)
        {
            idx = 0;
            dirty = true;
        }
        else
        {
            finished = true;      /* stay on the last frame, nothing to redraw */
        }
    }

    /* Redraw only when the frame changed: about 40 ms of SPI per 64x64 frame */
    if (dirty)
    {
        ILI9341_DrawBitmap(pos_x, pos_y, spr_w, spr_h, a->frames[idx]);
        dirty = false;
    }
}

bool Anim_IsFinished(void)
{
    return finished;
}

AnimId Anim_Current(void)
{
    return cur;
}