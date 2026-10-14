
#include "pet_ui.h"
#include "ili9341.h"
#include "gfx01m2_conf.h"

#define BTN_UP       0x01
#define BTN_DOWN     0x02
#define BTN_LEFT     0x04
#define BTN_CENTER   0x08

#define DEBOUNCE_MS  30U
#define MENU_COUNT   4U

typedef enum {
    SCREEN_NONE,
    SCREEN_MAIN,
    SCREEN_MENU,
    SCREEN_DEAD,
    SCREEN_RUNAWAY
} UI_Screen;

static UI_Screen currentScreen = SCREEN_NONE;
static uint8_t menuSelection = 0;

static int displayedHunger = -1;
static int displayedHappiness = -1;

static uint8_t lastRawButtons = 0;
static uint8_t stableButtons = 0;
static uint8_t inputArmed = 1;
static uint32_t lastInputTime = 0;

static const char *menuOptions[MENU_COUNT] = {
    "FEED",
    "PLAY",
    "SLEEP",
    "BACK TO IDLE"
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


// Status bar

static uint8_t UI_LimitValue(int value)
{
    if (value < 0)
        return 0;

    if (value > 100)
        return 100;

    return (uint8_t)value;
}


static void UI_DrawBar(uint16_t y,
                       int value,
                       uint16_t color)
{
    uint8_t level = UI_LimitValue(value);

    ILI9341_FillRect(20, y, 200, 10,
                     ILI9341_COLOR_BLACK);

    ILI9341_DrawRect(20, y, 200, 10,
                     ILI9341_COLOR_WHITE);

    uint16_t width = (uint16_t)level * 196 / 100;

    if (width > 0)
    {
        ILI9341_FillRect(22, y + 2,
                         width, 6, color);
    }
}


// Main screen

void UI_DrawMainScreen(void)
{
    currentScreen = SCREEN_MAIN;

    displayedHunger = -1;
    displayedHappiness = -1;

    ILI9341_FillScreen(ILI9341_COLOR_BLACK);

    ILI9341_DrawString(87, 10, "VIRTUAL PET",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_DrawRect(20, 35, 200, 145,
                     ILI9341_COLOR_CYAN);

    ILI9341_DrawString(102, 105, "SPROUT",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_DrawString(20, 190, "FEED",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_DrawString(20, 225, "PLAY",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_DrawString(20, 260, "SLEEP",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_DrawRect(20, 273, 200, 10,
                     ILI9341_COLOR_WHITE);

    ILI9341_DrawString(105, 274, "N/A",
                       ILI9341_COLOR_GRAY,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_DrawString(20, 300, "CENTER: MENU",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    UI_UpdateHunger();
    UI_UpdateHappiness();
}


// Hunger status

void UI_UpdateHunger(void)
{
    if (currentScreen != SCREEN_MAIN)
        return;

    int hunger = getHunger();

    if (hunger == displayedHunger)
        return;

    displayedHunger = hunger;

    UI_DrawBar(203, hunger, ILI9341_COLOR_GREEN);
}


// Happiness status

void UI_UpdateHappiness(void)
{
    if (currentScreen != SCREEN_MAIN)
        return;

    int happiness = getHappiness();

    if (happiness == displayedHappiness)
        return;

    displayedHappiness = happiness;

    UI_DrawBar(238, happiness, ILI9341_COLOR_YELLOW);
}


// Menu option

static void UI_DrawMenuOption(uint8_t index,
                              uint8_t selected)
{
    uint16_t y = 75 + index * 48;

    ILI9341_FillRect(20, y, 200, 38,
                     ILI9341_COLOR_BLACK);

    if (selected)
    {
        ILI9341_FillRect(20, y, 200, 38,
                         ILI9341_COLOR_CYAN);

        ILI9341_DrawString(55, y + 15,
                           menuOptions[index],
                           ILI9341_COLOR_BLACK,
                           ILI9341_COLOR_CYAN, 1);
    }
    else
    {
        ILI9341_DrawRect(20, y, 200, 38,
                         ILI9341_COLOR_WHITE);

        ILI9341_DrawString(55, y + 15,
                           menuOptions[index],
                           ILI9341_COLOR_WHITE,
                           ILI9341_COLOR_BLACK, 1);
    }
}


// Menu screen

void UI_DrawMenu(uint8_t selection)
{
    if (selection >= MENU_COUNT)
        selection = 0;

    currentScreen = SCREEN_MENU;
    menuSelection = selection;

    ILI9341_FillScreen(ILI9341_COLOR_BLACK);

    ILI9341_DrawString(96, 25, "PET MENU",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    for (uint8_t i = 0; i < MENU_COUNT; i++)
    {
        UI_DrawMenuOption(i, i == menuSelection);
    }

    ILI9341_DrawString(25, 285, "CENTER: SELECT",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);
}


// Menu selection

static void UI_ChangeSelection(uint8_t newSelection)
{
    if (newSelection >= MENU_COUNT)
        return;

    if (newSelection == menuSelection)
        return;

    uint8_t oldSelection = menuSelection;
    menuSelection = newSelection;

    UI_DrawMenuOption(oldSelection, 0);
    UI_DrawMenuOption(menuSelection, 1);
}


// Return to Idle

void UI_ReturnToMain(void)
{
    setPetState(IDLE);

    if (getPetState() == IDLE)
    {
        UI_DrawMainScreen();
    }
}


// Death screen

void UI_DrawDeathScreen(void)
{
    currentScreen = SCREEN_DEAD;

    ILI9341_FillScreen(ILI9341_COLOR_BLACK);

    ILI9341_DrawString(87, 10, "VIRTUAL PET",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_DrawRect(20, 35, 200, 145,
                     ILI9341_COLOR_RED);

    ILI9341_DrawString(90, 90, "SPROUT HAS",
                       ILI9341_COLOR_RED,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_DrawString(108, 110, "DIED",
                       ILI9341_COLOR_RED,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_DrawString(93, 205, "GAME OVER",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_DrawString(75, 285, "CENTER: RESTART",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);
}


// Runaway screen

void UI_DrawRunawayScreen(void)
{
    currentScreen = SCREEN_RUNAWAY;

    ILI9341_FillScreen(ILI9341_COLOR_BLACK);

    ILI9341_DrawString(87, 10, "VIRTUAL PET",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_DrawRect(20, 35, 200, 145,
                     ILI9341_COLOR_YELLOW);

    ILI9341_DrawString(90, 90, "SPROUT RAN",
                       ILI9341_COLOR_YELLOW,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_DrawString(108, 110, "AWAY",
                       ILI9341_COLOR_YELLOW,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_DrawString(93, 205, "GAME OVER",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_DrawString(75, 285, "CENTER: RESTART",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);
}


// Read joystick

static uint8_t UI_ReadJoystick(void)
{
    uint8_t buttons = 0;

    if (HAL_GPIO_ReadPin(JOY_UP_GPIO_Port,
                         JOY_UP_Pin) == GPIO_PIN_RESET)
        buttons |= BTN_UP;

    if (HAL_GPIO_ReadPin(JOY_DOWN_GPIO_Port,
                         JOY_DOWN_Pin) == GPIO_PIN_RESET)
        buttons |= BTN_DOWN;

    if (HAL_GPIO_ReadPin(JOY_LEFT_GPIO_Port,
                         JOY_LEFT_Pin) == GPIO_PIN_RESET)
        buttons |= BTN_LEFT;

    if (HAL_GPIO_ReadPin(JOY_CENTER_GPIO_Port,
                         JOY_CENTER_Pin) == GPIO_PIN_RESET)
        buttons |= BTN_CENTER;

    return buttons;
}


// Process input

static void UI_ProcessButton(uint8_t button)
{
    PetState state = getPetState();

    if (state == IDLE)
    {
        if (button == BTN_CENTER)
        {
            setPetState(MENU);
            UI_DrawMenu(0);
        }
    }
    else if (state == MENU)
    {
        if (button == BTN_UP)
        {
            UI_ChangeSelection(
                (menuSelection + MENU_COUNT - 1) % MENU_COUNT);
        }
        else if (button == BTN_DOWN)
        {
            UI_ChangeSelection(
                (menuSelection + 1) % MENU_COUNT);
        }
        else if (button == BTN_LEFT)
        {
            UI_ReturnToMain();
        }
        else if (button == BTN_CENTER)
        {
            if (menuSelection == 0)
            {
                updatePetState(FEED);
            }
            else if (menuSelection == 1)
            {
                updatePetState(PLAY);
            }
            else if (menuSelection == 2)
            {
                updatePetState(SLEEP);
            }
            else
            {
                UI_ReturnToMain();
                return;
            }

            if (getPetState() != DEAD &&
                getPetState() != RUN_AWAY)
            {
                UI_ReturnToMain();
            }
        }
    }
    else if (state == DEAD || state == RUN_AWAY)
    {
        if (button == BTN_CENTER)
        {
            Pet_Init();
            UI_DrawMainScreen();
        }
    }
}


// Joystick debounce

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
        return;

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


// UI update

void UI_Update(void)
{
    updatePetState(getPetState());

    PetState state = getPetState();

    if (state == DEAD)
    {
        if (currentScreen != SCREEN_DEAD)
            UI_DrawDeathScreen();
    }
    else if (state == RUN_AWAY)
    {
        if (currentScreen != SCREEN_RUNAWAY)
            UI_DrawRunawayScreen();
    }
    else if (state == MENU)
    {
        if (currentScreen != SCREEN_MENU)
            UI_DrawMenu(menuSelection);
    }
    else if (state == IDLE)
    {
        if (currentScreen != SCREEN_MAIN)
            UI_DrawMainScreen();

        UI_UpdateHunger();
        UI_UpdateHappiness();
    }

    UI_HandleInput();
}
