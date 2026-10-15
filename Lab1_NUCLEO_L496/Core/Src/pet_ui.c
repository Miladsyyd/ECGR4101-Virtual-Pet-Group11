
#include "pet_ui.h"
#include "ili9341.h"
#include "gfx01m2_conf.h"

#define BTN_UP      0x01
#define BTN_DOWN    0x02
#define BTN_LEFT    0x04
#define BTN_CENTER  0x08
#define BTN_RIGHT   0x10

#define DEBOUNCE_MS 30U
#define MENU_COUNT 4U
#define GAME_MENU_COUNT 3U

typedef enum {
    SCREEN_NONE,
    SCREEN_MAIN,
    SCREEN_MENU,
    SCREEN_GAME_MENU,
    SCREEN_SHAKE,
    SCREEN_RADIO,
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

static const char *gameMenuOptions[GAME_MENU_COUNT] = {
    "SHAKE GAME",
    "RADIO GAME",
    "BACK"
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


// Status bars

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
    uint16_t width = (uint16_t)level * 196U / 100U;

    ILI9341_FillRect(20, y, 200, 10,
                     ILI9341_COLOR_BLACK);

    ILI9341_DrawRect(20, y, 200, 10,
                     ILI9341_COLOR_WHITE);

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

    ILI9341_DrawString(20, 300, "CENTER: MENU R: GAMES",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    UI_UpdateHunger();
    UI_UpdateHappiness();
}


// Hunger and happiness

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


// Menu drawing

static void UI_DrawMenuOption(uint8_t index,
                              uint8_t selected,
                              const char *options[])
{
    uint16_t y = 75 + index * 48;

    ILI9341_FillRect(20, y, 200, 38,
                     ILI9341_COLOR_BLACK);

    if (selected)
    {
        ILI9341_FillRect(20, y, 200, 38,
                         ILI9341_COLOR_CYAN);

        ILI9341_DrawString(55, y + 15,
                           options[index],
                           ILI9341_COLOR_BLACK,
                           ILI9341_COLOR_CYAN, 1);
    }
    else
    {
        ILI9341_DrawRect(20, y, 200, 38,
                         ILI9341_COLOR_WHITE);

        ILI9341_DrawString(55, y + 15,
                           options[index],
                           ILI9341_COLOR_WHITE,
                           ILI9341_COLOR_BLACK, 1);
    }
}

static void UI_ChangeSelection(uint8_t newSelection,
                               uint8_t count,
                               const char *options[])
{
    if (newSelection >= count)
        return;

    if (newSelection == menuSelection)
        return;

    uint8_t oldSelection = menuSelection;
    menuSelection = newSelection;

    UI_DrawMenuOption(oldSelection, 0, options);
    UI_DrawMenuOption(menuSelection, 1, options);
}

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
        UI_DrawMenuOption(i, i == menuSelection,
                          menuOptions);
    }

    ILI9341_DrawString(25, 285, "CENTER: SELECT",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);
}


// Games menu

