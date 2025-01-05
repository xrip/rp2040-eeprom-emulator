#include <hardware/gpio.h>
#include <hardware/clocks.h>
#include <hardware/vreg.h>
#include <stdio.h>
#include <pico/multicore.h>
#include <pico/time.h>


#include "pin_definitions.h"
#include "rom.h"

//#define DEBUG 1
#define HW_SN76489

#if !defined(HW_SN76489)
#include "sn76489.c"
#include <hardware/pwm.h>
#endif


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

#if defined(HW_SN76489)
    gpio_init(SN_CS_PIN);
    gpio_set_dir(SN_CS_PIN, GPIO_OUT);
    gpio_put(SN_CS_PIN, 1);
#endif
}

#if !defined(HW_SN76489)
volatile uint8_t sn_byte = 0;

void second_core() {
    pwm_config pwm = pwm_get_default_config();
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);

    pwm_config_set_clkdiv(&pwm, 1.0f);
    pwm_config_set_wrap(&pwm, 4095); // MAX PWM value

    pwm_init(pwm_gpio_to_slice_num(PWM_PIN), &pwm, true);

    sn76489_reset();

    uint8_t last_sn_byte = 0;
    while (1) {
        sleep_us(50);
        if (last_sn_byte != sn_byte) {
            last_sn_byte = sn_byte;

            sn76489_out(sn_byte);
            const int16_t sample = sn76489_sample();

            pwm_set_gpio_level(PWM_PIN, (uint16_t) ((int32_t) sample + 0x8000L) >> 4);
        }
    }
}
#endif

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

#if !defined(HW_SN76489)
    multicore_launch_core1(second_core);
#endif

    // Listen the BUS
    while (1) {
        const uint8_t RD = !gpio_get(RD_PIN);
        const uint8_t WR = !gpio_get(WR_PIN);
        const uint8_t IORQ = !gpio_get(IORQ_PIN);

        if (!IORQ) {
            if (RD) {
                const uint32_t bus = gpio_get_all();
                const uint16_t address = bus & ADDRESS_BUS_MASK;
                gpio_set_dir_out_masked(DATA_BUS_MASK);
                gpio_put_masked(DATA_BUS_MASK, MEMORY[address] << 16);
            } else if (WR) {
                const uint32_t bus = gpio_get_all();
                const uint16_t address = bus & ADDRESS_BUS_MASK;
                gpio_set_dir_in_masked(DATA_BUS_MASK);
                MEMORY[address] = (uint8_t) (bus >> 16);
            } else {
                gpio_set_dir_in_masked(DATA_BUS_MASK);
            }
        } else {
            gpio_set_dir_in_masked(DATA_BUS_MASK);

            const uint32_t bus = gpio_get_all();

            if (WR && bus & 0x40) {
#if defined(HW_SN76489)
                gpio_put(SN_CS_PIN, 0);
                sleep_us(10);
                gpio_put(SN_CS_PIN, 1);
#else
                sn_byte = bus >> 16 & 0xff;
#endif
            }
        }


        tight_loop_contents();
    }

    __unreachable();
}
