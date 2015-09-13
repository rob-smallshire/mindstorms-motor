#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <avr/io.h>

void joystick_init(void);

uint16_t read_adc(uint8_t channel);
#endif
