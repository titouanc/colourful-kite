/*
 * Copyright (c) 2025 Titouan Christophe
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/i2c.h>

#include "seesaw.h"

#define DT_DRV_COMPAT adafruit_seesaw

#define LOG_LEVEL CONFIG_SEESAW_LOG_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(seesaw);

static const char *module_names[] = {
	[MOD_STATUS] = "STATUS",
	[MOD_GPIO] = "GPIO",
	[MOD_SERCOM0] = "SERCOM0",
	[MOD_SERCOM1] = "SERCOM1",
	[MOD_SERCOM2] = "SERCOM2",
	[MOD_SERCOM3] = "SERCOM3",
	[MOD_SERCOM4] = "SERCOM4",
	[MOD_SERCOM5] = "SERCOM5",
	[MOD_PWM] = "PWM",
	[MOD_ADC] = "ADC",
	[MOD_DAC] = "DAC",
	[MOD_INTERRUPT] = "INTERRUPT",
	[MOD_DAP] = "DAP",
	[MOD_EEPROM] = "EEPROM",
	[MOD_NEOPIXEL] = "NEOPIXEL",
	[MOD_TOUCH] = "TOUCH",
	[MOD_KEYPAD] = "KEYPAD",
	[MOD_ENCODER] = "ENCODER",
};

#define SEESAW_MODULE_NAME(_mod) \
	((_mod) < ARRAY_SIZE(module_names)) ? module_names[_mod] : "unknown"

enum seesaw_mod_status {
	STATUS_HW_ID = 0x01,
	STATUS_VERSION = 0x02,
	STATUS_OPTIONS = 0x03,
	STATUS_TEMP = 0x04,
	STATUS_SWRST = 0x7F,
};

struct seesaw_config {
	struct i2c_dt_spec i2c;
};

struct seesaw_data {
	struct k_sem sem;
	uint32_t modules;
	const struct device *claimed_modules[MOD_MAX_NUM];
};

int seesaw_claim_module(const struct device *dev, uint32_t module, const struct device *subdevice)
{
	if (!device_is_ready(dev)) {
		return -EIO;
	}
	struct seesaw_data *drv_data = dev->data;
	if (module > MOD_MAX_NUM) {
		return -EINVAL;
	}
	if (!(drv_data->modules & BIT(module))) {
		LOG_DBG("Seesaw module %s is not supported", SEESAW_MODULE_NAME(module));
		return -ENOTSUP;
	}
	if (drv_data->claimed_modules[module]) {
		LOG_DBG("Seesaw module %s is already claimed by %s", SEESAW_MODULE_NAME(module), drv_data->claimed_modules[module]->name);
		return -EBUSY;
	}
	drv_data->claimed_modules[module] = subdevice;
	LOG_DBG("Seesaw module %s claimed by %s", SEESAW_MODULE_NAME(module), drv_data->claimed_modules[module]->name);
	return 0;
}

int seesaw_cmd(const struct device *dev, enum seesaw_mod module, uint8_t reg)
{
	const struct seesaw_config *const config = dev->config;
	struct seesaw_data *drv_data = dev->data;
	uint8_t transaction[] = {module, reg};

	k_sem_take(&drv_data->sem, K_FOREVER);
	int r = i2c_write_dt(&config->i2c, transaction, sizeof(transaction));

	k_sem_give(&drv_data->sem);
	return r;
}

int seesaw_write(const struct device *dev, enum seesaw_mod module, uint8_t reg, const uint8_t *buf, size_t len)
{
	const struct seesaw_config *const config = dev->config;

	if (len > 126) {
		return -E2BIG;
	}

	uint8_t transaction[128] = {module, reg};
	memcpy(&transaction[2], buf, len);

	struct seesaw_data *drv_data = dev->data;
	k_sem_take(&drv_data->sem, K_FOREVER);

	int r = i2c_write_dt(&config->i2c, transaction, 2 + len);

	k_sem_give(&drv_data->sem);
	return r;
}

int seesaw_read(const struct device *dev, enum seesaw_mod module, uint8_t reg, uint8_t *buf, size_t len)
{
	const struct seesaw_config *const config = dev->config;
	const uint8_t request[] = {module, reg};

	struct seesaw_data *drv_data = dev->data;
	k_sem_take(&drv_data->sem, K_FOREVER);

	int r = i2c_write_dt(&config->i2c, request, sizeof(request));
	if (r == 0) {
		k_sleep(K_MSEC(1));
		r = i2c_read_dt(&config->i2c, buf, len);
	}

	k_sem_give(&drv_data->sem);
	return r;
}

static int seesaw_init(const struct device *dev)
{
	struct seesaw_data *drv_data = dev->data;

	k_sem_init(&drv_data->sem, 1, 1);

	int r = seesaw_cmd(dev, MOD_STATUS, STATUS_SWRST);
	if (r) {
		LOG_ERR("[%s] Unable to software reset", dev->name);
		return r;
	}
	k_sleep(K_MSEC(10));

	uint32_t data;
	r = seesaw_read(dev, MOD_STATUS, STATUS_OPTIONS, (uint8_t *)&data, sizeof(data));
	if (r) {
		LOG_ERR("[%s] Unable to query options", dev->name);
		return r;
	}
	drv_data->modules = sys_be32_to_cpu(data);

	LOG_DBG("[%s] Available modules: %08X", dev->name, drv_data->modules);
	for (size_t i=0; i<ARRAY_SIZE(module_names); i++) {
		if (drv_data->modules & i) {
			LOG_DBG("- %s", module_names[i]);
		}
	}

	return 0;
}

#define SEESAW_INIT(inst)                                                                          \
	static struct seesaw_data seesaw_##inst##_data;                                            \
	static const struct seesaw_config seesaw_##inst##_config = {                               \
		.i2c = I2C_DT_SPEC_INST_GET(inst),                                                 \
	};                                                                                         \
	DEVICE_DT_INST_DEFINE(inst, seesaw_init, NULL, &seesaw_##inst##_data,                         \
			      &seesaw_##inst##_config, POST_KERNEL, CONFIG_I2C_INIT_PRIORITY,      \
			      NULL);

DT_INST_FOREACH_STATUS_OKAY(SEESAW_INIT)
