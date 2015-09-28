#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <avr/io.h>

void init_adc(void);

uint8_t read_adc(uint8_t channel);
#endif
