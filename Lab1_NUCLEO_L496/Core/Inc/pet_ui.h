
#ifndef PET_UI_H
#define PET_UI_H

#include <stdint.h>

// Actions sent to pet logic
typedef enum {
    UI_ACTION_NONE,
    UI_ACTION_FEED,
    UI_ACTION_PLAY,
    UI_ACTION_THIRD
} UI_Action;

// Graphics test
void UI_GraphicsTest(void);

// Main screen
void UI_DrawMainScreen(void);

// Status meters
void UI_UpdateHunger(uint8_t hunger);
void UI_UpdateHappiness(uint8_t happiness);

// Menu
void UI_DrawMenu(uint8_t selection);
void UI_ReturnToMain(void);

// Joystick handling
void UI_HandleInput(void);

// Action retrieval for Member 3
UI_Action UI_GetAction(void);

// Periodic UI update
void UI_Update(void);

#endif
