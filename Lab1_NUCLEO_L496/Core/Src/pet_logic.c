
#include "pet_logic.h"
#include "main.h"
#include <stdint.h>

#define DECAY_INTERVAL 10000U

PetState pet_state;

static uint32_t lastDecayTime = 0;
static int happiness;
static int hunger;
static int energy;

static void decay(void);
static void CheckValues(void);
static void CapValues(void);

void Pet_Init(void)
{
    pet_state = IDLE;
    happiness = 100;
    hunger = 100;
    energy = 100;

    lastDecayTime = HAL_GetTick();
}

void updatePetState(PetState newState)
{
    if (pet_state == DEAD || pet_state == RUN_AWAY)
        return;

    decay();

    if (pet_state == DEAD || pet_state == RUN_AWAY)
        return;

    if (newState != pet_state)
    {
        pet_state = newState;

        switch (pet_state)
        {
            case FEED:
                pet_feed();
                break;

            case PLAY:
                pet_play();
                break;

            case SLEEP:
                pet_sleep();
                break;

            default:
                break;
        }
    }
}

void pet_idle(void)
{
    // Idle behavior
}

void pet_menu(void)
{
    // Menu behavior
}

void pet_dead(void)
{
    // Dead state
}

void pet_run_away(void)
{
    // Runaway state
}

void pet_feed(void)
{
    hunger += 20;

    CapValues();
    CheckValues();
}

void pet_play(void)
{
    happiness += 20;
    energy -= 10;

    CapValues();
    CheckValues();
}

void pet_sleep(void)
{
    hunger -= 25;
    happiness += 10;
    energy += 30;

    CapValues();
    CheckValues();
}

static void decay(void)
{
    uint32_t currentTime = HAL_GetTick();

    if ((uint32_t)(currentTime - lastDecayTime)
            >= DECAY_INTERVAL)
    {
        hunger -= 5;
        happiness -= 3;
        energy -= 4;

        CapValues();
        CheckValues();

        lastDecayTime = currentTime;
    }
}

static void CheckValues(void)
{
    if (hunger <= 0)
    {
        pet_state = DEAD;
    }
    else if (happiness <= 0)
    {
        pet_state = RUN_AWAY;
    }
}

static void CapValues(void)
{
    if (hunger > 100)
        hunger = 100;
    else if (hunger < 0)
        hunger = 0;

    if (happiness > 100)
        happiness = 100;
    else if (happiness < 0)
        happiness = 0;

    if (energy > 100)
        energy = 100;
    else if (energy < 0)
        energy = 0;
}

void setPetState(PetState newState)
{
    if (pet_state != DEAD && pet_state != RUN_AWAY)
    {
        if (newState == MENU ||
            newState == FEED ||
            newState == PLAY ||
            newState == SLEEP ||
            newState == IDLE)
        {
            pet_state = newState;
        }
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

int getEnergy(void)
{
    return energy;
}