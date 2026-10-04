/*
 * main.cpp
 *
 * UART test program for ATmega328P.
 *
 * Test:
 *   - uart_init()
 *   - uart_transmit()
 *   - uart_receive()
 *   - uart_available()
 *   - RX interrupt
 *   - TX interrupt
 *   - TX ring buffer
 *   - RX ring buffer
 *   - stdio / printf()
 *   - getchar()
 *   - puts()
 *   - CRLF conversion
 *   - formatted output
 *   - binary byte transmission
 *   - RX echo
 *   - TX buffer-full behavior
 *   - RX buffer behavior
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <stdlib.h>
#include <stddef.h>

#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "uart.h"


#define UART_BAUD_RATE 115200UL


/* --------------------------------------------------------------------------
 * Small helpers
 * -------------------------------------------------------------------------- */

static void print_line(void)
{
    puts_P(PSTR(
        "\n"
        "----------------------------------------"
        "--------------------\n"
    ));
}


static void print_header(const char *title)
{
    print_line();
    printf_P(PSTR("TEST: %s\n"), title);
    print_line();
}


static uint8_t wait_byte(void)
{
    uint8_t data;

    while (!uart_receive(&data));

    return data;
}


static void drain_rx(void)
{
    uint8_t data;

    while (uart_receive(&data));
}


static bool send_raw_string_P(const char *str)
{
    char c;

    while ((c = (char)pgm_read_byte(str++)) != '\0') {
        while (!uart_transmit((uint8_t)c));
    }

    return true;
}


// Test 1
static void test_basic_tx(void)
{
    print_header("Basic TX");

    puts_P(PSTR(
        "UART TX is working.\n"
    ));

    printf_P(PSTR(
        "This line was sent through printf().\n"
    ));

    puts_P(PSTR(
        "This line was sent through puts_P()."
    ));

    puts_P(PSTR(
        "Raw uart_transmit(): "
    ));

    uart_transmit('A');
    uart_transmit('B');
    uart_transmit('C');
    uart_transmit('\r');
    uart_transmit('\n');

    puts_P(PSTR("Basic TX test complete.\n"));
}


// Test 2
static void test_formatted_output(void)
{
    uint8_t  u8  = 200;
    uint16_t u16 = 54321;
    uint32_t u32 = 123456789UL;

    int8_t   s8  = -42;
    int16_t  s16 = -12345;
    int32_t  s32 = -123456789L;

    print_header("Formatted output");

    printf_P(PSTR(
        "uint8_t  = %u\n"
        "int8_t   = %d\n"
        "uint16_t = %u\n"
        "int16_t  = %d\n"
        "uint32_t = %lu\n"
        "int32_t  = %ld\n"
    ),
        (unsigned)u8,
        (int)s8,
        (unsigned)u16,
        (int)s16,
        (unsigned long)u32,
        (long)s32
    );

    printf_P(PSTR(
        "Hex      = 0x%08lX\n"
    ), (unsigned long)u32);

    puts_P(PSTR("Formatted output test complete.\n"));
}


// Test 3
static void test_raw_tx(void)
{
    print_header("Raw uart_transmit()");

    send_raw_string_P(PSTR(
        "This complete sentence is transmitted exclusively "
        "through uart_transmit().\n\r"
    ));

    puts_P(PSTR("Raw TX test complete.\n"));
}


// Test 4
static void test_available(void)
{
    print_header("uart_available()");

    drain_rx();

    puts_P(PSTR(
        "RX buffer cleared.\n"
        "Current available bytes = %u\n"
        "\n"
        "Send characters, then press ENTER.\n"
    ));

    while (1) {

        uint8_t count = uart_available();

        if (count != 0) {

            // Read until newline.
            while (uart_available() != 0) {

                uint8_t data = wait_byte();

                if (data == '\r' || data == '\n') {
                    goto done;
                }
            }
        }
    }

done:
    puts_P(PSTR("\nuart_available() test complete.\n"));
}


// Test 5
static void test_receive(void)
{
    print_header("uart_receive()");

    puts_P(PSTR(
        "Type 10 characters.\n"
        "\n"
    ));

    for (uint8_t i = 0; i < 10; ++i) {

        uint8_t data = wait_byte();

        printf_P(
            PSTR(
                "Byte %u: 0x%02X '%c'\n"
            ),
            (unsigned)i,
            (unsigned)data,
            (data >= 32 && data <= 126) ? data : '.'
        );
    }

    puts_P(PSTR("Receive test complete.\n"));
}


// Test 6
static void test_echo(void)
{
    print_header("Echo");

    drain_rx();

    puts_P(PSTR(
        "Echo mode active.\n"
        "Press ESC to exit.\n"
        "\n"
    ));

    while (1) {
        uint8_t data = wait_byte();

        if (data == 0x1B) {
            break;
        }

        while (!uart_transmit(data)) {
        }
    }

    puts_P(PSTR("\nEcho mode terminated.\n"));
}


