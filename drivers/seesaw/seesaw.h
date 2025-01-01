/*
 * Copyright (c) 2025 Titouan Christophe
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef DEFINE_NEOTRELLIS_HEADER
#define DEFINE_NEOTRELLIS_HEADER

#include <stdint.h>
#include <stdlib.h>
#include <zephyr/device.h>
#include <zephyr/sys/byteorder.h>

/* See https://learn.adafruit.com/adafruit-seesaw-atsamd09-breakout/using-the-seesaw-platform */
enum seesaw_mod {
	MOD_STATUS 	= 0x00,
	MOD_GPIO 	= 0x01,
	MOD_SERCOM0 	= 0x02,
	MOD_SERCOM1 	= 0x03,
	MOD_SERCOM2 	= 0x04,
	MOD_SERCOM3 	= 0x05,
	MOD_SERCOM4 	= 0x06,
	MOD_SERCOM5 	= 0x07,
	MOD_PWM 	= 0x08,
	MOD_ADC 	= 0x09,
	MOD_DAC 	= 0x0a,
	MOD_INTERRUPT 	= 0x0b,
	MOD_DAP 	= 0x0c,
	MOD_EEPROM 	= 0x0d,
	MOD_NEOPIXEL 	= 0x0e,
	MOD_TOUCH 	= 0x0f,
	MOD_KEYPAD 	= 0x10,
	MOD_ENCODER 	= 0x11,
	MOD_MAX_NUM,
};

struct seesaw_keypad_event {
	unsigned edge: 2;
	unsigned col : 3;
	unsigned row : 3;
} __attribute__ ((__packed__));

int seesaw_claim_module(const struct device *dev, uint32_t module, const struct device *subdevice);

int seesaw_cmd(const struct device *dev, enum seesaw_mod module, uint8_t reg);

int seesaw_write(const struct device *dev, enum seesaw_mod module, uint8_t reg, const uint8_t *buf, size_t len);

int seesaw_read(const struct device *dev, enum seesaw_mod module, uint8_t reg, uint8_t *buf, size_t len);

#define sys_be8_to_cpu(x) x
#define sys_cpu_to_be8(x) x
#define UT(size) uint##size##_t
#define T(size) int##size##_t

#define ACCESSOR(size) \
	static inline UT(size) seesaw_read_uint##size(const struct device *dev, enum seesaw_mod module, uint8_t reg) \
	{ \
		UT(size) buf; \
		seesaw_read(dev, module, reg, (uint8_t *) &buf, size/8); \
		return sys_be##size##_to_cpu(buf); \
	} \
	static inline T(size) seesaw_read_int##size(const struct device *dev, enum seesaw_mod module, uint8_t reg) \
	{return seesaw_read_uint##size(dev, module, reg);} \
	static inline void seesaw_write_uint##size(const struct device *dev, enum seesaw_mod module, uint8_t reg, UT(size) val) \
	{ \
		const UT(size) buf = sys_cpu_to_be##size(val); \
		seesaw_write(dev, module, reg, (const uint8_t *) &buf, size/8); \
	} \
	static inline void seesaw_write_int##size(const struct device *dev, enum seesaw_mod module, uint8_t reg, T(size) val) \
	{seesaw_write_uint##size(dev, module, reg, val);}

ACCESSOR(8)
ACCESSOR(16)
ACCESSOR(32)

#endif
