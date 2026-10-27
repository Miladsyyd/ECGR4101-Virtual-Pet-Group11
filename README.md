This project is an embedded virtual pet game developed for the STM32 NUCLEO-L496ZG-P. The system combines a graphical LCD interface, joystick controls, sensors, animations, and sound to create a small Tamagotchi-style virtual pet.

The pet has three primary status meters:

Food

Fun

Sleep

These values gradually decrease over time, requiring the user to interact with the pet to keep it alive and happy. 
Each stat is restored through a different interaction. Food can be restored directly through the menu, while Fun and Sleep use hardware-based minigames.

The project demonstrates integration of GPIO, ADC, I2C, SPI, PWM/timers, UART debugging, graphics, sensors, and state-based embedded software.
