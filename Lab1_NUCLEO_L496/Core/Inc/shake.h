#ifndef SHAKE_H
#define SHAKE_H

#include "stm32l4xx_hal.h"

typedef struct
{
    int32_t x_mg;
    int32_t y_mg;
    int32_t z_mg;

    int32_t delta_x;
    int32_t delta_y;
    int32_t delta_z;

    uint8_t shake_flag;

} SHAKE_Data;


/* Wake and verify MPU6050 */
HAL_StatusTypeDef SHAKE_Init(I2C_HandleTypeDef *hi2c);


/* Read MPU and update acceleration / shake state */
HAL_StatusTypeDef SHAKE_Update(I2C_HandleTypeDef *hi2c,
                               SHAKE_Data *data);

#endif