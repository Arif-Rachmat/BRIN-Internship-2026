#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include "millis.h"

#define LED_PIN PB5 // Onboard LED on ATmega328P (Arduino Uno Pin 13)

int main(void) {
    // Configure PB5 as an output
    DDRB |= (1 << LED_PIN);

    // Initialize Timer0 and enable global interrupts
    millis_init();

    uint32_t previous_millis = 0;
    const uint32_t interval = 1000; // Blink interval: 1000 ms

    while (1) {
        uint32_t current_millis = millis();

        // Check if the 1000 ms interval has elapsed
        if (current_millis - previous_millis >= interval) {
            previous_millis = current_millis;

            // Toggle LED state
            PORTB ^= (1 << LED_PIN);
        }
    }
    return 0;
}
