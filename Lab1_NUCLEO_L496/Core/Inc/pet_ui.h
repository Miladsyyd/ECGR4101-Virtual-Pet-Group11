
#ifndef PET_UI_H
#define PET_UI_H

#include <stdint.h>

typedef enum
{
    UI_ACTION_NONE,
    UI_ACTION_FEED,
    UI_ACTION_PLAY,
    UI_ACTION_SLEEP,
    UI_ACTION_BACK,
    UI_ACTION_RESTART,
    UI_ACTION_SHAKE_GAME,
    UI_ACTION_RADIO_GAME
} UI_Action;

void UI_GraphicsTest(void);
uint8_t UI_IsMainScreen(void);

void UI_DrawMainScreen(void);
void UI_DrawMenu(uint8_t selection);
void UI_DrawGameMenu(uint8_t selection);
void UI_DrawShakeGame(void);
void UI_DrawRadioGame(void);
void UI_ReturnToMain(void);

void UI_UpdateFeed(uint8_t feed);
void UI_UpdatePlay(uint8_t play);
void UI_UpdateSleep(uint8_t sleep);

void UI_DrawDeathScreen(void);
void UI_DrawRunawayScreen(void);

void UI_Update(void);
UI_Action UI_GetAction(void);

#endif
