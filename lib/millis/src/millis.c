/**
 * @file millis.c
 * @brief Implementation of millisecond timing routines using AVR Timer0.
 * @details Configures Timer0 in CTC (Clear Timer on Compare Match) mode with a 64 
 *          prescaler to generate an interrupt every 1ms at 16 MHz.
 *
 * @author Arif Rachmat (ngaripar1203@gmail.com)
 * @date 2026-09-17
 * @version 1.0.0
 * 
 * @copyright Copyright (c) 2026 Muhammad Arif Rachmat
 *            Licensed under the MIT License (see LICENSE for details).
 */

#include "millis.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static volatile uint32_t g_system_millis = 0;

// This function is automatically called exactly once every millisecond.
ISR(TIMER0_COMPA_vect) {
    g_system_millis++;
}

void millis_init(void) {
    // --- Timer0 Configuration ---
    // Mode: CTC (Clear Timer on Compare Match)
    // Prescaler: 64
    // Compare Match: 249
    // This results in an interrupt frequency of 1000 Hz (1ms).
    // Calculation: (16,000,000 Hz / 64) / (249 + 1) = 1000 Hz

    // Configure Timer0 for CTC (Clear Timer on Compare Match) Mode
    TCCR0A = (1 << WGM01);

    // Set Prescaler to 64 (16 MHz / 64 = 250,000 Hz -> 4 us per tick)
    TCCR0B = (1 << CS01) | (1 << CS00);

    // Compare Match value for 1 ms interval: (250,000 / 1000) - 1 = 249
    OCR0A = 249;

    // Enable Timer0 Compare Match A Interrupt
    TIMSK0 |= (1 << OCIE0A);

    // Enable Global Interrupts
    sei();
}

uint32_t millis(void) {
    uint32_t millis_value;
    uint8_t sreg = SREG; // Save global interrupt flag

    // Disable interrupts temporarily to safely read the 4-byte
    // g_system_millis variable. This prevents the ISR from
    // changing the value halfway through the read.
    cli(); // Disables all interrupts
    millis_value = g_system_millis;
    SREG = sreg; // Restore global interrupt flag

    return millis_value;
}
