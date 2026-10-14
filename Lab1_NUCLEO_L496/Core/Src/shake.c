#include "shake.h"

#include <stdint.h>


/*   
   MPU6050 SETTINGS
      */

#define MPU6050_ADDR             (0x68 << 1)

#define MPU6050_WHO_AM_I         0x75
#define MPU6050_PWR_MGMT_1       0x6B
#define MPU6050_ACCEL_XOUT_H     0x3B


/*   
   SHAKE GAME SETTINGS
      */

/*
 * Minimum change in acceleration required
 * to count as a shake.
 *
 * Easily adjustable.
 */
#define SHAKE_THRESHOLD_MG       600


/*
 * Once a shake is detected, keep the
 * shake flag active for 10 seconds.
 */
#define SHAKE_FLAG_TIME_MS       10000


/*   
   PRIVATE VARIABLES
      */

static int32_t previous_x = 0;
static int32_t previous_y = 0;
static int32_t previous_z = 0;

static uint8_t first_sample = 1;

static uint32_t shake_start_time = 0;

  HAL_StatusTypeDef SHAKE_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t who_am_i = 0;
    uint8_t wake = 0x00;

    HAL_StatusTypeDef status;


    /*   
       VERIFY MPU6050
          */

    status = HAL_I2C_Mem_Read(hi2c,
                              MPU6050_ADDR,
                              MPU6050_WHO_AM_I,
                              I2C_MEMADD_SIZE_8BIT,
                              &who_am_i,
                              1,
                              100);

    if (status != HAL_OK)
    {
        return status;
    }


    /*
     * A normal MPU6050 should return 0x68.
     */
    if (who_am_i != 0x68)
    {
        return HAL_ERROR;
    }


    /*   
       WAKE MPU6050
          */

    status = HAL_I2C_Mem_Write(hi2c,
                               MPU6050_ADDR,
                               MPU6050_PWR_MGMT_1,
                               I2C_MEMADD_SIZE_8BIT,
                               &wake,
                               1,
                               100);

    if (status != HAL_OK)
    {
        return status;
    }

    HAL_Delay(100);


    /* Reset internal shake state */
    previous_x = 0;
    previous_y = 0;
    previous_z = 0;

    first_sample = 1;
    shake_start_time = 0;


    return HAL_OK;
}

HAL_StatusTypeDef SHAKE_Update(I2C_HandleTypeDef *hi2c,
                               SHAKE_Data *data)
{
    uint8_t accel_data[6];

    int16_t accel_x_raw;
    int16_t accel_y_raw;
    int16_t accel_z_raw;

    HAL_StatusTypeDef status;


    /*   
       READ ACCELEROMETER
          */

    status = HAL_I2C_Mem_Read(hi2c,
                              MPU6050_ADDR,
                              MPU6050_ACCEL_XOUT_H,
                              I2C_MEMADD_SIZE_8BIT,
                              accel_data,
                              6,
                              100);

    if (status != HAL_OK)
    {
        return status;
    }


    /*   
       COMBINE HIGH/LOW BYTES
          */

    accel_x_raw =
        (int16_t)((accel_data[0] << 8) |
                   accel_data[1]);

    accel_y_raw =
        (int16_t)((accel_data[2] << 8) |
                   accel_data[3]);

    accel_z_raw =
        (int16_t)((accel_data[4] << 8) |
                   accel_data[5]);


    /*   
       CONVERT TO milli-g
          */

    /*
     * Default MPU6050 range:
     * +/-2g = 16384 counts per g
     */

    data->x_mg =
        ((int32_t)accel_x_raw * 1000) / 16384;

    data->y_mg =
        ((int32_t)accel_y_raw * 1000) / 16384;

    data->z_mg =
        ((int32_t)accel_z_raw * 1000) / 16384;


    /*   
       FIRST SAMPLE
          */

    if (first_sample)
    {
        previous_x = data->x_mg;
        previous_y = data->y_mg;
        previous_z = data->z_mg;

        data->delta_x = 0;
        data->delta_y = 0;
        data->delta_z = 0;

        data->shake_flag = 0;

        first_sample = 0;

        return HAL_OK;
    }


    /*   
       CALCULATE DELTA
          */

    data->delta_x =
        data->x_mg - previous_x;

    data->delta_y =
        data->y_mg - previous_y;

    data->delta_z =
        data->z_mg - previous_z;


    /* Absolute value */
    if (data->delta_x < 0)
    {
        data->delta_x = -data->delta_x;
    }

    if (data->delta_y < 0)
    {
        data->delta_y = -data->delta_y;
    }

    if (data->delta_z < 0)
    {
        data->delta_z = -data->delta_z;
    }


    /* Save readings for next update */
    previous_x = data->x_mg;
    previous_y = data->y_mg;
    previous_z = data->z_mg;


    /*   
       SHAKE DETECTION
          */

    if (data->shake_flag == 0)
    {
        if ((data->delta_x >= SHAKE_THRESHOLD_MG) ||
            (data->delta_y >= SHAKE_THRESHOLD_MG) ||
            (data->delta_z >= SHAKE_THRESHOLD_MG))
        {
            data->shake_flag = 1;

            shake_start_time = HAL_GetTick();
        }
    }


    /*   
       CLEAR FLAG AFTER 10 SEC
          */

    if (data->shake_flag == 1)
    {
        if ((HAL_GetTick() - shake_start_time)
            >= SHAKE_FLAG_TIME_MS)
        {
            data->shake_flag = 0;
        }
    }


    return HAL_OK;
}