#include "i2cLCD.h"
#include <util/delay.h>

#define LCD_RS_BIT (1 << 0)
#define LCD_RW_BIT (1 << 1) // Assumed 0 (grounded on most modules)
#define LCD_E_BIT  (1 << 2)
#define LCD_BL_BIT (1 << 3) // Backlight
#define LCD_D4_BIT (1 << 4)
#define LCD_D5_BIT (1 << 5)
#define LCD_D6_BIT (1 << 6)
#define LCD_D7_BIT (1 << 7)

static uint8_t g_backlight_state = LCD_BL_BIT; // Backlight is ON by default

#define I2C_SCL_HI() I2C_PORT |= (1 << I2C_SCL)
#define I2C_SCL_LO() I2C_PORT &= ~(1 << I2C_SCL)

#define I2C_SDA_HI() I2C_PORT |= (1 << I2C_SDA)
#define I2C_SDA_LO() I2C_PORT &= ~(1 << I2C_SDA)

#define I2C_SDA_OUT() I2C_DDR |= (1 << I2C_SDA)
#define I2C_SDA_IN()  I2C_DDR &= ~(1 << I2C_SDA)

/**
 * @brief Initializes I2C pins. SCL=output, SDA=output.
 */
static inline void i2c_init(void) {
    // Set SCL and SDA as outputs
    I2C_DDR |= (1 << I2C_SCL) | (1 << I2C_SDA);
    // Set both high (idle state)
    I2C_PORT |= (1 << I2C_SCL) | (1 << I2C_SDA);
}

/**
 * @brief Generates I2C START condition.
 */
static inline void i2c_start(void) {
    I2C_SDA_OUT(); // Ensure SDA is output
    I2C_SDA_HI();
    I2C_SCL_HI();
    I2C_DELAY();
    I2C_SDA_LO();
    I2C_DELAY();
    I2C_SCL_LO();
    I2C_DELAY();
}

/**
 * @brief Generates I2C STOP condition.
 */
static inline void i2c_stop(void) {
    I2C_SDA_OUT(); // Ensure SDA is output
    I2C_SDA_LO();
    I2C_SCL_HI();
    I2C_DELAY();
    I2C_SDA_HI();
    I2C_DELAY();
}

/**
 * @brief Writes one byte to the I2C bus.
 * @note Skips ACK check for simplicity and speed.
 */
static inline void i2c_write(uint8_t data) {
    I2C_SDA_OUT(); // Ensure SDA is output
    for (uint8_t i = 0; i < 8; i++) {
        // Set SDA bit
        if (data & 0x80) {
            I2C_SDA_HI();
        } else {
            I2C_SDA_LO();
        }
        data <<= 1;
        
        // Pulse SCL
        I2C_SCL_HI();
        I2C_DELAY();
        I2C_SCL_LO();
        I2C_DELAY();
    }
    
    // Clock out the 9th bit (ACK/NACK)
    I2C_SDA_IN();
    I2C_SDA_HI();
    I2C_SCL_HI();
    I2C_DELAY();
    I2C_SCL_LO();
    I2C_DELAY();
    I2C_SDA_OUT();
}



// --- PCF8574 Driver (static) ---

/**
 * @brief Writes a byte to the PCF8574 expander.
 */
static inline void pcf_write(uint8_t data) {
    i2c_start();
    // Send 7-bit address + Write bit (0)
    i2c_write(PCF8574_ADDR << 1); 
    // Send data byte, preserving backlight state
    i2c_write(data | g_backlight_state);
    i2c_stop();
}


/**
 * @brief Pulses the LCD Enable (E) pin to latch data.
 */
static inline void lcd_pulse_enable(uint8_t data) {
    pcf_write(data | LCD_E_BIT);  // E high
    _delay_us(1); // Min 450ns
    pcf_write(data & ~LCD_E_BIT); // E low
    _delay_us(50); // Command settling time
}

/**
 * @brief Sends a 4-bit nibble to the LCD.
 */
static inline void lcd_write_nibble(uint8_t nibble, bool rs) {
    // Map high nibble (D4-D7) to PCF8574 pins P4-P7
    uint8_t data = (nibble << 4) & 0xF0;
    if (rs) {
        data |= LCD_RS_BIT;
    }
    // RW is assumed 0 (tied to GND)
    lcd_pulse_enable(data);
}

/**
 * @brief Sends a full 8-bit byte (command or data) to the LCD.
 */
static inline void lcd_send_byte(uint8_t byte, bool rs) {
    lcd_write_nibble(byte >> 4, rs);   // High nibble first
    lcd_write_nibble(byte & 0x0F, rs); // Low nibble second
}

/**
 * @brief Sends a command to the LCD.
 */
void lcd_command(uint8_t cmd) {
    lcd_send_byte(cmd, false);
}

/**
 * @brief Sends data (a character) to the LCD.
 */
void lcd_data(uint8_t data) {
    lcd_send_byte(data, true);
}

/**
 * @brief Clears the LCD display.
 */
void lcd_clear(void) {
    lcd_command(0x01); // Clear Display command
    _delay_ms(2); // This command takes longer to execute
}

/**
 * @brief Sets the cursor position.
 */
void lcd_goto(uint8_t row, uint8_t col) {
    // 0x00 is line 1, 0x40 is line 2
    uint8_t addr = (row == 0) ? 0x00 : 0x40;
    lcd_command(0x80 | (addr + col)); // Set DDRAM Address command
}

/**
 * @brief Prints a null-terminated string to the LCD.
 */
void lcd_string(const char* str) {
    while (*str) {
        lcd_data(*str++);
    }
}

/**
 * @brief Controls the LCD backlight.
 */
void lcd_backlight(bool on) {
    g_backlight_state = on ? LCD_BL_BIT : 0;
    pcf_write(0); // Write 0 data, which only updates the backlight bit
}

/**
 * @brief Initializes the LCD into 4-bit mode.
 */
void lcd_init(void) {
    i2c_init(); // Initialize I2C pins
    
    _delay_ms(50);
    
    lcd_write_nibble(0x03, false);
    _delay_ms(5);
    
    lcd_write_nibble(0x03, false);
    _delay_us(150);
    
    lcd_write_nibble(0x03, false);
    _delay_us(150);
    
    lcd_write_nibble(0x02, false);
    _delay_us(150);
    
    
    lcd_command(0x28);
    
    lcd_command(0x0C);
    
    lcd_clear();
    
    lcd_command(0x06);
}