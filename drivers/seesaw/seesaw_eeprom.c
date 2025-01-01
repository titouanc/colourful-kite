/*
 * Copyright (c) 2025 Titouan Christophe
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT adafruit_seesaw_eeprom

#include "seesaw.h"
#include <zephyr/drivers/eeprom.h>

#define LOG_LEVEL CONFIG_EEPROM_LOG_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(seesaw_eeprom);


struct seesaw_eeprom_config {
	const struct device *seesaw;
	size_t size;
};

static int seesaw_eeprom_read(const struct device *dev, off_t offset,
				void *buf,
				size_t len)
{
	const struct seesaw_eeprom_config *config = dev->config;
	if (offset + len > config->size) {
		return -EINVAL;
	}
	return seesaw_read(config->seesaw, MOD_EEPROM, offset, (uint8_t *) buf, len);
}

static int seesaw_eeprom_write(const struct device *dev, off_t offset,
				const void *buf, size_t len)
{
	const struct seesaw_eeprom_config *config = dev->config;
	if (offset + len > config->size) {
		return -EINVAL;
	}
	return seesaw_write(config->seesaw, MOD_EEPROM, offset, (const uint8_t *) buf, len);
}

static size_t seesaw_eeprom_size(const struct device *dev)
{
	const struct seesaw_eeprom_config *config = dev->config;
	return config->size;
}

static int seesaw_eeprom_init(const struct device *dev)
{
	const struct seesaw_eeprom_config *config = dev->config;
	int r = seesaw_claim_module(config->seesaw, MOD_EEPROM, dev);
	if (r){
		LOG_ERR("Unable to claim eeprom (%d) module: %d", MOD_EEPROM, r);
	}
	return r;
}

static const struct eeprom_driver_api seesaw_eeprom_api = {
	.read = seesaw_eeprom_read,
	.write = seesaw_eeprom_write,
	.size = seesaw_eeprom_size,
};

#define SEESAW_EEPROM_INIT(inst)					\
	static const struct seesaw_eeprom_config config_##inst = {	\
		.seesaw = DEVICE_DT_GET(DT_INST_PARENT(inst)),		\
		.size = DT_INST_PROP(inst, size),			\
	};								\
	DEVICE_DT_INST_DEFINE(inst, seesaw_eeprom_init, NULL, NULL,	\
			      &config_##inst, POST_KERNEL,		\
			      CONFIG_EEPROM_INIT_PRIORITY,		\
			      &seesaw_eeprom_api);

DT_INST_FOREACH_STATUS_OKAY(SEESAW_EEPROM_INIT)
