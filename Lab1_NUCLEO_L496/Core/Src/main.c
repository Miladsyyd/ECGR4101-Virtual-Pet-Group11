/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : X-NUCLEO-GFX01M2 LCD demo for the NUCLEO-L496ZG-P
  *
  * This demo drives the 2.2" ILI9341 QVGA display on the X-NUCLEO-GFX01M2
  * expansion board over SPI1 and cycles through a few simple graphics demos:
  * color bars, filled/outlined shapes, text, and a live joystick (B1)
  * readout. See Drivers/BSP/GFX01M2 for the small display driver used here.
  *
  * WIRING - the GFX01M2 plugs directly onto the top of this board's CN11/CN12
  * ST Morpho headers. NOTE: ST's own datasheet (DB4236) only lists Nucleo-64
  * boards as compatible with the GFX01M2 - the NUCLEO-L496ZG-P (Nucleo-144)
  * is not officially validated, so UM2750 has no pin table for this exact
  * combination. Pins were instead cross-referenced against this board's own
  * schematic (mb1312-l4xxzx-a03): RESET=PA1, CS=PA9, DC=PB10, SPI1 SCK/MISO/
  * MOSI=PA5/PA6/PA7. See Drivers/BSP/GFX01M2/gfx01m2_conf.h for the full
  * derivation and the PA9/USB_VBUS_Pin sharing note.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "ili9341.h"
#include "gfx01m2_conf.h"
#include "anim.h"
#include "pet_anims.h"
#include "pet_sprites.h"
#include "pet_logic.h"
#include "shake.h"
#include "radio_game.h"
#include "sound.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
ADC_HandleTypeDef hadc1;
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef hlpuart1;
TIM_HandleTypeDef htim1;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
void PeriphCommonClock_Config(void);
static void MX_LPUART1_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);

static void Demo_ColorBars(void);
static void Demo_Shapes(void);
static void Demo_Text(void);
static void Demo_Joystick(void);

/*EDITED by Milad*******************************/

   static void Demo_Bitmap(void);
   static void Demo_Anim(void);

/*EDITED by Milad*******************************/

/* Joystick (B1) bit flags returned by Joystick_Read() */
#define JOY_LEFT_MASK   PET_BUTTON_LEFT
#define JOY_CENTER_MASK PET_BUTTON_CENTER
#define JOY_DOWN_MASK   PET_BUTTON_DOWN
#define JOY_RIGHT_MASK  PET_BUTTON_RIGHT
#define JOY_UP_MASK     PET_BUTTON_UP

static uint8_t Joystick_Read(void)
{
  uint8_t state = 0;

  /* All five directions are active low (see gfx01m2_conf.h) */
  if (HAL_GPIO_ReadPin(JOY_LEFT_GPIO_Port, JOY_LEFT_Pin) == GPIO_PIN_RESET)     { state |= JOY_LEFT_MASK; }
  if (HAL_GPIO_ReadPin(JOY_CENTER_GPIO_Port, JOY_CENTER_Pin) == GPIO_PIN_RESET) { state |= JOY_CENTER_MASK; }
  if (HAL_GPIO_ReadPin(JOY_DOWN_GPIO_Port, JOY_DOWN_Pin) == GPIO_PIN_RESET)     { state |= JOY_DOWN_MASK; }
  if (HAL_GPIO_ReadPin(JOY_RIGHT_GPIO_Port, JOY_RIGHT_Pin) == GPIO_PIN_RESET)   { state |= JOY_RIGHT_MASK; }
  if (HAL_GPIO_ReadPin(JOY_UP_GPIO_Port, JOY_UP_Pin) == GPIO_PIN_RESET)         { state |= JOY_UP_MASK; }

  return state;
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();
  PeriphCommonClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_LPUART1_UART_Init();
  MX_TIM1_Init();
  Sound_Init();


  const char startup_msg[] = "\r\nUART STARTED\r\n";

HAL_UART_Transmit(&hlpuart1,
                  (uint8_t *)startup_msg,
                  sizeof(startup_msg) - 1,
                  HAL_MAX_DELAY);


  /* Initialize the LCD and show a splash screen */
  ILI9341_Init(&hspi1);
  ILI9341_DrawString(18, 12, "GFX01M2 LCD DEMO", ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK, 2);
  ILI9341_DrawString(30, 40, "NUCLEO-L496ZG-P", ILI9341_COLOR_CYAN, ILI9341_COLOR_BLACK, 1);
  HAL_Delay(2000);

/*Sorayda*/
  Anim_Init(112, 60, PET_SPRITES_WIDTH, PET_SPRITES_HEIGHT, 3);
  PetAnims_RegisterAll();
  Pet_Init();
 
  SHAKE_Data shake = {0};
  RadioGame_Data radio = {0};

 PetState lastPetState = IDLE;
 
  while (1)
  {
    Anim_Update(HAL_GetTick());
    Sound_Update();
    UI_Update();
    Pet_Update();

   

    PetState currentPetState = getPetState();

   
    if (currentPetState != lastPetState)
    {
      if (currentPetState == SHAKE_GAME)
      {
        memset(&shake, 0, sizeof(shake));
        if (SHAKE_Init(&hi2c1) != HAL_OK)
        {
          Error_Handler();
        }
      }
      else if (currentPetState == RADIO_GAME)
      {
        RadioGame_Init(&radio);
      }

      lastPetState = currentPetState;
    }

   if (currentPetState == SHAKE_GAME)
    {
      if (SHAKE_Update(&hi2c1, &shake) != HAL_OK)
      {
        Error_Handler();
      }
      else if (shake.shake_flag != 0U)
      {
        Pet_GameComplete();
      }
    }
    else if (currentPetState == RADIO_GAME)
    {
      RadioGame_Update(&hadc1, &hlpuart1, &radio);
      if (radio.complete_flag != 0U)
      {
        Pet_GameComplete();
      }
    }

    HAL_Delay(10);
  }
}
   
