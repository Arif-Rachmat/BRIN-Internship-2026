#ifndef I2C_LCD_H
#define I2C_LCD_H

#include <stdbool.h>
#include <avr/io.h> 
#include <stdint.h> 

#ifndef F_CPU
#warning "F_CPU not defined for avr_i2c_lcd.h. Defaulting to 16MHz."
#define F_CPU 16000000UL
#endif

// --- I2C Pin Configuration ---
#define I2C_DDR  DDRC
#define I2C_PORT PORTC
#define I2C_PIN  PINC
#define I2C_SCL  PC5
#define I2C_SDA  PC4

#define PCF8574_ADDR 0x27

// --- I2C Timing Delay ---
// F_CPU=16MHz: _delay_us(2) -> ~170kHz
// F_CPU=16MHz: _delay_us(4) -> ~100kHz (Standard Mode)
// F_CPU=8MHz:  _delay_us(1) -> ~170kHz
// F_CPU=8MHz:  _delay_us(4) -> ~100kHz
// F_CPU=1MHz:  _delay_us(4) -> ~18kHz
#define I2C_DELAY() _delay_us(4)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the LCD into 4-bit mode via I2C.
 */
void lcd_init(void);

/**
 * @brief Clears the entire LCD display and returns cursor to home.
 */
void lcd_clear(void);

/**
 * @brief Sets the cursor position (DDRAM address).
 * @param row 0 for the first line, 1 for the second.
 * @param col 0 to 15 (column).
 */
void lcd_goto(uint8_t row, uint8_t col);

/**
 * @brief Prints a null-terminated string to the LCD at the current cursor.
 * @param str The string to print.
 */
void lcd_string(const char* str);

/**
 * @brief Sends a single data byte (character) to the LCD.
 * @param data The character to display.
 */
void lcd_data(uint8_t data);

/**
 * @brief Sends a command byte to the LCD.
 * @param cmd The command byte (e.g., 0x01 for clear, 0x0C for display on).
 */
void lcd_command(uint8_t cmd);

/**
 * @brief Controls the LCD backlight.
 * @param on True to turn the backlight on, False to turn it off.
 */
void lcd_backlight(bool on);

#ifdef __cplusplus
}
#endif

#endif