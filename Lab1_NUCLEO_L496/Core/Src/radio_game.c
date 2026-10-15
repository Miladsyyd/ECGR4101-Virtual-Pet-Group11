#include "radio_game.h"
#include "sound.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define RADIO_TARGET_MIN      400
#define RADIO_TARGET_MAX      3600
#define RADIO_TOLERANCE       200
#define RADIO_HOLD_TIME_MS    5000
#define RADIO_CHANNEL_COUNT   10


/* Private radio-game state */
static uint32_t radio_target = 0;
static uint8_t target_channel = 0;

static uint8_t previous_channel = 255;

static uint8_t radio_hold_active = 0;
static uint32_t radio_hold_start_time = 0;
static uint8_t previous_hint_flag = 0;

void RadioGame_Init(RadioGame_Data *game)
{
    game->pot_raw = 0;
    game->pot_percent = 0;

    game->current_channel = 0;
    game->hint_flag = 0;
    game->complete_flag = 0;

    previous_channel = 255;

    previous_hint_flag = 0;

    radio_hold_active = 0;
    radio_hold_start_time = 0;

    srand(HAL_GetTick());

    radio_target =
        RADIO_TARGET_MIN +
        (rand() %
        (RADIO_TARGET_MAX - RADIO_TARGET_MIN + 1));

    target_channel =
        (radio_target * RADIO_CHANNEL_COUNT) / 4096;
}


void RadioGame_Update(ADC_HandleTypeDef *hadc,
                      UART_HandleTypeDef *huart,
                      RadioGame_Data *game)
{
    char msg[120];

    /*
     * READ POTENTIOMETER
     */
    HAL_ADC_Start(hadc);

    if (HAL_ADC_PollForConversion(hadc, 100) == HAL_OK)
    {
        game->pot_raw = HAL_ADC_GetValue(hadc);

        game->pot_percent =
            (game->pot_raw * 100) / 4095;
    }

    HAL_ADC_Stop(hadc);


    /*
     * DETERMINE RADIO CHANNEL
     */
    game->current_channel =
        (game->pot_raw * RADIO_CHANNEL_COUNT) / 4096;

    if (game->current_channel > 9)
    {
        game->current_channel = 9;
    }


    /*
     * Only print when channel changes
     */
    if (game->current_channel != previous_channel)
    {
        snprintf(msg,
                 sizeof(msg),
                 "\r\n=== RADIO CHANNEL %d ===\r\n",
                 game->current_channel);

        HAL_UART_Transmit(huart,
                          (uint8_t *)msg,
                          strlen(msg),
                          HAL_MAX_DELAY);

        previous_channel = game->current_channel;
    }


    /*
     * HINT FLAG
     */
    if ((game->current_channel == target_channel) &&
        (game->complete_flag == 0))
    {
        game->hint_flag = 1;
    }
    else
    {
        game->hint_flag = 0;
    }


    /*
     * Play hint sound only when entering
     * the target channel.
     */
    if ((game->hint_flag == 1) &&
        (previous_hint_flag == 0))
    {
        Sound_Play(SOUND_HAPPY);
    }

    previous_hint_flag = game->hint_flag;


    /*
     * DISTANCE FROM TARGET
     */
    int32_t radio_error =
        (int32_t)game->pot_raw -
        (int32_t)radio_target;

    if (radio_error < 0)
    {
        radio_error = -radio_error;
    }


    /*
     * HOLD TIMER
     */
    if (game->complete_flag == 0)
    {
        if (radio_error <= RADIO_TOLERANCE)
        {
            if (radio_hold_active == 0)
            {
                radio_hold_active = 1;
                radio_hold_start_time = HAL_GetTick();

                const char tuning_msg[] =
                    "Good signal! Hold this tuning...\r\n";

                HAL_UART_Transmit(huart,
                                  (uint8_t *)tuning_msg,
                                  sizeof(tuning_msg) - 1,
                                  HAL_MAX_DELAY);
            }

            if ((HAL_GetTick() - radio_hold_start_time)
                >= RADIO_HOLD_TIME_MS)
            {
                game->complete_flag = 1;

                radio_hold_active = 0;

                const char complete_msg[] =
                    "\r\n*** RADIO TUNED! PET IS LISTENING ***\r\n";

                HAL_UART_Transmit(huart,
                                  (uint8_t *)complete_msg,
                                  sizeof(complete_msg) - 1,
                                  HAL_MAX_DELAY);
            }
        }
        else
        {
            radio_hold_active = 0;
        }
    }
}


   

