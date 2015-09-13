#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "joystick.h"

#include <util/delay.h>

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
 BLINK_DELAY_MS = 200,
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


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int main (void)
{
    joystick_init();

    sei();

    stdin = stdout = &uart0_stream;

    // set pin 5 of PORTB for output
    DDRB |= _BV(DDB5);

    // USB Serial 0
    uart0_init(UART_BAUD_SELECT(9600, F_CPU));

    printf("mindstorms-motor!\n");
    while (1) {
        // pin 5 high to turn led on
        PORTB |= _BV(PORTB5);
        _delay_ms(BLINK_DELAY_MS);

        // set pin 5 low to turn led off
        PORTB &= ~_BV(PORTB5);
        _delay_ms(BLINK_DELAY_MS);
    }
    return 0;
}
#pragma clang diagnostic pop
