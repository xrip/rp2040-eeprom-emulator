#include <hardware/gpio.h>
#include <hardware/clocks.h>
#include <hardware/vreg.h>
#include <stdio.h>
#include <pico/time.h>

#include "pin_definitions.h"
#include "rom.h"

//#define DEBUG 1

// Pointer needed if we want implement some sort of mapper that adds offset ot rom start. 
uint8_t *MEMORY;

static inline void setup_gpio_pins() {
    gpio_init_mask(ADDRESS_BUS_MASK | DATA_BUS_MASK);

    gpio_set_dir_in_masked(ADDRESS_BUS_MASK);
    gpio_set_dir_out_masked(DATA_BUS_MASK);

    gpio_init(RD_PIN);
    gpio_set_dir(RD_PIN, GPIO_IN);

    gpio_init(WR_PIN);
    gpio_set_dir(WR_PIN, GPIO_IN);

    gpio_init(IORQ_PIN);
    gpio_set_dir(IORQ_PIN, GPIO_IN);
}

int main() {
    // Set system clock speed.
    hw_set_bits(&vreg_and_chip_reset_hw->vreg, VREG_AND_CHIP_RESET_VREG_VSEL_BITS);
    sleep_us(250);
    set_sys_clock_khz(372 * 1000, true);

#if DEBUG
    uint8_t led = 0;

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
#endif
    setup_gpio_pins();

    MEMORY = rom;

    // Listen the BUS
    while (1) {
        const uint8_t RD = !gpio_get(RD_PIN);
        const uint8_t WR = !gpio_get(WR_PIN);
        const uint8_t IORQ = !gpio_get(IORQ_PIN);

        if (!IORQ) {
            const uint32_t bus = gpio_get_all();
            const uint16_t address = bus & ADDRESS_BUS_MASK;
            if (RD) {
                gpio_set_dir_out_masked(DATA_BUS_MASK);
                gpio_put_masked(DATA_BUS_MASK, MEMORY[address] << 16);
            } else if (WR) {
                gpio_set_dir_in_masked(DATA_BUS_MASK);
                MEMORY[address] = bus >> 16;
            } else {
                gpio_set_dir_in_masked(DATA_BUS_MASK);
            }
        } else {
            gpio_set_dir_in_masked(DATA_BUS_MASK);
        }

        tight_loop_contents();
    }

    __unreachable();
}
