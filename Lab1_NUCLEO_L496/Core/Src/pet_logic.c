#include "pet_logic.h"

PetState pet_state;
static int happiness;
static int hunger;
static void CheckHungerAndHappiness(void);
static void CapHungerAndHappiness(void);

void Pet_Init(void) { //called at the start or when restarting the pet
    pet_state = IDLE;
    happiness = 100;
    hunger = 100;
}

void Pet_Update(void) {
    switch (pet_state) {
        case IDLE:
            pet_idle();
            break;
        case MENU:
            pet_menu();
            break;
        case FEED:
            pet_feed();
            break;
        case PLAY:
            pet_play();
            break;
        case SLEEP:
            pet_sleep();
            break;
        case DEAD:
            pet_dead();
            break;
        case RUN_AWAY:
            pet_run_away();
            break;
    }
}

void pet_idle(void) {
    // Logic for idle state, to be implemented
  CheckHungerAndHappiness(); 
}

void pet_menu(void) {
    // Logic for menu state, to be implemented
    /*
    can be used to display the menu options and handle user input for selecting actions
    can go to FEED, PLAY or SLEEP
    */
}

void pet_dead(void) {
    // Logic for dead state, to be implemented
    /*
            printf("Pet is dead. Please restart the game.\n");
            idk press any key to restart the game
            call Pet_Init() to restart the game
            */

    Pet_Init(); // Restart the game when the pet is dead
}

void pet_run_away(void) {
    // Logic for run away state, to be implemented
    /*
            printf("Pet ran away. Please restart the game.\n");
            idk press any key to restart the game
            call Pet_Init() to restart the game
            */

    Pet_Init(); // Restart the game when the pet runs away
}

void pet_feed(void){
    hunger += 20;
    CapHungerAndHappiness(); // Ensure hunger does not exceed 100

    pet_state = IDLE; // Return to idle state after feeding
}

void pet_play(void){
    happiness += 20;
    CapHungerAndHappiness(); // Ensure happiness does not exceed 100

    pet_state = IDLE; // Return to idle state after playing
}

void pet_sleep(void){
    hunger -= 50;
    happiness += 10;

    CheckHungerAndHappiness(); // Check if hunger or happiness has reached 0
    CapHungerAndHappiness(); // Ensure hunger and happiness do not exceed 100

    pet_state = IDLE; // Return to idle state after sleeping
}

void CheckHungerAndHappiness(void) {
    if (hunger <= 0) {
        pet_state = DEAD; // Pet dies if hunger reaches 0
    } else if (happiness <= 0) {
        pet_state = RUN_AWAY; // Pet runs away if happiness reaches 0
    }
}

void CapHungerAndHappiness(void) {
    if (hunger > 100) {
        hunger = 100; // Cap hunger at 100
    }
    if (happiness > 100) {
        happiness = 100; // Cap happiness at 100
    }
}

PetState get_pet_state(void) {
    return pet_state;
}

int get_happiness(void) {
    return happiness;
}

int get_hunger(void) {
    return hunger;
}