static void MX_TIM1_Init(void)
{
  TIM_OC_InitTypeDef sConfigOC = {0};
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_TIM1_CLK_ENABLE();

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  __HAL_RCC_GPIOE_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

/**
  * @brief  Fills the screen with a set of vertical color bars.
  */
static void Demo_ColorBars(void)
{
  static const uint16_t colors[] =
  {
    ILI9341_COLOR_WHITE, ILI9341_COLOR_YELLOW, ILI9341_COLOR_CYAN, ILI9341_COLOR_GREEN,
    ILI9341_COLOR_MAGENTA, ILI9341_COLOR_RED, ILI9341_COLOR_BLUE, ILI9341_COLOR_ORANGE
  };
  const uint16_t bar_count = sizeof(colors) / sizeof(colors[0]);
  const uint16_t bar_width = ILI9341_WIDTH / bar_count;
  uint16_t i;

  for (i = 0; i < bar_count; i++)
  {
    ILI9341_FillRect((uint16_t)(i * bar_width), 0, bar_width, ILI9341_HEIGHT, colors[i]);
  }
}

/**
  * @brief  Draws a handful of filled and outlined shapes.
  */
static void Demo_Shapes(void)
{
  ILI9341_FillScreen(ILI9341_COLOR_BLACK);

  ILI9341_FillRect(20, 20, 80, 60, ILI9341_COLOR_RED);
  ILI9341_DrawRect(120, 20, 80, 60, ILI9341_COLOR_GREEN);

  ILI9341_DrawLine(20, 110, 220, 110, ILI9341_COLOR_WHITE);
  ILI9341_DrawLine(20, 110, 220, 260, ILI9341_COLOR_YELLOW);
  ILI9341_DrawLine(220, 110, 20, 260, ILI9341_COLOR_CYAN);

  ILI9341_FillRect(70, 220, 100, 40, ILI9341_COLOR_BLUE);
  ILI9341_DrawRect(65, 215, 110, 50, ILI9341_COLOR_WHITE);
}

/**
  * @brief  Demonstrates text rendering at a few different sizes.
  */
static void Demo_Text(void)
{
  ILI9341_FillScreen(ILI9341_COLOR_BLACK);

  ILI9341_DrawString(10, 20, "HELLO CLASS!", ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK, 3);
  ILI9341_DrawString(10, 60, "ILI9341 QVGA TFT", ILI9341_COLOR_GREEN, ILI9341_COLOR_BLACK, 2);
  ILI9341_DrawString(10, 90, "DRIVEN OVER SPI1", ILI9341_COLOR_GREEN, ILI9341_COLOR_BLACK, 2);
  ILI9341_DrawString(10, 130, "TRY CHANGING THE", ILI9341_COLOR_YELLOW, ILI9341_COLOR_BLACK, 1);
  ILI9341_DrawString(10, 145, "COLORS AND TEXT IN", ILI9341_COLOR_YELLOW, ILI9341_COLOR_BLACK, 1);
  ILI9341_DrawString(10, 160, "DEMO TEXT() IN MAIN.C", ILI9341_COLOR_YELLOW, ILI9341_COLOR_BLACK, 1);
}

/*MIIIIIIIIIIIIIIIIIIIIIIIIIIIIIILAD*/

   /**
     * @brief  Tests ILI9341_DrawBitmap with a generated 48x48 image:
     *         red, green and blue stripes, plus a white square in the
     *         top-left corner to check orientation.
     */
   #define TEST_IMG_W 48U
   #define TEST_IMG_H 48U
   static uint16_t test_img[TEST_IMG_W * TEST_IMG_H];  /* static: 1 KB stack */

   static void Demo_Bitmap(void)
   {
     uint16_t x, y;

     for (y = 0; y < TEST_IMG_H; y++)
     {
       for (x = 0; x < TEST_IMG_W; x++)
       {
         uint16_t c;
         if (y < 16U)      { c = ILI9341_COLOR_RED; }
         else if (y < 32U) { c = ILI9341_COLOR_GREEN; }
         else              { c = ILI9341_COLOR_BLUE; }
         if ((x < 8U) && (y < 8U)) { c = ILI9341_COLOR_WHITE; }
         test_img[y * TEST_IMG_W + x] = c;
       }
     }

     ILI9341_FillScreen(ILI9341_COLOR_BLACK);
     ILI9341_DrawString(10, 10, "BITMAP TEST", ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK, 2);
     ILI9341_DrawBitmap(96, 100, TEST_IMG_W, TEST_IMG_H, test_img);
     ILI9341_DrawString(10, 170, "RED/GRN/BLU, WHITE TOP-LEFT", ILI9341_COLOR_YELLOW, ILI9341_COLOR_BLACK, 1);
   }


   /**
  * @brief  Shows the real pet for 20 s. Idle by default; joystick:
  *         CENTER = feed, RIGHT = play, LEFT = sleep,
  *         UP = runaway, DOWN = death.
  */
static void Demo_Anim(void)
{
  uint32_t start = HAL_GetTick();
  uint8_t prev_joy = 0;

  ILI9341_FillScreen(ILI9341_COLOR_BLACK);
  ILI9341_DrawString(10, 10, "ANIM TEST", ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK, 2);
  ILI9341_DrawString(10, 40, "C=FEED R=PLAY L=SLEEP", ILI9341_COLOR_YELLOW, ILI9341_COLOR_BLACK, 1);
  ILI9341_DrawString(10, 55, "U=RUNAWAY D=DEATH", ILI9341_COLOR_YELLOW, ILI9341_COLOR_BLACK, 1);

  Anim_Init(40, 90, PET_SPRITES_WIDTH, PET_SPRITES_HEIGHT, 5);   /* 32x32 drawn as 128x128, centered */  PetAnims_RegisterAll();
  Anim_Play(ANIM_IDLE);

  while ((HAL_GetTick() - start) < 20000U)
  {
    uint8_t joy = Joystick_Read();
    uint8_t pressed = (uint8_t)(joy & (uint8_t)~prev_joy);  /* new presses only */
    prev_joy = joy;

    if      (pressed & JOY_CENTER_MASK) { Anim_Play(ANIM_FEED); }
    else if (pressed & JOY_RIGHT_MASK)  { Anim_Play(ANIM_PLAY); }
    else if (pressed & JOY_LEFT_MASK)   { Anim_Play(ANIM_ACTION3); }
    else if (pressed & JOY_UP_MASK)     { Anim_Play(ANIM_RUNAWAY); }
    else if (pressed & JOY_DOWN_MASK)   { Anim_Play(ANIM_DEATH); }

    /* Feed and play return to idle; death and runaway stay on their last frame */
    if (Anim_IsFinished() &&
        ((Anim_Current() == ANIM_FEED) || (Anim_Current() == ANIM_PLAY)))
    {
      Anim_Play(ANIM_IDLE);
    }

    Anim_Update(HAL_GetTick());
  }
}



/**
  * @brief  Live joystick (B1) status readout for a few seconds.
  */
static void Demo_Joystick(void)
{
  uint32_t start = HAL_GetTick();
  char line[] = "L:_ C:_ D:_ R:_ U:_";

  ILI9341_FillScreen(ILI9341_COLOR_BLACK);
  ILI9341_DrawString(10, 10, "JOYSTICK TEST", ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK, 2);
  ILI9341_DrawString(10, 40, "MOVE THE STICK (B1)", ILI9341_COLOR_YELLOW, ILI9341_COLOR_BLACK, 1);

  while ((HAL_GetTick() - start) < 5000U)
  {
    uint8_t joy = Joystick_Read();

    line[2]  = (joy & JOY_LEFT_MASK)   ? '1' : '0';
    line[6]  = (joy & JOY_CENTER_MASK) ? '1' : '0';
    line[10] = (joy & JOY_DOWN_MASK)   ? '1' : '0';
    line[14] = (joy & JOY_RIGHT_MASK)  ? '1' : '0';
    line[18] = (joy & JOY_UP_MASK)     ? '1' : '0';

    ILI9341_DrawString(10, 70, line, ILI9341_COLOR_GREEN, ILI9341_COLOR_BLACK, 2);
    HAL_Delay(50);
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLLSAI1;
  PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_MSI;
  PeriphClkInit.PLLSAI1.PLLSAI1M = 5;
  PeriphClkInit.PLLSAI1.PLLSAI1N = 20;
  PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_48M2CLK|RCC_PLLSAI1_ADC1CLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00201A25;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief LPUART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPUART1_UART_Init(void)
{

  /* USER CODE BEGIN LPUART1_Init 0 */

  /* USER CODE END LPUART1_Init 0 */

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 115200;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPUART1_Init 2 */

  /* USER CODE END LPUART1_Init 2 */

}


/**
  * @brief SPI1 Initialization Function
  *        SCK = PA5, MISO = PA6, MOSI = PA7 - connects to the GFX01M2 LCD.
  * @retval None
  */
static void MX_SPI1_Init(void)
{
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  /* PCLK2 is now ~56.8 MHz (HCLK was previously quartered by a stray AHB
   * DIV4 - see SystemClock_Config). /32 keeps the same ~1.775 MHz target
   * that gave comfortable margin over the ILI9341's 10 MHz write ceiling
   * on this jumper-wired, unsupported-board setup. */
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_9;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 71;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  /* DIV4 here previously left HCLK at 56.8/4 = 14.2 MHz despite the PLL
   * being configured for 56.8 MHz - DIV1 makes HCLK match the PLL output. */
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  /* HCLK is now ~56.8 MHz - VOS Range 1 requires 3 wait states for
   * 48-64 MHz (RM0351), not the 0 WS that was only valid up to 16 MHz. */
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  HAL_PWREx_EnableVddIO2();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, USB_PowerSwitchOn_Pin|SMPS_V1_Pin|SMPS_EN_Pin|SMPS_SW_Pin, GPIO_PIN_RESET);

  /* LCD CS/RESET idle high, DC state does not matter until the first transfer */
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : USB_OverCurrent_Pin SMPS_PG_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin|SMPS_PG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : USB_PowerSwitchOn_Pin SMPS_V1_Pin SMPS_EN_Pin SMPS_SW_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin|SMPS_V1_Pin|SMPS_EN_Pin|SMPS_SW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_CS_Pin (X-NUCLEO-GFX01M2) */
  GPIO_InitStruct.Pin = LCD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(LCD_CS_GPIO_Port, &GPIO_InitStruct);

  /* LCD_DC is on a different port than LCD_CS (GPIOB vs GPIOA) - must be a separate HAL_GPIO_Init call */
  GPIO_InitStruct.Pin = LCD_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(LCD_DC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_RESET_Pin (X-NUCLEO-GFX01M2) */
  GPIO_InitStruct.Pin = LCD_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(LCD_RESET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : JOY_LEFT_Pin JOY_DOWN_Pin JOY_RIGHT_Pin (X-NUCLEO-GFX01M2 joystick) */
  GPIO_InitStruct.Pin = JOY_LEFT_Pin|JOY_DOWN_Pin|JOY_RIGHT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : JOY_CENTER_Pin JOY_UP_Pin (X-NUCLEO-GFX01M2 joystick) */
  GPIO_InitStruct.Pin = JOY_CENTER_Pin|JOY_UP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
