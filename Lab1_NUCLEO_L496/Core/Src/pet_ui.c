
#include "pet_ui.h"
#include "ili9341.h"
#include "gfx01m2_conf.h"

#define BTN_UP       0x01
#define BTN_DOWN     0x02
#define BTN_LEFT     0x04
#define BTN_CENTER   0x08

#define DEBOUNCE_MS  30U
#define MENU_COUNT   4U

static uint8_t menuSelection = 0;

static int hungerValue = 100;
static int happinessValue = 100;
static uint8_t sleepValue = 100;

static uint8_t lastRawButtons = 0;
static uint8_t stableButtons = 0;
static uint8_t inputArmed = 1;
static uint32_t lastInputTime = 0;

static PetState displayedState = IDLE;

static const char *menuOptions[MENU_COUNT] = {
    "FEED",
    "PLAY",
    "SLEEP",
    "BACK TO IDLE"
};


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


void UI_DrawMainScreen(void)
{
    displayedState = IDLE;

    hungerValue = getHunger();
    happinessValue = getHappiness();

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

    UI_DrawBar(203, hungerValue,
               ILI9341_COLOR_GREEN);

    ILI9341_DrawString(20, 225, "PLAY",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    UI_DrawBar(238, happinessValue,
               ILI9341_COLOR_YELLOW);

    ILI9341_DrawString(20, 260, "SLEEP",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    UI_DrawBar(273, sleepValue,
               ILI9341_COLOR_BLUE);

    ILI9341_DrawString(20, 300, "CENTER: MENU",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);
}


void UI_UpdateHunger(int hunger)
{
    hunger = UI_LimitValue(hunger);

    if (hunger == hungerValue)
        return;

    hungerValue = hunger;

    if (displayedState == IDLE)
    {
        UI_DrawBar(203, hungerValue,
                   ILI9341_COLOR_GREEN);
    }
}


void UI_UpdateHappiness(int happiness)
{
    happiness = UI_LimitValue(happiness);

    if (happiness == happinessValue)
        return;

    happinessValue = happiness;

    if (displayedState == IDLE)
    {
        UI_DrawBar(238, happinessValue,
                   ILI9341_COLOR_YELLOW);
    }
}


void UI_UpdateSleep(uint8_t sleep)
{
    sleep = UI_LimitValue(sleep);

    if (sleep == sleepValue)
        return;

    sleepValue = sleep;

    if (displayedState == IDLE)
    {
        UI_DrawBar(273, sleepValue,
                   ILI9341_COLOR_BLUE);
    }
}


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


void UI_DrawMenu(uint8_t selection)
{
    if (selection >= MENU_COUNT)
        selection = 0;

    displayedState = MENU;
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


void UI_ReturnToMain(void)
{
    setPetState(IDLE);
    UI_DrawMainScreen();
}


void UI_DrawDeathScreen(void)
{
    displayedState = DEAD;

    ILI9341_FillScreen(ILI9341_COLOR_BLACK);

    ILI9341_DrawString(87, 10, "VIRTUAL PET",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_DrawRect(20, 35, 200, 145,
                     ILI9341_COLOR_RED);

    ILI9341_DrawString(75, 90, "SPROUT HAS",
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


void UI_DrawRunawayScreen(void)
{
    displayedState = RUN_AWAY;

    ILI9341_FillScreen(ILI9341_COLOR_BLACK);

    ILI9341_DrawString(87, 10, "VIRTUAL PET",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    ILI9341_DrawRect(20, 35, 200, 145,
                     ILI9341_COLOR_YELLOW);

    ILI9341_DrawString(75, 90, "SPROUT RAN",
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
            sleepValue = 100;
            UI_DrawMainScreen();
        }
    }
}


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


void UI_Update(void)
{
    // Run the pet's time-based stat decay
    updatePetState(getPetState());

    PetState state = getPetState();

    if (state == DEAD)
    {
        if (displayedState != DEAD)
            UI_DrawDeathScreen();
    }
    else if (state == RUN_AWAY)
    {
        if (displayedState != RUN_AWAY)
            UI_DrawRunawayScreen();
    }
    else if (state == MENU)
    {
        if (displayedState != MENU)
            UI_DrawMenu(menuSelection);
    }
    else if (state == IDLE)
    {
        if (displayedState != IDLE)
            UI_DrawMainScreen();

        UI_UpdateHunger(getHunger());
        UI_UpdateHappiness(getHappiness());
    }

    UI_HandleInput();
}
