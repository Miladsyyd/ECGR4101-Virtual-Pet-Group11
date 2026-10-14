
#ifndef PET_UI_H
#define PET_UI_H

#include <stdint.h>
#include "pet_logic.h"

void UI_GraphicsTest(void);

void UI_DrawMainScreen(void);
void UI_DrawMenu(uint8_t selection);
void UI_ReturnToMain(void);

void UI_UpdateHunger(int hunger);
void UI_UpdateHappiness(int happiness);
void UI_UpdateSleep(uint8_t sleep);

void UI_DrawDeathScreen(void);
void UI_DrawRunawayScreen(void);

void UI_HandleInput(void);
void UI_Update(void);

#endif
