#include "pet_logic.h"

#include "anim.h"
#include "main.h"
#include "pet_ui.h"
#include "sound.h"

#define DECAY_INTERVAL_MS 10000U

static PetState pet_state = IDLE;
static uint32_t lastDecayTime;
static uint8_t previousButtons;
static uint8_t menuSelection;
static uint8_t stateEntered;
static PetState enteredState;
static int happiness;
static int hunger;
static int sleep;

static void decay(void);
static void capValues(void);
static void enterCurrentState(void);
static void checkConditions(void);

void Pet_Init(void)
{
    pet_state = IDLE;
    happiness = 100;
    hunger = 100;
    sleep = 100;
    lastDecayTime = HAL_GetTick();
    previousButtons = 0U;
    menuSelection = 0U;
    stateEntered = 0U;
}

void Pet_Update(uint8_t buttons)
{
    uint8_t pressed = (uint8_t)(buttons & (uint8_t)~previousButtons);
    previousButtons = buttons;

    decay();
    checkConditions();

    if ((stateEntered == 0U) || (enteredState != pet_state))
    {
        enterCurrentState();
    }

    switch (pet_state)
    {
        case IDLE:
            if ((pressed & PET_BUTTON_CENTER) != 0U)
            {
                menuSelection = 0U;
                pet_state = MENU;
            }
            break;

        case MENU:
            if ((pressed & (PET_BUTTON_UP | PET_BUTTON_LEFT)) != 0U)
            {
                menuSelection = (uint8_t)((menuSelection + 3U) % 4U);
                UI_DrawMenu(menuSelection);
            }
            else if ((pressed & (PET_BUTTON_DOWN | PET_BUTTON_RIGHT)) != 0U)
            {
                menuSelection = (uint8_t)((menuSelection + 1U) % 4U);
                UI_DrawMenu(menuSelection);
            }
            else if ((pressed & PET_BUTTON_CENTER) != 0U)
            {
                switch (menuSelection)
                {
                    case 0U:
                        pet_state = FEED;
                        break;
                    case 1U:
                        pet_state = PLAY;
                        break;
                    case 2U:
                        pet_state = SLEEP;
                        break;
                    default:
                        pet_state = IDLE;
                        break;
                }
            }
            break;

        case FEED:
            if (Anim_IsFinished())
            {
                pet_state = IDLE;
            }
            break;

        case PLAY:
            if (Anim_IsFinished())
            {
                pet_state = PLAY_GAME_SELECT;
            }
            break;

        case SLEEP:
            if (Anim_IsFinished())
            {
                pet_state = IDLE;
            }
            break;

        case PLAY_GAME_SELECT:
            if ((pressed & (PET_BUTTON_UP | PET_BUTTON_LEFT)) != 0U)
            {
                menuSelection = (uint8_t)((menuSelection + 2U) % 3U);
                UI_DrawGameMenu(menuSelection);
            }
            else if ((pressed & (PET_BUTTON_DOWN | PET_BUTTON_RIGHT)) != 0U)
            {
                menuSelection = (uint8_t)((menuSelection + 1U) % 3U);
                UI_DrawGameMenu(menuSelection);
            }
            else if ((pressed & PET_BUTTON_CENTER) != 0U)
            {
                switch (menuSelection)
                {
                    case 0U:
                        pet_state = SHAKE_GAME;
                        break;
                    case 1U:
                        pet_state = RADIO_GAME;
                        break;
                    default:
                        pet_state = IDLE;
                        break;
                }
            }
            break;

        case SHAKE_GAME:
        case RADIO_GAME:
            break;

        case DEAD:
        case RUN_AWAY:
            if (pressed != 0U)
            {
                Sound_Stop();
                Pet_Init();
                previousButtons = buttons;
            }
            break;

        default:
            pet_state = IDLE;
            break;
    }
}

void Pet_GameComplete(void)
{
    if ((pet_state == SHAKE_GAME) || (pet_state == RADIO_GAME))
    {
        pet_play();
        UI_UpdateHappiness();
        pet_state = IDLE;
    }
}

