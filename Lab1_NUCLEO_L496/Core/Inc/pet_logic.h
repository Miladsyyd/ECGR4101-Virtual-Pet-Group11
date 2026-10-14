#ifndef PET_LOGIC_H
#define PET_LOGIC_H

#include <stdint.h>

// Pet states
typedef enum {
    IDLE,
    MENU,
    FEED,
    PLAY,
    SLEEP,
    DEAD,
    RUN_AWAY
} PetState;

// Current pet state
extern PetState pet_state;

// Pet initialization
void Pet_Init(void);

// Pet actions
void pet_idle(void);
void pet_feed(void);
void pet_play(void);
void pet_sleep(void);

// Menu
void menu(int buttonPressed);

// Death and runaway
void death(void);
void gone(void);

// State management
void setPetState(PetState newState);
PetState getPetState(void);

// Pet stats
int getHappiness(void);
int getHunger(void);

#endif // PET_LOGIC_H
