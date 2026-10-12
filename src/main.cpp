#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <util/delay.h>
#include "i2cLCD.h"


int main(void) {

    lcd_init();
    lcd_clear();

    lcd_string("Counter:");
    uint8_t counter = 200; // Max = 65535

    while (1) {
        char buffer[6];
        memset(buffer, 0, sizeof(buffer));
        itoa(counter++, buffer, 10);
        char *ptr = &buffer[5];
        while(!*(--ptr)) *ptr = ' ';

        lcd_goto(0,8);
        lcd_string(buffer);

        _delay_ms(250);     // Wait for 1 second.
    }
}
