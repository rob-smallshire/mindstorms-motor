#include "current_sense.h"

void init_adc(void)
{
    //16MHz/128 = 125kHz the ADC reference clock
    ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));
    ADMUX  |= (1<<REFS0) | (1<<REFS1);       // Set Voltage reference to internal 1.1v
    ADMUX |= (1 << ADLAR);     // Left adjust ADC result to allow easy 8 bit reading
    ADCSRA |= (1<<ADEN);       // Turn on ADC
    ADCSRA |= (1<<ADSC);       // Do an initial conversion
}

uint8_t read_adc(uint8_t channel)
{
    ADMUX = (ADMUX & 0xf0) | (0x0f & channel);
    ADCSRA |= (1<<ADSC);       // Starts a new conversion
    while(ADCSRA & (1<<ADSC)); // Wait until the conversion is done
    return ADCH;               // Returns the ADC value of the chosen channel
}         
