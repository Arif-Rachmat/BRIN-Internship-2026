# **AVR Millis Library**

Lightweight, non-blocking millisecond timekeeping library for the AVR microcontroller family (ATmega328P, ATmega, etc.). Designed for standard **C** and **C++** bare-metal development workflows, based on [AVR CMake Development Template](https://github.com/Arif-Rachmat/AVR-CMake-Template). This library provides an interrupt-safe 32-bit millisecond counter similar to Arduino's `millis()` function.

---

## **Table of content 📋**

- [**AVR Millis Library**](#avr-millis-library)
  - [**Table of content 📋**](#table-of-content-)
  - [**Prerequisites❗**](#prerequisites)
  - [**Quick Starts🛠️**](#quick-starts️)
  - [**Usage Example 💡**](#usage-example-)
  - [**API Reference 📖**](#api-reference-)
    - [`void millis_init(void)`](#void-millis_initvoid)
    - [`uint32_t millis(void)`](#uint32_t-millisvoid)
  - [**Hardware Configuration ⚙️**](#hardware-configuration-️)
  - [**To-Do List 📌**](#to-do-list-)
  - [**License 📜**](#license-)

---

## **Prerequisites❗**

This library is specifically built on top of and designed to integrate with the **[AVR-CMake-Template](https://github.com/Arif-Rachmat/AVR-CMake-Template)** repository. For smooth development and compilation, ensure your project is built using that template as its base.

Your host environment must meet the base project toolchain requirements:

* **Base Project Template**: **[AVR-CMake-Template](https://github.com/Arif-Rachmat/AVR-CMake-Template)** (verify your main application setup follows this structure).
* **AVR Toolchain**: [Microchip AVR Toolchain](https://www.microchip.com/en-us/tools-resources/develop/microchip-studio/gcc-compilers) (`avr-gcc`, `binutils-avr`, and `avr-libc`).
* **Build System**: [CMake](https://cmake.org/download/) (v3.16+) and a build generator like [Ninja](https://ninja-build.org/) or GNU [Make].

> **Note**: For platform-specific toolchain installation steps (Windows/MSYS2 UCRT64, Linux/Debian/Ubuntu, or macOS/Homebrew), please refer directly to the **[AVR-CMake-Template Prerequisites](https://github.com/Arif-Rachmat/AVR-CMake-Template#prerequisites)** section.

---

## **Quick Starts🛠️**

Simply Clone this repository into your project's `lib/` directory:

```bash
git clone https://github.com/Arif-Rachmat-AVR/avr-millis.git lib/millis
```

Then include the header file to you project files just like any C/C++ library:
```C
#include "millis.h"

int main(){
    millis_init();
    for(;;){
        unsigned int current_time = millis();
    }
}
```

---

## **Usage Example 💡**

The following snippet demonstrates how to perform non-blocking timing operations (e.g., blinking an LED without using delay functions):

```c
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
```

---

## **API Reference 📖**

### `void millis_init(void)`

Configures hardware **Timer0** into CTC mode, sets the prescaler to 64, enables the compare match interrupt, and globally enables interrupts via `sei()`. Must be called once during startup before invoking `millis()`.

### `uint32_t millis(void)`

Returns the number of milliseconds elapsed since `millis_init()` was called.

* **Return Value**: `uint32_t` — Time elapsed in milliseconds.
* **Thread Safety**: Atomic read operation safe from interrupt preemption.
* **Overflow Period**: Rollover occurs approximately every 49.7 days.

---

## **Hardware Configuration ⚙️**

By default, the library calculations target an **ATmega328P running at 16 MHz**:

| Parameter | Configuration | Notes |
| --- | --- | --- |
| **Timer Hardware** | Timer0| Uses `TIMER0_COMPA_vect`|
| **Timer Mode** | CTC (Clear Timer on Compare Match) | Mode 2 (`WGM01 = 1`) |
| **Prescaler** | 64 | `CS01` and `CS00` set |
| **Compare Match Value** | 249 (`OCR0A`) | Yields 1000 Hz / 1 ms interrupt rate at 16 MHz |

> **Note**: Calling `millis_init()` automatically executes `sei()` to enable global interrupts. If your application requires global interrupts disabled during initialization, call `millis_init()` before critical setup sections and re-enable global interrupts manually.

---

## **To-Do List 📌**

- [ ] **Dynamic Clock Frequency Support (`F_CPU`)**: Implement macro-based prescaler and `OCR` calculations to support flexible clock speeds (e.g., 8 MHz, 1 MHz, 20 MHz) instead of hardcoded 16 MHz values.
- [ ] **Expanded Hardware Timer Support**: Add preprocessor options to allow switching the underlying timer hardware (e.g., Timer1, Timer2) to prevent resource conflicts with other libraries.
- [ ] **Broader AVR MCU Support**: Test, validate, and document compatibility across additional AVR chip families (ATtiny series, ATmega2560, megaAVR 0-series).
- [ ] **Microsecond Timekeeping (`micros()`)**: Add a non-blocking `micros()` routine for high-precision sub-millisecond timing operations.
- [ ] **Non-blocking Timeout Helper Macros**: Provide utility macros/inline functions (e.g., `millis_has_elapsed()`) to simplify periodic task timing code.

---

## **License 📜**

This project is licensed under the [MIT License](/LICENSE) — free for both personal and commercial use.