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

void Pet_Init(void);
void Pet_Update(void);
//void Pet_Restart(void);


void pet_idle(void);
void pet_menu(void);
void pet_dead(void);
void pet_run_away(void);

void pet_feed(void);
void pet_play(void);
void pet_sleep(void);

PetState get_pet_state(void);
int get_happiness(void);
int get_hunger(void);

#endif
