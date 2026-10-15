
#ifndef PET_LOGIC_H
#define PET_LOGIC_H

typedef enum {
    IDLE,
    MENU,
    FEED,
    PLAY,
    SLEEP,
    DEAD,
    RUN_AWAY
} PetState;

void Pet_Init(void);
void updatePetState(PetState newState);

void pet_idle(void);
void pet_menu(void);
void pet_dead(void);
void pet_run_away(void);

void pet_feed(void);
void pet_play(void);
void pet_sleep(void);

void setPetState(PetState newState);

PetState getPetState(void);

int getHappiness(void);
int getHunger(void);
int getEnergy(void);

#endif