static void enterCurrentState(void)
{
    enteredState = pet_state;
    stateEntered = 1U;

    switch (pet_state)
    {
        case IDLE:
            Anim_Pause(false);
            Anim_Play(ANIM_IDLE);
            UI_ReturnToMain();
            break;

        case MENU:
            Anim_Pause(true);
            UI_DrawMenu(menuSelection);
            break;

        case FEED:
            Anim_Pause(false);
            UI_ReturnToMain();
            pet_feed();
            Anim_Play(ANIM_FEED);
            Sound_Play(SOUND_HAPPY);
            UI_UpdateHunger();
            break;

        case PLAY:
            Anim_Pause(false);
            UI_ReturnToMain();
            Anim_Play(ANIM_PLAY);
            Sound_Play(SOUND_HAPPY);
            break;

        case SLEEP:
            Anim_Pause(false);
            UI_ReturnToMain();
            pet_sleep();
            Anim_Play(ANIM_ACTION3);
            Sound_Play(SOUND_HAPPY);
            break;

        case PLAY_GAME_SELECT:
            Anim_Pause(true);
            menuSelection = 0U;
            UI_DrawGameMenu(menuSelection);
            break;

        case SHAKE_GAME:
            Anim_Pause(true);
            UI_DrawShakeGame();
            break;

        case RADIO_GAME:
            Anim_Pause(true);
            UI_DrawRadioGame();
            break;

        case DEAD:
            Anim_Pause(false);
            Anim_Play(ANIM_DEATH);
            Sound_Play(SOUND_DYING);
            UI_DrawDeathScreen();
            break;

        case RUN_AWAY:
            Anim_Pause(false);
            Anim_Play(ANIM_RUNAWAY);
            UI_DrawRunawayScreen();
            break;

        default:
            pet_state = IDLE;
            stateEntered = 0U;
            break;
    }
}

static void decay(void)
{
    uint32_t currentTime = HAL_GetTick();

    if ((uint32_t)(currentTime - lastDecayTime) >= DECAY_INTERVAL_MS)
    {
        int oldHunger = hunger;
        int oldHappiness = happiness;

        hunger -= 5;
        happiness -= 3;
        capValues();
        lastDecayTime = currentTime;

        if ((pet_state != MENU) && (pet_state != DEAD) &&
            (pet_state != RUN_AWAY) && (hunger != oldHunger))
        {
            UI_UpdateHunger();
        }
        if ((pet_state != MENU) && (pet_state != DEAD) &&
            (pet_state != RUN_AWAY) && (happiness != oldHappiness))
        {
            UI_UpdateHappiness();
        }
    }
}

static void checkConditions(void)
{
    if ((pet_state != DEAD) && (pet_state != RUN_AWAY))
    {
        if (hunger == 0)
        {
            pet_state = DEAD;
        }
        else if (happiness == 0)
        {
            pet_state = RUN_AWAY;
        }
    }
}

static void capValues(void)
{
    if (hunger > 100)
    {
        hunger = 100;
    }
    else if (hunger < 0)
    {
        hunger = 0;
    }

    if (happiness > 100)
    {
        happiness = 100;
    }
    else if (happiness < 0)
    {
        happiness = 0;
    }

    if (sleep > 100)
    {
        sleep = 100;
    }
    else if (sleep < 0)
    {
        sleep = 0;
    }
}

void pet_feed(void)
{
    hunger += 20;
    capValues();
}

void pet_play(void)
{
    happiness += 20;
    capValues();
}

void pet_sleep(void)
{
    sleep += 20;
    capValues();
}

void setPetState(PetState newState)
{
    if ((pet_state != DEAD) && (pet_state != RUN_AWAY) &&
        (newState >= IDLE) && (newState <= SLEEP))
    {
        pet_state = newState;
    }
}

PetState getPetState(void)
{
    return pet_state;
}

int getHappiness(void)
{
    return happiness;
}

int getHunger(void)
{
    return hunger;
}

int getSleep(void)
{
    return sleep;
}
