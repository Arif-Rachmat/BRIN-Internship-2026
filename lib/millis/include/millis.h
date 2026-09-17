/**
 * @file millis.h
 * @brief Millisecond timekeeping library for AVR microcontrollers.
 * @details Provides a millisecond counter using hardware Timer0 configured in 
 *          CTC mode. Provides thread-safe / interrupt-safe readings 
 *          similar to Arduino's millis() function.
 *
 * @author Arif Rachmat (ngaripar1203@gmail.com)
 * @date 2026-09-17
 * @version 1.0.0
 * 
 * @copyright Copyright (c) 2026 Muhammad Arif Rachmat
 *            Licensed under the MIT License (see LICENSE for details).
 *
 * @note Currently only designed for AVR microcontrollers running at 16 MHz by default.
 */

#ifndef MILLIS_TIMER_H
#define MILLIS_TIMER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes Timer0 to generate an interrupt every millisecond.
 *
 * This function configures Timer0 in CTC mode with a prescaler of 64.
 * It sets up the compare match value to trigger an interrupt precisely
 * every 1ms. Global interrupts is enabled automaticaly with sei()
 * after calling this function.
 */
void millis_init(void);

/**
 * @brief Returns the number of milliseconds since the program started.
 *
 * This function returns the value of a 32-bit millisecond counter which
 * is incremented by a timer interrupt. The read operation is interrupt-safe.
 *
 * @return The number of milliseconds as an unsigned long.
 */
uint32_t millis(void);

#ifdef __cplusplus
}
#endif

#endif // MILLIS_TIMER_H
