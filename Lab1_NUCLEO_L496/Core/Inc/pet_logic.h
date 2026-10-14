#ifndef PET_LOGIC_H
#define PET_LOGIC_H

typedef enum{
  IDLE,
  MENU,
  FEED,
  PLAY,
  SLEEP,
  DEAD,
  RUN_AWAY
} PetState;

//Starts or restarts the pet, setting hunger and happiness to 100 and state to IDLE
void Pet_Init(void); 

//Runs the pet logic repeatedly in main loop, includes decay
void updatePetState(PetState newState);

void pet_idle(void); //placeholder for idle state logic, can be used to display the pet's idle behavior
void pet_menu(void); // placeholder for menu state logic, can be used to display the menu options and handle user input for selecting actions
void pet_dead(void); //placeholder for dead state logic, can be used to display a message or handle user input for restarting the pet
void pet_run_away(void); //placeholder for run away state logic, can be used to display a message or handle user input for restarting the pet

void pet_feed(void); //Increases hunger by 20
void pet_play(void); //Increases happiness by 20
void pet_sleep(void); //Decreases hunger by 25 and increases happiness by 10

void setPetState(PetState newState); 

PetState getPetState(void);
int getHappiness(void);
int getHunger(void);

#endif
