#include "pet_logic.h"
#include "anim.h"
#include "pet_anim.h"
#include "pet_anims.h"
#include "pet_sprites.h"
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
    sleep = 100;

    lastDecayTime = HAL_GetTick(); // Initialize the last decay time
}

void pet_idle(void) {
    animSound(); // Play idle animation and sound
    if (anim_finished) {
        return;
    }
}

void menu(int buttonPressed) {
    displayMenu(); // Display the menu screen

    //do stuff with button presses
    if(buttonPressed == 1) {//right
        pet_state = FEED;
    } else if(buttonPressed == 2) {//left
        pet_state = PLAY;
    } else if(buttonPressed == 3) {//up
        pet_state = SLEEP;
    }else if(buttonPressed == 4) {//down
        pet_state = IDLE;
    }
    return;
}

void pet_feed(void){
    hunger += 20;
    CapValues(); // Ensure hunger does not exceed 100
    animSound(); // Play feeding animation and sound

    if(anim_finshed){
        return; 
    }
    //checkConditions(); // Check if hunger or happiness has reached 0
}

void pet_play(void){
    happiness += 20;
    CapValues(); // Ensure happiness does not exceed 100
    animSound();

    if(anim_finshed){
        return; 
    }
    //checkConditions(); // Check if hunger or happiness has reached 0
}

void pet_sleep(void){
    sleep += 20;
    CapValues(); // Ensure sleep does not exceed 100
    animSound();

    if(anim_finshed){
        displayGames(); // Display the mini-game screen
        if (buttonPressed()) {
            startMiniGame();
            return;
        }
    }
   // checkConditions(); // Check if hunger or happiness has reached 0
}

void death(void) {
    animSound(); // Play death + animation

    if(anim_finshed){
        displayDeath(); // Display the death screen or message and ask user to restart the game pressing any button?
        if (buttonPressed()) {
            Pet_Init(); // Reset the pet state and values
        }
    }
}

void gone(void) {
    animSound(); // Play run away + animation

    if(anim_finshed){
        displayRunAway(); // Display the run away screen or message and ask user to restart the game pressing any button?
        if (buttonPressed()) {
            Pet_Init(); // Reset the pet state and values
        }
    }
}

static void animSound(void) {
    switch (pet_state) {
        case DEATH:
            Anim_Play(ANIM_DEATH);
            //call sound effect function for death here

            break;
        case RUN_AWAY:
            Anim_Play(ANIM_RUNAWAY);
            //add sound func

            break;
        case FEED:
            Anim_Play(ANIM_FEED);
            //add sound func

            break;
        case PLAY:
            Anim_Play(ANIM_PLAY);
            //add sound func

            break;
        case SLEEP:
            Anim_Play(ANIM_ACTION3);
            //add sound func

            break;
        case IDLE:
            Anim_Play(ANIM_IDLE);
            //add sound func

            break;
        default:
            break;
    }
}


static void decay(void) {
    uint32_t currentTime = HAL_GetTick();

    if (currentTime - lastDecayTime >= DECAY_INTERVAL) {
        hunger -= 5; // Decrease hunger by 5
        happiness -= 3; // Decrease happiness by 3

        CapValues(); // Ensure hunger and happiness do not exceed 100
       // checkConditions(); // Check if hunger or happiness has reached 0

        lastDecayTime = currentTime; // Update the last decay time
    }
}


static void checkConditions(void) {
    if (hunger <= 0) {
        pet_state = DEAD; // Set pet state to DEAD
        death(); // Call the death function to handle pet death
    } else if (happiness <= 0) {
        pet_state = RUN_AWAY; // Set pet state to RUN_AWAY
        gone();
    }else{
        return;
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

    if (sleep > 100) {
        sleep = 100; // Cap sleep at 100
    }else if (sleep < 0) {
        sleep = 0; // Ensure sleep does not go below 0
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
