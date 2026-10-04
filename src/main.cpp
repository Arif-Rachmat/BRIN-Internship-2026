#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include <avr/interrupt.h>
#include "uart.h"

uint8_t iterator;
uint16_t adcSum;

inline int32_t adc_to_millidegrees(uint16_t adc_val) {
    // Cast to uint32_t prevents overflow (1023 * 6875 + 32 = 7,033,157 max)
    uint32_t temp_mC = ((uint32_t)adc_val * 6875UL + 32UL) >> 6;
    return (int32_t)temp_mC;
}

ISR(ADC_vect){
    if (iterator--) {
        volatile uint16_t adcRaw = ADC;
        adcSum += adcRaw;
    } else {
        uint32_t millidegree = adc_to_millidegrees(adcSum/10);
        printf("%lu.%lu\n", millidegree/1000, millidegree%1000);
        adcSum = 0;
        iterator = 10;
    }
}

int main(void) {
 
    iterator = 10;
    adcSum = 0;
    uart_init(115200, F_CPU);

    ADMUX = _BV(REFS1) | _BV(REFS0); // 1.1V reference, PC0/ADC0 ADC input
    ADCSRB &= ~(_BV(ADTS2) | _BV(ADTS1) | _BV(ADTS0)); // Free running mode
    ADCSRA = _BV(ADEN) | _BV(ADSC) | _BV(ADATE) | _BV(ADIE) | _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);
    DIDR0 |= _BV(ADC0D);

    sei();

    while (1) {
        for (volatile uint8_t i = 0; i < UINT8_MAX; i++);
    }
}
