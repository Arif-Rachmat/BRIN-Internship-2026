#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <stdlib.h>
#include <string.h> // memset

#include "millis.h"

int main (void) {
    DDRB |= _BV(DDB5);
    DDRC |= 0b00111111;
    DDRD = 0xFF;
    PORTB = 0xFF;
    PORTC = _BV(PORTC0);
    PORTD = 0b10101010;

    uint8_t toggleCounter = 4;
    uint16_t adjDelay = 3000;
    uint32_t timeNote[3];
    memset (timeNote, 0, sizeof (timeNote));

    millis_init ();

    while (1) {
        uint32_t now = millis ();

        // Task 1
        if (now - timeNote[0] > adjDelay) {
            if (!toggleCounter--)
            {
                toggleCounter = 4;
                adjDelay = 3000;
            } else {
                adjDelay = 100;
                PINB = _BV(PORTB5);
            }
            timeNote[0] = now;
        }

        // Task 2
        if (now - timeNote[1] > 700) {
            PIND = 0xFF;
            timeNote[1] = now;
        }

        // Task 3
        if (now - timeNote[2] > 250) {
            if (PORTC >= 32) {
                PORTC = _BV (PORTC0);
            } else {
                PORTC <<= 1;
            }
            timeNote[2] = now;
        }
    }
}
