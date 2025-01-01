# Colourful kite

External [Zephyr](https://github.com/zephyrproject-rtos/zephyr) module with device drivers and samples for fancy "maker" modules with leds, inputs, and various other features

## Usage

Installation:

```bash
# initialize my-workspace for the example-application (main branch)
west init -m https://github.com/titouanc/colourful-kite --mr main my-workspace
# update Zephyr modules
cd my-workspace
west update
```

Running a sample

```bash
west build -b $BOARD [ -S debug ] colourfulkite/samples/$SAMPLE
west flash
```

## Supported modules

### Adafruit Seesaw

> Adafruit seesaw is a near-universal converter framework which allows you to add and extend hardware support to any I2C-capable microcontroller or microcomputer. Instead of getting separate I2C GPIO expanders, ADCs, PWM drivers, etc, seesaw can be configured to give a wide range of capabilities.

At the moment, the following modules are supported:

- GPIO
- PWM
- NEOPIXEL leds
- EEPROM
- KEYPAD input keys

#### Samples for [Seesaw ATTiny817](https://learn.adafruit.com/adafruit-attiny817-seesaw)

- [act_led](samples/act_led): toggle the on-board red indicator LED
- [pwm_led](samples/pwm_rgbled): drive an LED wired to pin 0

#### Samples for [Neotrellis](https://learn.adafruit.com/adafruit-neotrellis/overview)

- [neotrellis](samples/neotrellis): illuminate keys of the neotrellis when pressed
