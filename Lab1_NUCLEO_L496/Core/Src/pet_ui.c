
#include "pet_ui.h"
#include "ili9341.h"
#include "gfx01m2_conf.h"

// Screen states
typedef enum {
    SCREEN_MAIN,
    SCREEN_MENU
} UI_Screen;

// Joystick button identifiers
#define BTN_UP      0x01
#define BTN_DOWN    0x02
#define BTN_LEFT    0x04
#define BTN_CENTER  0x08

#define DEBOUNCE_MS 30U

// Current screen and menu selection
static UI_Screen currentScreen = SCREEN_MAIN;
static uint8_t menuSelection = 0;

// Selected action for Member 3
static UI_Action pendingAction = UI_ACTION_NONE;

// Stored pet values
static uint8_t previousHunger = 100;
static uint8_t previousHappiness = 100;

// Joystick debounce variables
static uint8_t lastRawButtons = 0;
static uint8_t stableButtons = 0;
static uint8_t inputArmed = 1;
static uint32_t lastInputTime = 0;

// Menu options
static const char *menuOptions[3] = {
    "FEED",
    "PLAY",
    "ACTION 3"
};


// ---------------------------------------
// Graphics Test
// ---------------------------------------

void UI_GraphicsTest(void)
{
    // Clear display
    ILI9341_FillScreen(ILI9341_COLOR_BLACK);

    // Display title
    ILI9341_DrawString(20, 20, "GRAPHICS TEST",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 2);

    // Colored rectangles
    ILI9341_FillRect(20, 70, 50, 40,
                     ILI9341_COLOR_RED);

    ILI9341_FillRect(90, 70, 50, 40,
                     ILI9341_COLOR_GREEN);

    ILI9341_FillRect(160, 70, 50, 40,
                     ILI9341_COLOR_BLUE);

    // Outlined rectangle
    ILI9341_DrawRect(20, 140, 190, 60,
                     ILI9341_COLOR_YELLOW);

    // Diagonal lines
    ILI9341_DrawLine(20, 230, 210, 280,
                     ILI9341_COLOR_CYAN);

    ILI9341_DrawLine(210, 230, 20, 280,
                     ILI9341_COLOR_MAGENTA);
}


// ---------------------------------------
// Draw Status Meter
// ---------------------------------------

static void UI_DrawMeter(uint16_t y,
                         uint8_t value,
                         uint16_t color)
{
    if (value > 100)
        value = 100;

    // Clear only the meter region
    ILI9341_FillRect(20, y, 200, 15,
                     ILI9341_COLOR_BLACK);

    // Meter border
    ILI9341_DrawRect(20, y, 200, 15,
                     ILI9341_COLOR_WHITE);

    // Calculate filled width
    uint16_t width = (uint16_t)value * 196 / 100;

    // Draw filled portion if nonzero
    if (width > 0)
    {
        ILI9341_FillRect(22, y + 2,
                         width, 11, color);
    }
}


// ---------------------------------------
// Main Pet Screen
// ---------------------------------------

