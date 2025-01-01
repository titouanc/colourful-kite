/*
 * Copyright (c) 2025 Titouan Christophe
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT adafruit_seesaw_keypad

#include <zephyr/input/input.h>
#include <zephyr/kernel.h>

#include "seesaw.h"

#define LOG_LEVEL CONFIG_SEESAW_LOG_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(seesaw_keypad);

enum {
	KEYPAD_STATUS = 0x00,
	KEYPAD_EVENT = 0x01,
	KEYPAD_INTENSET = 0x02,
	KEYPAD_INTENCLR = 0x03,
	KEYPAD_COUNT = 0x04,
	KEYPAD_FIFO = 0x10,
};

enum key_edge {
	EDGE_HIGH = 0,
	EDGE_LOW = 1,
	EDGE_FALLING = 2,
	EDGE_RISING = 3,
};

struct seesaw_keypad_config {
	const struct device *seesaw;
};

struct seesaw_keypad_data {
	const struct device *dev;
	struct k_work_delayable work;
};

static int seesaw_keypad_get_event(const struct device *dev)
{
	const struct seesaw_keypad_config *config = dev->config;

	uint8_t keypad_event;
	int r = seesaw_read(config->seesaw, MOD_KEYPAD, KEYPAD_FIFO, &keypad_event, 1);
	if (r < 0) {
		LOG_ERR("Error polling events FIFO from %s: %d", dev->name, r);
		return r;
	}

	// EOF
	if (keypad_event == 0xff) {
		return 0;
	}

	input_report(dev, INPUT_EV_KEY, (keypad_event >> 2), (keypad_event & 1), true, K_NO_WAIT);
	return 1;
}

static void seesaw_keypad_poll(struct k_work *work)
{
	struct k_work_delayable *dwork = k_work_delayable_from_work(work);
	struct seesaw_keypad_data *drv_data = CONTAINER_OF(dwork, struct seesaw_keypad_data, work);

	while (seesaw_keypad_get_event(drv_data->dev) > 0)
		;

	k_work_reschedule(dwork, K_MSEC(CONFIG_SEESAW_INPUT_KEYPAD_POLL_INTERVAL));
}

static int seesaw_keypad_init(const struct device *dev)
{
	const struct seesaw_keypad_config *config = dev->config;
	int r = seesaw_claim_module(config->seesaw, MOD_KEYPAD, dev);
	if (r) {
		LOG_ERR("Unable to claim keypad module: %d", r);
		return r;
	}

	LOG_DBG("Initializing %s...", dev->name);

	struct seesaw_keypad_data *drv_data = dev->data;
	drv_data->dev = dev;
	k_work_init_delayable(&drv_data->work, seesaw_keypad_poll);

	// Enable RISE/FALL events on all keys
	uint8_t edges = BIT(EDGE_RISING) | BIT(EDGE_FALLING);
	for (int i = 0; i < 16; i++) {
		uint8_t buf[] = {
			(i & 0xc) << 1 | (i & 0x03),
			(edges << 1) | 1,
		};
		r = seesaw_write(config->seesaw, MOD_KEYPAD, KEYPAD_EVENT, buf, sizeof(buf));
		if (r) {
			LOG_ERR("Unable to configure keypad event: %d", r);
			return r;
		}
	}

	k_work_reschedule(&drv_data->work, K_MSEC(CONFIG_SEESAW_INPUT_KEYPAD_POLL_INTERVAL));
	LOG_DBG("%s initialized", dev->name);
	return 0;
}

#define SEESAW_KEYPAD_INIT(inst)                                                                   \
	static struct seesaw_keypad_data data_##inst;                                              \
	static const struct seesaw_keypad_config config_##inst = {                                 \
		.seesaw = DEVICE_DT_GET(DT_INST_PARENT(inst)),                                     \
	};                                                                                         \
	DEVICE_DT_INST_DEFINE(inst, seesaw_keypad_init, NULL, &data_##inst, &config_##inst,        \
			      POST_KERNEL, CONFIG_INPUT_INIT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(SEESAW_KEYPAD_INIT)
