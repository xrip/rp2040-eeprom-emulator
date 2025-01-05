#pragma once

#define ADDRESS_BUS_WIDTH 16
#define ADDRESS_BUS_MASK ((1 << ADDRESS_BUS_WIDTH)-1)

#define DATA_BUS_MASK (0xFF0000)
// Address bus start GPIO pin
#define A0 0

// Data bus start GPIO pin
#define D0 16

#define SN_CS_PIN 26
#define PWM_PIN 26

#define WR_PIN 27
#define RD_PIN 28
#define IORQ_PIN 29

