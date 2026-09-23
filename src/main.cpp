#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "ds18b20.h"
#include "onewire.h"
#include "uart.h"

static const onewire_bus_t sensorPin = {.data =
                                            ONEWIRE_GPIO(DDRB, PORTB, PINB, 0),
                                        .strong_pullup = {0, 0, 0, 0},
                                        .strong_pullup_active_high = true,
                                        .critical = NULL};

int main(void) {
    uint8_t rom[8];
    int16_t rawTemp;

    uart_init(115200, F_CPU);

    if (onewire_init(&sensorPin) != ONEWIRE_OK) {
        puts_P(PSTR("Onewire Initialization Failed"));
        while (1)
            ;
    }

    if (onewire_read_rom(&sensorPin, rom) != ONEWIRE_OK) {
        puts_P(PSTR("No Onewire Device connected or ROM CRC bad"));
        while (1)
            ;
    } else {
        puts_P(PSTR("Connected Device ROM:"));
        for (auto &&i : rom) {
            printf("%X ", i);
        }
        putchar('\n');
    }

    while (1) {
        if (ds18b20_convert_wait(&sensorPin, rom, false,
                                 DS18B20_RESOLUTION_12BIT) != ONEWIRE_OK) {
            continue;
        }
        
        if(ds18b20_read_raw(&sensorPin, rom, &rawTemp) != ONEWIRE_OK){
            continue;
        }

        printf("%d.%04d\n", rawTemp >> 4, (rawTemp & 0xF) * 625);
    }
}
