/**
 * @file    pet_anims.c
 * @brief   Frame tables and timing for each pet animation.
 *          All tables are const, so they stay in Flash.
 */
#include "pet_anims.h"
#include "anim.h"
#include "pet_sprites.h"

static const uint16_t *const idle_frames[]    = { pet_idle_0,    pet_idle_1,    pet_idle_2,    pet_idle_3 };
static const uint16_t *const feed_frames[]    = { pet_feed_0,    pet_feed_1,    pet_feed_2,    pet_feed_3 };
static const uint16_t *const play_frames[]    = { pet_play_0,    pet_play_1,    pet_play_2,    pet_play_3 };
static const uint16_t *const sleep_frames[]   = { pet_sleep_0,   pet_sleep_1,   pet_sleep_2,   pet_sleep_3 };
static const uint16_t *const death_frames[]   = { pet_death_0,   pet_death_1,   pet_death_2,   pet_death_3 };
static const uint16_t *const runaway_frames[] = { pet_runaway_0, pet_runaway_1, pet_runaway_2, pet_runaway_3 };

/*                                 frames          count  ms/frame  loop */
static const Animation pet_anims[ANIM_COUNT] = {
    [ANIM_IDLE]    = { idle_frames,    4U,    250U,   true  },
    [ANIM_FEED]    = { feed_frames,    4U,    200U,   false },
    [ANIM_PLAY]    = { play_frames,    4U,    150U,   false },
    [ANIM_ACTION3] = { sleep_frames,   4U,    400U,   true  },  /* sleep */
    [ANIM_DEATH]   = { death_frames,   4U,    400U,   false },
    [ANIM_RUNAWAY] = { runaway_frames, 4U,    250U,   false },
};

void PetAnims_RegisterAll(void)
{
    uint8_t i;

    for (i = 0; i < (uint8_t)ANIM_COUNT; i++)
    {
        Anim_Register((AnimId)i, &pet_anims[i]);
    }
}