# RP2040 as EPROM / EEPROM emulator 
Based on [PicoROM](https://github.com/nickbild/picoROM)

Emulates 27c512 / 28c512 with just a RP2040, without any other electronic components 

*You should use **black chinese** Raspberry Pi Pico RP2040*

Wiring:

**Address line**:<br> A0 - A15 = GPIO**0** - GPIO**15**

**Data bus**:<br> D0 - D7 = GPIO**16** - GPIO**23**

**CE/OE**:<br>
GPIO**28**
