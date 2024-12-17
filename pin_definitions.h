#pragma once

#define ADDRESS_BUS_WIDTH 16
#define ADDRESS_BUS_MASK ((1 << ADDRESS_BUS_WIDTH)-1)

#define DATA_BUS_MASK (0xFF0000)
// Address bus start GPIO pin
#define A0 0

// Data bus start GPIO pin
#define D0 16

// OE/CE pin
#define CE_PIN 28