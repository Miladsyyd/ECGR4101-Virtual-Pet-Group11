#ifndef PET_LOGIC_H
#define PET_LOGIC_H

typedef enum
{
  PET_IDLE,
  PET_MENU,
  PET_FEED,
  PET_PLAY,
  PET_3RDACTION, //placeholder
  PET_DEAD,
  PET_RAN_AWAY
} PetState;

void Pet_Init(void);
void Pet_Update(void);
void Pet_Feed(void);
void Pet_Play(void);
void Pet_3rdAction(void);
void Pet_Restart(void);

#endif
