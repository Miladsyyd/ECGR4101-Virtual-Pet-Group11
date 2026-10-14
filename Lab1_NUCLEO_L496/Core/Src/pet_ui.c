
#include "pet_ui.h"
#include "ili9341.h"
#include "gfx01m2_conf.h"

typedef enum {
    SCREEN_MAIN,
    SCREEN_MENU
} UI_Screen;

#define BTN_UP      0x01
#define BTN_DOWN    0x02
#define BTN_LEFT    0x04
#define BTN_CENTER  0x08

#define DEBOUNCE_MS 30U

static UI_Screen currentScreen = SCREEN_MAIN;
static uint8_t menuSelection = 0;

static UI_Action pendingAction = UI_ACTION_NONE;

static uint8_t feedValue = 100;
static uint8_t playValue = 100;
static uint8_t sleepValue = 100;

static uint8_t lastRawButtons = 0;
static uint8_t stableButtons = 0;
static uint8_t inputArmed = 1;
static uint32_t lastInputTime = 0;

static const char *menuOptions[3] = {
    "FEED",
    "PLAY",
    "SLEEP"
};


// Graphics test

void UI_GraphicsTest(void)
{
    ILI9341_FillScreen(ILI9341_COLOR_BLACK);

    ILI9341_DrawString(20, 20, "GRAPHICS TEST",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_FillRect(20, 70, 50, 40,
                     ILI9341_COLOR_RED);

    ILI9341_FillRect(90, 70, 50, 40,
                     ILI9341_COLOR_GREEN);

    ILI9341_FillRect(160, 70, 50, 40,
                     ILI9341_COLOR_BLUE);

    ILI9341_DrawRect(20, 140, 190, 60,
                     ILI9341_COLOR_YELLOW);

    ILI9341_DrawLine(20, 230, 210, 280,
                     ILI9341_COLOR_CYAN);

    ILI9341_DrawLine(210, 230, 20, 280,
                     ILI9341_COLOR_MAGENTA);
}


// Draw status bar

static void UI_DrawBar(uint16_t y,
                       uint8_t value,
                       uint16_t color)
{
    if (value > 100)
        value = 100;

    ILI9341_FillRect(20, y, 200, 10,
                     ILI9341_COLOR_BLACK);

    ILI9341_DrawRect(20, y, 200, 10,
                     ILI9341_COLOR_WHITE);

    uint16_t width = (uint16_t)value * 196 / 100;

    if (width > 0)
    {
        ILI9341_FillRect(22, y + 2,
                         width, 6, color);
    }
}


// Main pet screen

void UI_DrawMainScreen(void)
{
    currentScreen = SCREEN_MAIN;

    ILI9341_FillScreen(ILI9341_COLOR_BLACK);

    ILI9341_DrawString(87, 10, "VIRTUAL PET",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    // Sprout display area
    ILI9341_DrawRect(20, 35, 200, 145,
                     ILI9341_COLOR_CYAN);

    // Placeholder for Member 2's artwork
    ILI9341_DrawString(102, 105, "SPROUT",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_DrawString(20, 190, "FEED",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    UI_DrawBar(203, feedValue, ILI9341_COLOR_GREEN);

    ILI9341_DrawString(20, 225, "PLAY",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    UI_DrawBar(238, playValue, ILI9341_COLOR_YELLOW);

    ILI9341_DrawString(20, 260, "SLEEP",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    UI_DrawBar(273, sleepValue, ILI9341_COLOR_BLUE);

    ILI9341_DrawString(20, 300, "CENTER: MENU",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);
}


// Feed status

void UI_UpdateFeed(uint8_t feed)
{
    if (feed > 100)
        feed = 100;

    if (feed == feedValue)
        return;

    feedValue = feed;

    if (currentScreen == SCREEN_MAIN)
    {
        UI_DrawBar(203, feedValue,
                   ILI9341_COLOR_GREEN);
    }
}


// Play status

void UI_UpdatePlay(uint8_t play)
{
    if (play > 100)
        play = 100;

    if (play == playValue)
        return;

    playValue = play;

    if (currentScreen == SCREEN_MAIN)
    {
        UI_DrawBar(238, playValue,
                   ILI9341_COLOR_YELLOW);
    }
}


// Sleep status

void UI_UpdateSleep(uint8_t sleep)
{
    if (sleep > 100)
        sleep = 100;

    if (sleep == sleepValue)
        return;

    sleepValue = sleep;

    if (currentScreen == SCREEN_MAIN)
    {
        UI_DrawBar(273, sleepValue,
                   ILI9341_COLOR_BLUE);
    }
}


// Draw one menu option

static void UI_DrawMenuOption(uint8_t index,
                              uint8_t selected)
{
    uint16_t y = 90 + index * 55;

    ILI9341_FillRect(20, y, 200, 40,
                     ILI9341_COLOR_BLACK);

    if (selected)
    {
        ILI9341_FillRect(20, y, 200, 40,
                         ILI9341_COLOR_CYAN);

        ILI9341_DrawString(75, y + 16,
                           menuOptions[index],
                           ILI9341_COLOR_BLACK,
                           ILI9341_COLOR_CYAN, 1);
    }
    else
    {
        ILI9341_DrawRect(20, y, 200, 40,
                         ILI9341_COLOR_WHITE);

        ILI9341_DrawString(75, y + 16,
                           menuOptions[index],
                           ILI9341_COLOR_WHITE,
                           ILI9341_COLOR_BLACK, 1);
    }
}


// Menu screen

void UI_DrawMenu(uint8_t selection)
{
    if (selection > 2)
        selection = 0;

    currentScreen = SCREEN_MENU;
    menuSelection = selection;

    ILI9341_FillScreen(ILI9341_COLOR_BLACK);

    ILI9341_DrawString(96, 25, "PET MENU",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    for (uint8_t i = 0; i < 3; i++)
    {
        UI_DrawMenuOption(i, i == menuSelection);
    }

    ILI9341_DrawString(25, 285, "CENTER: SELECT",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);
}


// Change menu selection

static void UI_ChangeSelection(uint8_t newSelection)
{
    if (newSelection > 2)
        return;

    if (newSelection == menuSelection)
        return;

    uint8_t oldSelection = menuSelection;

    menuSelection = newSelection;

    UI_DrawMenuOption(oldSelection, 0);
    UI_DrawMenuOption(menuSelection, 1);
}


// Return to main screen

void UI_ReturnToMain(void)
{
    UI_DrawMainScreen();
}


// Read joystick

static uint8_t UI_ReadJoystick(void)
{
    uint8_t buttons = 0;

    if (HAL_GPIO_ReadPin(JOY_UP_GPIO_Port,
                         JOY_UP_Pin) == GPIO_PIN_RESET)
    {
        buttons |= BTN_UP;
    }

    if (HAL_GPIO_ReadPin(JOY_DOWN_GPIO_Port,
                         JOY_DOWN_Pin) == GPIO_PIN_RESET)
    {
        buttons |= BTN_DOWN;
    }

    if (HAL_GPIO_ReadPin(JOY_LEFT_GPIO_Port,
                         JOY_LEFT_Pin) == GPIO_PIN_RESET)
    {
        buttons |= BTN_LEFT;
    }

    if (HAL_GPIO_ReadPin(JOY_CENTER_GPIO_Port,
                         JOY_CENTER_Pin) == GPIO_PIN_RESET)
    {
        buttons |= BTN_CENTER;
    }

    return buttons;
}


// Process button input

static void UI_ProcessButton(uint8_t button)
{
    if (currentScreen == SCREEN_MAIN)
    {
        if (button == BTN_CENTER)
        {
            UI_DrawMenu(0);
        }
    }
    else if (currentScreen == SCREEN_MENU)
    {
        if (button == BTN_UP)
        {
            uint8_t next = (menuSelection + 2) % 3;
            UI_ChangeSelection(next);
        }
        else if (button == BTN_DOWN)
        {
            uint8_t next = (menuSelection + 1) % 3;
            UI_ChangeSelection(next);
        }
        else if (button == BTN_CENTER)
        {
            if (menuSelection == 0)
                pendingAction = UI_ACTION_FEED;
            else if (menuSelection == 1)
                pendingAction = UI_ACTION_PLAY;
            else
                pendingAction = UI_ACTION_SLEEP;

            UI_ReturnToMain();
        }
        else if (button == BTN_LEFT)
        {
            UI_ReturnToMain();
        }
    }
}


// Handle joystick input

void UI_HandleInput(void)
{
    uint32_t currentTime = HAL_GetTick();
    uint8_t rawButtons = UI_ReadJoystick();

    if (rawButtons != lastRawButtons)
    {
        lastRawButtons = rawButtons;
        lastInputTime = currentTime;
    }

    if ((uint32_t)(currentTime - lastInputTime)
            < DEBOUNCE_MS)
    {
        return;
    }

    if (rawButtons == stableButtons)
        return;

    stableButtons = rawButtons;

    if (stableButtons == 0)
    {
        inputArmed = 1;
        return;
    }

    if (inputArmed)
    {
        inputArmed = 0;
        UI_ProcessButton(stableButtons);
    }
}


// Retrieve selected action

UI_Action UI_GetAction(void)
{
    UI_Action action = pendingAction;
    pendingAction = UI_ACTION_NONE;

    return action;
}


// Update interface

void UI_Update(void)
{
    UI_HandleInput();
}
