#include <hardware/gpio.h>
#include <hardware/clocks.h>
#include <hardware/vreg.h>
#include <stdio.h>
#include <pico/time.h>

#include "pin_definitions.h"
#include "rom.h"

//#define DEBUG 1

// Pointer needed if we want implement some sort of mapper that adds offset ot rom start. 
uint8_t *rom_pointer;

static inline uint16_t get_requested_address() {
    return gpio_get_all() & ADDRESS_BUS_MASK;
}

static inline void put_data_on_bus(const uint16_t address) {
    gpio_put_masked(0xFF0000, rom_pointer[address] << 16);
}

static inline void setup_gpio_pins() {
    for (uint8_t i = 0; i < ADDRESS_BUS_WIDTH; i++) {
        gpio_init(A0 + i);
        gpio_set_dir(A0 + i, GPIO_IN);
    }

    for (uint8_t i = 0; i < 8; i++) {
        gpio_init(D0 + i);
        gpio_set_dir(D0 + i, GPIO_OUT);
    }

    gpio_init(CE_PIN);
    gpio_set_dir(CE_PIN, GPIO_IN);
}


int main() {
    // Set system clock speed.
    hw_set_bits(&vreg_and_chip_reset_hw->vreg, VREG_AND_CHIP_RESET_VREG_VSEL_BITS);
    sleep_us(250);
    set_sys_clock_khz(372 * 1000, true);

#if DEBUG
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
#endif

    setup_gpio_pins();

    rom_pointer = rom;

    // Listen the BUS
    while (1) {
        const uint8_t output_enabled = !gpio_get(CE_PIN); // Check if your CE/OE signal isn't inverted

#if DEBUG
        gpio_put(PICO_DEFAULT_LED_PIN, !output_enabled);
#endif

        if (output_enabled) {
            gpio_set_dir_out_masked(DATA_BUS_PINS);
            put_data_on_bus(get_requested_address());
        } else {
            gpio_set_dir_in_masked(DATA_BUS_PINS);
        }

        tight_loop_contents();
    }

    __unreachable();
}