void UI_DrawMainScreen(void)
{
    currentScreen = SCREEN_MAIN;

    // Clear display
    ILI9341_FillScreen(ILI9341_COLOR_BLACK);

    // Main title
    ILI9341_DrawString(45, 15, "VIRTUAL PET",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 2);

    // Pet display area
    ILI9341_DrawRect(20, 55, 200, 135,
                     ILI9341_COLOR_CYAN);

    // Placeholder for Member 2 artwork
    ILI9341_DrawString(75, 110, "PET",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 2);

    // Hunger label
    ILI9341_DrawString(20, 210, "HUNGER",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    // Happiness label
    ILI9341_DrawString(20, 255, "HAPPINESS",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);

    // Draw saved status values
    UI_DrawMeter(225, previousHunger,
                 ILI9341_COLOR_GREEN);

    UI_DrawMeter(270, previousHappiness,
                 ILI9341_COLOR_YELLOW);

    // Menu instructions
    ILI9341_DrawString(20, 300, "CENTER: MENU",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);
}


// ---------------------------------------
// Update Hunger Meter
// ---------------------------------------

void UI_UpdateHunger(uint8_t hunger)
{
    if (hunger > 100)
        hunger = 100;

    // Skip if value is unchanged
    if (hunger == previousHunger)
        return;

    // Store current value
    previousHunger = hunger;

    // Only draw if main screen is visible
    if (currentScreen == SCREEN_MAIN)
    {
        UI_DrawMeter(225, hunger,
                     ILI9341_COLOR_GREEN);
    }
}


// ---------------------------------------
// Update Happiness Meter
// ---------------------------------------

void UI_UpdateHappiness(uint8_t happiness)
{
    if (happiness > 100)
        happiness = 100;

    // Skip if value is unchanged
    if (happiness == previousHappiness)
        return;

    // Store current value
    previousHappiness = happiness;

    // Only draw if main screen is visible
    if (currentScreen == SCREEN_MAIN)
    {
        UI_DrawMeter(270, happiness,
                     ILI9341_COLOR_YELLOW);
    }
}


// ---------------------------------------
// Draw One Menu Option
// ---------------------------------------

static void UI_DrawMenuOption(uint8_t index,
                              uint8_t selected)
{
    uint16_t y = 90 + index * 55;

    // Clear only this option's region
    ILI9341_FillRect(20, y, 200, 40,
                     ILI9341_COLOR_BLACK);

    if (selected)
    {
        // Highlight selected option
        ILI9341_FillRect(20, y, 200, 40,
                         ILI9341_COLOR_CYAN);

        ILI9341_DrawString(45, y + 12,
                           menuOptions[index],
                           ILI9341_COLOR_BLACK,
                           ILI9341_COLOR_CYAN, 2);
    }
    else
    {
        // Draw unselected option
        ILI9341_DrawRect(20, y, 200, 40,
                         ILI9341_COLOR_WHITE);

        ILI9341_DrawString(45, y + 12,
                           menuOptions[index],
                           ILI9341_COLOR_WHITE,
                           ILI9341_COLOR_BLACK, 2);
    }
}


// ---------------------------------------
// Draw Full Menu Screen
// ---------------------------------------

void UI_DrawMenu(uint8_t selection)
{
    if (selection > 2)
        selection = 0;

    currentScreen = SCREEN_MENU;
    menuSelection = selection;

    // Full redraw only when entering menu
    ILI9341_FillScreen(ILI9341_COLOR_BLACK);

    // Menu heading
    ILI9341_DrawString(55, 25, "PET MENU",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 2);

    // Draw all three options
    for (uint8_t i = 0; i < 3; i++)
    {
        UI_DrawMenuOption(i, i == menuSelection);
    }

    // Instructions
    ILI9341_DrawString(25, 285, "CENTER: SELECT",
                       ILI9341_COLOR_WHITE,
                       ILI9341_COLOR_BLACK, 1);
}


// ---------------------------------------
// Change Menu Selection
// ---------------------------------------

static void UI_ChangeSelection(uint8_t newSelection)
{
    if (newSelection > 2)
        return;

    if (newSelection == menuSelection)
        return;

    uint8_t oldSelection = menuSelection;

    menuSelection = newSelection;

    // Redraw only the previous selection
    UI_DrawMenuOption(oldSelection, 0);

    // Redraw only the new selection
    UI_DrawMenuOption(menuSelection, 1);
}


// ---------------------------------------
// Return to Main Screen
// ---------------------------------------

void UI_ReturnToMain(void)
{
    currentScreen = SCREEN_MAIN;

    // Redraw using saved hunger/happiness
    UI_DrawMainScreen();
}


// ---------------------------------------
// Read Joystick Buttons
// ---------------------------------------

static uint8_t UI_ReadJoystick(void)
{
    uint8_t buttons = 0;

    // Joystick signals are active low
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


// ---------------------------------------
// Process a Single Button Press
// ---------------------------------------

static void UI_ProcessButton(uint8_t button)
{
    if (currentScreen == SCREEN_MAIN)
    {
        if (button == BTN_CENTER)
        {
            // Open menu
            UI_DrawMenu(0);
        }
    }
    else if (currentScreen == SCREEN_MENU)
    {
        if (button == BTN_UP)
        {
            // Move selection up
            uint8_t next = (menuSelection + 2) % 3;
            UI_ChangeSelection(next);
        }
        else if (button == BTN_DOWN)
        {
            // Move selection down
            uint8_t next = (menuSelection + 1) % 3;
            UI_ChangeSelection(next);
        }
        else if (button == BTN_CENTER)
        {
            // Save the selected action
            if (menuSelection == 0)
                pendingAction = UI_ACTION_FEED;
            else if (menuSelection == 1)
                pendingAction = UI_ACTION_PLAY;
            else
                pendingAction = UI_ACTION_THIRD;

            // Return to main screen
            UI_ReturnToMain();
        }
        else if (button == BTN_LEFT)
        {
            // Exit menu without selecting
            UI_ReturnToMain();
        }
    }
}


// ---------------------------------------
// Handle Joystick Input
// ---------------------------------------

void UI_HandleInput(void)
{
    uint32_t currentTime = HAL_GetTick();
    uint8_t rawButtons = UI_ReadJoystick();

    // Detect a change in raw input
    if (rawButtons != lastRawButtons)
    {
        lastRawButtons = rawButtons;
        lastInputTime = currentTime;
    }

    // Wait until input has stabilized
    if ((uint32_t)(currentTime - lastInputTime)
            < DEBOUNCE_MS)
    {
        return;
    }

    // No new stable input
    if (rawButtons == stableButtons)
        return;

    stableButtons = rawButtons;

    // All buttons released: allow next press
    if (stableButtons == 0)
    {
        inputArmed = 1;
        return;
    }

    // Process only one press per release
    if (inputArmed)
    {
        inputArmed = 0;

        // Only individual buttons are handled
        UI_ProcessButton(stableButtons);
    }
}


// ---------------------------------------
// Retrieve Selected Action
// ---------------------------------------

UI_Action UI_GetAction(void)
{
    UI_Action action = pendingAction;

    // Clear pending action after reading
    pendingAction = UI_ACTION_NONE;

    return action;
}


// ---------------------------------------
// Main UI Update Function
// ---------------------------------------

void UI_Update(void)
{
    // Poll joystick without blocking
    UI_HandleInput();
}