void UI_DrawGameMenu(uint8_t selection)
{
    if (selection >= GAME_MENU_COUNT)
        selection = 0;

    currentScreen = SCREEN_GAME_MENU;
    menuSelection = selection;

    ILI9341_FillScreen(ILI9341_COLOR_BLACK);

    ILI9341_DrawString(75, 25, "CHOOSE A GAME",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    for (uint8_t i = 0; i < GAME_MENU_COUNT; i++)
    {
        UI_DrawMenuOption(i, i == menuSelection,
                          gameMenuOptions);
    }

    ILI9341_DrawString(25, 285, "CENTER: SELECT",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);
}

void UI_DrawShakeGame(void)
{
    currentScreen = SCREEN_SHAKE;

    ILI9341_FillScreen(ILI9341_COLOR_BLACK);

    ILI9341_DrawString(55, 25, "SHAKE GAME",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_DrawString(35, 140, "SHAKE TO PLAY",
                       ILI9341_COLOR_CYAN,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_DrawString(20, 285, "LEFT: BACK",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);
}

void UI_DrawRadioGame(void)
{
    currentScreen = SCREEN_RADIO;

    ILI9341_FillScreen(ILI9341_COLOR_BLACK);

    ILI9341_DrawString(55, 25, "RADIO GAME",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_DrawString(30, 140, "FIND THE CHANNEL",
                       ILI9341_COLOR_CYAN,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_DrawString(20, 285, "LEFT: BACK",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);
}


// Return to Idle

void UI_ReturnToMain(void)
{
    setPetState(IDLE);

    if (getPetState() == IDLE)
        UI_DrawMainScreen();
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

    ILI9341_DrawString(87, 90, "SPROUT HAS",
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


// Run Away screen

void UI_DrawRunawayScreen(void)
{
    currentScreen = SCREEN_RUNAWAY;

    ILI9341_FillScreen(ILI9341_COLOR_BLACK);

    ILI9341_DrawString(87, 10, "VIRTUAL PET",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_DrawRect(20, 35, 200, 145,
                     ILI9341_COLOR_YELLOW);

    ILI9341_DrawString(87, 90, "SPROUT RAN",
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


// Joystick

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

    if (HAL_GPIO_ReadPin(JOY_RIGHT_GPIO_Port,
                         JOY_RIGHT_Pin) == GPIO_PIN_RESET)
        buttons |= BTN_RIGHT;

    if (HAL_GPIO_ReadPin(JOY_CENTER_GPIO_Port,
                         JOY_CENTER_Pin) == GPIO_PIN_RESET)
        buttons |= BTN_CENTER;

    return buttons;
}


// Process buttons

static void UI_ProcessButton(uint8_t button)
{
    PetState state = getPetState();

    if (state == DEAD || state == RUN_AWAY)
    {
        if (button == BTN_CENTER)
        {
            Pet_Init();
            UI_DrawMainScreen();
        }
        return;
    }

    if (currentScreen == SCREEN_MAIN)
    {
        if (button == BTN_CENTER)
        {
            setPetState(MENU);
            UI_DrawMenu(0);
        }
        else if (button == BTN_RIGHT)
        {
            UI_DrawGameMenu(0);
        }
    }
    else if (currentScreen == SCREEN_MENU)
    {
        if (button == BTN_UP || button == BTN_LEFT)
        {
            UI_ChangeSelection(
                (menuSelection + MENU_COUNT - 1) % MENU_COUNT,
                MENU_COUNT, menuOptions);
        }
        else if (button == BTN_DOWN || button == BTN_RIGHT)
        {
            UI_ChangeSelection(
                (menuSelection + 1) % MENU_COUNT,
                MENU_COUNT, menuOptions);
        }
        else if (button == BTN_CENTER)
        {
            if (menuSelection == 0)
                updatePetState(FEED);
            else if (menuSelection == 1)
                updatePetState(PLAY);
            else if (menuSelection == 2)
                updatePetState(SLEEP);

            if (getPetState() == DEAD ||
                getPetState() == RUN_AWAY)
            {
                return;
            }

            UI_ReturnToMain();
        }
    }
    else if (currentScreen == SCREEN_GAME_MENU)
    {
        if (button == BTN_UP)
        {
            UI_ChangeSelection(
                (menuSelection + GAME_MENU_COUNT - 1)
                % GAME_MENU_COUNT,
                GAME_MENU_COUNT, gameMenuOptions);
        }
        else if (button == BTN_DOWN)
        {
            UI_ChangeSelection(
                (menuSelection + 1) % GAME_MENU_COUNT,
                GAME_MENU_COUNT, gameMenuOptions);
        }
        else if (button == BTN_LEFT)
        {
            UI_ReturnToMain();
        }
        else if (button == BTN_CENTER)
        {
            if (menuSelection == 0)
                UI_DrawShakeGame();
            else if (menuSelection == 1)
                UI_DrawRadioGame();
            else
                UI_ReturnToMain();
        }
    }
    else if (currentScreen == SCREEN_SHAKE ||
             currentScreen == SCREEN_RADIO)
    {
        if (button == BTN_LEFT)
            UI_DrawGameMenu(0);
    }
}


// Debounce input

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


// Update UI

void UI_Update(void)
{
    PetState state = getPetState();

    if (state != DEAD && state != RUN_AWAY)
    {
        updatePetState(state);
        state = getPetState();
    }

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
    else if (state == IDLE)
    {
        if (currentScreen == SCREEN_NONE ||
            currentScreen == SCREEN_DEAD ||
            currentScreen == SCREEN_RUNAWAY)
        {
            UI_DrawMainScreen();
        }

        UI_UpdateHunger();
        UI_UpdateHappiness();
    }
    else if (state == MENU &&
             currentScreen == SCREEN_MAIN)
    {
        UI_DrawMenu(menuSelection);
    }

    UI_HandleInput();
}
