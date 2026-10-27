/* radio_game.h */

#ifndef RADIO_GAME_H
#define RADIO_GAME_H

#include "stm32l4xx_hal.h"

typedef struct
{
    uint32_t pot_raw;
    uint32_t pot_percent;

    uint8_t current_channel;
    uint8_t hint_flag;
    uint8_t complete_flag;

} RadioGame_Data;

void RadioGame_Init(RadioGame_Data *game);

void RadioGame_Update(ADC_HandleTypeDef *hadc,
                      UART_HandleTypeDef *huart,
                      RadioGame_Data *game);

#endif