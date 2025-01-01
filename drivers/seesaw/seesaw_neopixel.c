/*
 * Copyright (c) 2025 Titouan Christophe
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT adafruit_seesaw_neopixel

#include <zephyr/drivers/led.h>
#include <zephyr/dt-bindings/led/led.h>

#include "seesaw.h"

#define LOG_LEVEL CONFIG_SEESAW_LOG_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(seesaw_neopixel);

enum seesaw_mod_neopixel {
	NEOPIXEL_PIN = 0x01,
	NEOPIXEL_SPEED = 0x02,
	NEOPIXEL_BUF_LENGTH = 0x03,
	NEOPIXEL_BUF = 0x04,
	NEOPIXEL_SHOW = 0x05,
};

struct seesaw_neopixel_config {
	const struct device *seesaw;
	size_t length;
	const uint8_t *color_mapping;
	uint8_t num_colors;
	uint8_t output_pin;
};

static int seesaw_neopixel_set_color(const struct device *dev, uint32_t led, uint8_t num_colors,
				     const uint8_t *color)
{
	const struct seesaw_neopixel_config *config = dev->config;
	if (led >= config->length || num_colors != config->num_colors) {
		return -EINVAL;
	}

	uint16_t offset = config->num_colors * led;
	uint8_t buf[6] = {offset >> 8, offset & 0xff};
	memcpy(buf + 2, color, num_colors);

	int r = seesaw_write(config->seesaw, MOD_NEOPIXEL, NEOPIXEL_BUF, buf, 2 + num_colors);
	if (r) {
		LOG_ERR("[%s] Unable to write color buffer: %d", dev->name, r);
		return r;
	}
	r = seesaw_cmd(config->seesaw, MOD_NEOPIXEL, NEOPIXEL_SHOW);
	if (r) {
		LOG_ERR("[%s] Unable to command show: %d", dev->name, r);
	}
	return r;
}

static int seesaw_neopixel_set_brightness(const struct device *dev, uint32_t led, uint8_t value)
{
	const struct seesaw_neopixel_config *config = dev->config;
	if (value > 100) {
		return -EINVAL;
	}
	uint8_t b = 255 * value / 100;
	uint8_t color[] = {b, b, b, b};
	return seesaw_neopixel_set_color(dev, led, config->num_colors, color);
}

static int seesaw_neopixel_on(const struct device *dev, uint32_t led)
{
	return seesaw_neopixel_set_brightness(dev, led, 100);
}

static int seesaw_neopixel_off(const struct device *dev, uint32_t led)
{
	return seesaw_neopixel_set_brightness(dev, led, 0);
}

static int seesaw_neopixel_init(const struct device *dev)
{
	const struct seesaw_neopixel_config *config = dev->config;
	int r = seesaw_claim_module(config->seesaw, MOD_NEOPIXEL, dev);
	if (r) {
		LOG_ERR("Unable to claim neopixel module: %d", r);
		return r;
	}

	seesaw_write_int8(config->seesaw, MOD_NEOPIXEL, NEOPIXEL_PIN, config->output_pin);
	seesaw_write_uint16(config->seesaw, MOD_NEOPIXEL, NEOPIXEL_BUF_LENGTH, 3 * config->length);
	return 0;
}

static const struct led_driver_api seesaw_neopixel_api = {
	.on = seesaw_neopixel_on,
	.off = seesaw_neopixel_off,
	.set_brightness = seesaw_neopixel_set_brightness,
	.set_color = seesaw_neopixel_set_color,
};

#define SEESAW_NEOPIXEL_INIT(inst)                                                                 \
	static const uint8_t cmap_##idx[] = DT_INST_PROP(inst, color_mapping);                     \
	static const struct seesaw_neopixel_config config_##inst = {                               \
		.seesaw = DEVICE_DT_GET(DT_INST_PARENT(inst)),                                     \
		.length = DT_INST_PROP(inst, chain_length),                                        \
		.color_mapping = cmap_##idx,                                                       \
		.num_colors = sizeof(cmap_##idx),                                                  \
		.output_pin = DT_INST_PROP(inst, output_pin),                                      \
	};                                                                                         \
	DEVICE_DT_INST_DEFINE(inst, seesaw_neopixel_init, NULL, NULL, &config_##inst, POST_KERNEL, \
			      CONFIG_LED_INIT_PRIORITY, &seesaw_neopixel_api);

DT_INST_FOREACH_STATUS_OKAY(SEESAW_NEOPIXEL_INIT)
