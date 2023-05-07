# Electronic-Leadscrew-2
A re-work of the original so that it uses two stepper motors instead of a stepper and rotary encoder. Also a dividing head

## Hardware Requirements

This project is designed to run on an RP2040 board such as the Pico.
It may be modified to work on other microcontroller, but it would mean reworking it to not rely on the Pico SDK.

The steppers can be driven by most stepper drivers. This project was tested with a DM542T and a DM556 driver.

It is designed to use the TM1638 to drive 8 7-segment displays (with decimal points), along with 8 LEDs for the status.

Schematics will be published once the project is in a practically usable state, before the end of 2023.

# Building

## Dependencies

You'll need the pico SDK cloned in /usr/share/pico-sdk.

Make sure there are submodules or USB output won't work!

## Build process

```
mkdir build
cd build
cmake ..
```

Once that's done once, you can just `cd build` then `make`

## Flashing

Output appears in build/leadscrew/main.uf2 - copy this to the Pi Pico when it's in bootsel mode.
