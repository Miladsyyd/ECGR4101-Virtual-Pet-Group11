#include "pet_logic.h"
#include "main.h"
#include <stdint.h>
#define DECAY_INTERVAL 10000U // 10 seconds in milliseconds

PetState pet_state;
static uint32_t lastDecayTime = 0;
static int happiness;
static int hunger;
static void decay(void);
static void CheckValues(void);
static void CapValues(void);

void Pet_Init(void) { //called at the start or when restarting the pet
    pet_state = IDLE;
    happiness = 100;
    hunger = 100;

    lastDecayTime = HAL_GetTick(); // Initialize the last decay time
}

void updatePetState(PetState newState) {
    decay(); // Call the decay function to update hunger and happiness over time

    if (newState != pet_state) {
        
        if (pet_state == DEAD || pet_state == RUN_AWAY){
            return;// If the pet is dead or has run away, it cannot change state
        }

        pet_state = newState; // Update the pet state to the new state

        switch (pet_state) {
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

void pet_idle(void) {
    //idle animation
}

void pet_menu(void) {
    // Logic for menu state, to be implemented
    /*
    can be used to display the menu options and handle user input for selecting actions
    can go to FEED, PLAY or SLEEP
    */
}

void pet_dead(void) {
    // Pet remains dead until the game is restarted
}

void pet_run_away(void) {
    //Pet remains gone until the game is restarted
}

void pet_feed(void){
    hunger += 20;
    CapValues(); // Ensure hunger does not exceed 100

}

void pet_play(void){
    happiness += 20;
    CapValues(); // Ensure happiness does not exceed 100

}

void pet_sleep(void){
    hunger -= 25;
    happiness += 10;

    CheckValues(); // Check if hunger or happiness has reached 0
    CapValues(); // Ensure hunger and happiness do not exceed 100

}

static void decay(void) {
    uint32_t currentTime = HAL_GetTick();

    if (currentTime - lastDecayTime >= DECAY_INTERVAL) {
        hunger -= 5; // Decrease hunger by 5
        happiness -= 3; // Decrease happiness by 3

        CapValues(); // Ensure hunger and happiness do not exceed 100
        CheckValues(); // Check if hunger or happiness has reached 0

        lastDecayTime = currentTime; // Update the last decay time
    }
}


static void CheckValues(void) {
    if (hunger <= 0) {
        pet_state = DEAD; // Pet dies if hunger reaches 0
    } else if (happiness <= 0) {
        pet_state = RUN_AWAY; // Pet runs away if happiness reaches 0
    }
}

static void CapValues(void) {
    if (hunger > 100) {
        hunger = 100; // Cap hunger at 100
    }else if (hunger < 0) {
        hunger = 0; // Ensure hunger does not go below 0
    }

    if (happiness > 100) {
        happiness = 100; // Cap happiness at 100
    }else if (happiness < 0) {
        happiness = 0; // Ensure happiness does not go below 0
    }
}

void setPetState(PetState newState) {
    if (pet_state != DEAD && pet_state != RUN_AWAY) {
        if (newState == MENU || newState == FEED || newState == PLAY || newState == SLEEP || newState == IDLE) {
            pet_state = newState;
        }
    }
}

PetState getPetState(void) {
    return pet_state;
}

int getHappiness(void) {
    return happiness;
}

int getHunger(void) {
    return hunger;
}
