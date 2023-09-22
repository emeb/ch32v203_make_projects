# SPI DAC Example
This is a pseudo I2S setup that can drive a PT8211 audio DAC at 48kHz. Connect
the PT8211 as follows:

* PA5 - BCLK
* PA7 - SD
* PA11 - WS

Sine and sawtooth waves will be output on the right and left channel pins.

## Coding style test
This example includes two different coding styles for the the periphera setup
which can be selected by uncommenting a cpp macro `WCH_PERIPH_API` at the
beginning of the `spi_dac.c` file:

* Macro disabled - does direct writes to the peripheral registers using only
the register and bit definitions from the ch32v20x.h file. This results in a
binary that's 11168 bytes
* Macro enabled (uncommented) - uses the WCH peripheral API to do the setup.
This results in a binary that's slightly larger at 11392. 

The conclusion is that there is not a lot of advantage in size when using the
direct register write approach. Using the WCH peripheral API is arguably more
readable and more portable when changing devices.