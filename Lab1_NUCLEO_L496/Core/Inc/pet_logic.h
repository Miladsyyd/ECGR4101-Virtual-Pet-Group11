#ifndef PET_LOGIC_H
#define PET_LOGIC_H

#include <stdint.h>

typedef enum
{
    IDLE,
    MENU,
    FEED,
    PLAY,
    SLEEP,
    DEAD,
    RUN_AWAY,
    PLAY_GAME_SELECT,
    SHAKE_GAME,
    RADIO_GAME
} PetState;

enum
{
    PET_BUTTON_LEFT = 1U << 0,
    PET_BUTTON_CENTER = 1U << 1,
    PET_BUTTON_DOWN = 1U << 2,
    PET_BUTTON_RIGHT = 1U << 3,
    PET_BUTTON_UP = 1U << 4
};

void Pet_Init(void);
void Pet_Update(uint8_t buttons);
void Pet_GameComplete(void);

void pet_feed(void);
void pet_play(void);
void pet_sleep(void);

void setPetState(PetState newState);
PetState getPetState(void);
int getHappiness(void);
int getHunger(void);
int getSleep(void);

#endif
