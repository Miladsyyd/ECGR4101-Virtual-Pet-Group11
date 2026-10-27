/**
 * @file    pet_anims.h
 * @brief   Connects the Sprout sprite frames to the animation engine.
 */
#ifndef PET_ANIMS_H
#define PET_ANIMS_H

/* Registers every pet animation (idle, feed, play, sleep, death, runaway).
 * Call once after Anim_Init(). */
void PetAnims_RegisterAll(void);

#endif /* PET_ANIMS_H */