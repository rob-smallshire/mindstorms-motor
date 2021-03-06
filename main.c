#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"
#include "current_sense.h"

#include <util/delay.h>
#include <util/atomic.h>

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

#ifdef __GNUC__
#  define UNUSED_FUNCTION(x) __attribute__((__unused__)) UNUSED_ ## x
#else
#  define UNUSED_FUNCTION(x) UNUSED_ ## x
#endif

enum {
 BLINK_DELAY_MS = 100,
};

int uart0_send_byte(char data, FILE* UNUSED(stream))
{
    if (data == '\n')
    {
        uart0_putc('\r');
    }
    uart0_putc((uint8_t)data);
    return 0;
}

int uart0_receive_byte(FILE* UNUSED(stream))
{
    uint8_t data = uart0_getc();
    return data;
}

static FILE uart0_stream = FDEV_SETUP_STREAM(
                            uart0_send_byte,
                            uart0_receive_byte,
                            _FDEV_SETUP_RW);


volatile uint16_t counter;
//volatile uint8_t channels;
uint8_t previous_channels;
//volatile uint8_t channels_changed;


ISR (PCINT2_vect)
{
    uint8_t channels = PIND & (_BV(PIND3) | _BV(PIND2));
    uint8_t channels_changed = channels ^ previous_channels;

    uint8_t a_channels = (channels_changed & _BV(PIND3)) >> 1;
    uint8_t b_channels = channels_changed & _BV(PIND2);
    uint8_t encoder_changed = a_channels ^ b_channels;

    //uint8_t encoder_skipped = a_channels & b_channels;
    if (encoder_changed)
    {
        uint8_t direction = ((channels & _BV(PIND3)) >> 1) ^ (previous_channels & _BV(PIND2));
        if (direction & _BV(PIND2))
        {
            ++counter;
        }
        else
        {
            --counter;
        }
    }
    previous_channels = channels;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int main (void)
{
    init_adc();

    stdin = stdout = &uart0_stream;

    // USB Serial 0
    uart0_init(UART_BAUD_SELECT(9600, F_CPU));
    printf("mindstorms-motor!\n");

    // Port B
    DDRB |= _BV(DDB0); // nSLEEP on Arduino D8
    DDRB |= _BV(DDB1); // DIR on Arduino D9
    DDRB |= _BV(DDB2); // BRAKE on Arduino D10
    DDRB |= _BV(DDB3); // PWM on Arduino D11
    DDRB &= ~_BV(DDB4); // nFAULT on Arduino D12
    DDRB |= _BV(DDB5); // LED on Arduino D13

    PORTB |= _BV(PORTB0);
    PORTB |= _BV(PORTB1);
    PORTB &= ~_BV(PORTB2);
    PORTB |= _BV(PORTB3);

    // Port D
    DDRD &= ~_BV(DDD2); // Encoder 0 on Arduino D2
    DDRD &= ~_BV(DDD3); // Encoder 1 on Arduino D3

    PORTD |= _BV(PORTD2); // Enable pullup on D2
    PORTD |= _BV(PORTD3); // Enable pullup on D3

    previous_channels = PIND & (_BV(PIND3) | _BV(PIND2));

    PCICR = (1 << PCIE2);
    PCMSK2 = (1 << PCINT18) | (1 << PCINT19);

    // Configure Timer 2 - 8 bit timer

    // Waveform generator mode - Fast PWM
    uint8_t waveform_generator_mode_a = (1 << WGM21) | (1 << WGM20);
    uint8_t waveform_generator_mode_b = (0 << WGM22);

    // Compare output mode
    uint8_t compare_output_mode_oc2a_a = (1 << COM2A1) | (0 << COM2A0); // Non-inverting
    uint8_t compare_output_mode_oc2b_a = (0 << COM2B1) | (0 << COM2B1); // Disconnected

    // Force output compare - disabled
    uint8_t force_output_compare_b = (0 << FOC2A) | (0 << FOC2B); // Zero in PWM modes

    // Clock select - Clock/8 = 16 MHz / 8 = 2 MHz
    uint8_t clock_select_b = (0 << CS22) | (1 << CS21) | (0 << CS20);

    TCCR2A = waveform_generator_mode_a | compare_output_mode_oc2a_a | compare_output_mode_oc2b_a;
    TCCR2B = waveform_generator_mode_b | force_output_compare_b | clock_select_b;

    OCR2A = 150;

    uint8_t speed = 255;

    int8_t acceleration = -1;

    sei();

    while (1) {
        // pin 5 high to turn led on
        PORTB |= _BV(PORTB5);
        _delay_ms(BLINK_DELAY_MS);


        OCR2A = speed;
        printf("speed = %d", speed);
        if (speed == 0) {
            acceleration = +1;
            PORTB ^= _BV(PORTB1);  // Reverse direction
        }
        else if (speed == 255)
        {
            acceleration = -1;
        }
        speed += acceleration;


        // set pin 5 low to turn led off
        PORTB &= ~_BV(PORTB5);
        _delay_ms(BLINK_DELAY_MS);

        uint8_t current = read_adc(0);
        printf(", current = %" PRIu8, current);

        uint16_t counter_copy;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            counter_copy = counter;
        }

        printf(", counter = %" PRIu8, counter_copy);
        printf("\n");
    }
    return 0;
}
#pragma clang diagnostic pop