// Test 7
// Deliberately attempt to overfill the TX ring buffer.
// No large array is required.
static void test_tx_overflow(void)
{
    uint16_t accepted = 0;
    uint16_t rejected = 0;

    print_header("TX buffer full condition");

    puts_P(PSTR(
        "Attempting to queue 256 bytes without waiting.\n"
        "This intentionally exercises the full-buffer path.\n"
        "\n"
    ));

    for (uint16_t i = 0; i < 256; ++i) {

        if (uart_transmit((uint8_t)('A' + (i % 26)))) {
            ++accepted;
        }
        else {
            ++rejected;
        }
    }

    printf_P(PSTR(
        "Accepted = %u\n"
        "Rejected = %u\n"
    ),
        (unsigned)accepted,
        (unsigned)rejected
    );

    // Give the transmitter time to empty.
    for (volatile uint16_t i = 0; i < 50000; ++i) {
    }

    puts_P(PSTR("TX buffer test complete.\n"));
}


// Test 8
// User manually sends a large amount of data.
// This test does NOT allocate a second RX buffer.
static void test_rx_buffer(void)
{
    uint16_t received = 0;

    print_header("RX buffer");

    drain_rx();

    puts_P(PSTR(
        "Send a stream of characters now.\n"
        "After transmission stops, press ENTER.\n"
    ));

    while (1) {

        if (uart_available() != 0) {

            uint8_t data = wait_byte();

            if (data == '\r' || data == '\n') {
                break;
            }

            ++received;
        }
    }

    printf_P(PSTR(
        "\nCharacters received = %u\n"
    ), (unsigned)received);

    puts_P(PSTR("RX buffer test complete.\n"));
}


// Test 9
static void test_getchar(void)
{
    print_header("getchar()");

    puts_P(PSTR("Press one character:\n"));

    int c = getchar();

    printf_P(PSTR(
        "\n"
        "getchar() returned:\n"
        "Decimal = %d\n"
        "Hex     = 0x%02X\n"
        "ASCII   = '%c'\n"
    ),
        c,
        (unsigned)c,
        (c >= 32 && c <= 126) ? c : '.'
    );
}


// Test 10
static void test_newline(void)
{
    print_header("CRLF conversion");

    puts_P(PSTR(
        "Line 1\n"
        "Line 2\n"
        "Line 3\n"
    ));

    puts_P(PSTR(
        "\n"
        "Expected wire format:\n"
        "0D 0A after every newline.\n"
    ));
}


// Test 11
// Binary values 0x00-0xFF.
static void test_binary(void)
{
    print_header("Binary TX");

    printf_P(PSTR(
        "Sending bytes 0x00 through 0xFF.\n"
        "Use a logic analyzer or binary-capable receiver.\n\n"
    ));

    for (uint16_t i = 0; i <= 0xFF; ++i) {
        while (!uart_transmit((uint8_t)i)) {
        }
    }

    puts_P(PSTR("\nBinary transmission complete.\n"));
}


// Test 12
// Sustained TX without allocating a large test buffer.
static void test_sustained_tx(void)
{
    print_header("Sustained TX");

    puts_P(PSTR("Sending 512 bytes of repeating alphabet characters.\n"));

    for (uint16_t i = 0; i < 512; ++i) {
        while (!uart_transmit((uint8_t)('A' + (i % 26))));
    }

    puts_P(PSTR("\nSustained TX test complete.\n"));
}


static void print_menu(void)
{
    print_line();

    puts_P(PSTR(
        "AVR UART TEST MENU\n"
        "\n"
        "1  Basic TX\n"
        "2  Formatted output\n"
        "3  Raw TX\n"
        "4  uart_available()\n"
        "5  uart_receive()\n"
        "6  Echo\n"
        "7  TX buffer full\n"
        "8  RX buffer\n"
        "9  getchar()\n"
        "A  CRLF conversion\n"
        "B  Binary TX\n"
        "C  Sustained TX\n"
        "\n"
        "R  Run all basic tests\n"
        "M  Menu\n"
        "\n"
        "> "
    ));
}


static void execute_command(uint8_t command)
{
    switch (command) {

        case '1':
            test_basic_tx();
            break;

        case '2':
            test_formatted_output();
            break;

        case '3':
            test_raw_tx();
            break;

        case '4':
            test_available();
            break;

        case '5':
            test_receive();
            break;

        case '6':
            test_echo();
            break;

        case '7':
            test_tx_overflow();
            break;

        case '8':
            test_rx_buffer();
            break;

        case '9':
            test_getchar();
            break;

        case 'A':
        case 'a':
            test_newline();
            break;

        case 'B':
        case 'b':
            test_binary();
            break;

        case 'C':
        case 'c':
            test_sustained_tx();
            break;

        case 'R':
        case 'r':
            test_basic_tx();
            test_formatted_output();
            test_raw_tx();
            test_newline();
            break;

        case 'M':
        case 'm':
            print_menu();
            break;

        default:
            printf_P(PSTR(
                "\nUnknown command: 0x%02X\n"
            ), (unsigned)command);
            break;
    }
}


int main(void)
{
    uart_init(UART_BAUD_RATE, F_CPU);

    print_line();

    printf_P(PSTR(
        "AVR UART LIBRARY TEST\n"
        "MCU  : ATmega328P\n"
        "F_CPU: %lu Hz\n"
        "BAUD : %lu\n"
    ),
        (unsigned long)F_CPU,
        (unsigned long)UART_BAUD_RATE
    );

    puts_P(PSTR("\nUART initialized successfully.\n"));

    print_menu();

    while (1) {

        // Wait for one command.
        uint8_t command = wait_byte();

        // Echo the command.
        while (!uart_transmit(command));
        while (!uart_transmit('\n'));

        execute_command(command);

        print_menu();
    }

    return 0;
}
