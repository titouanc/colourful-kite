/*
 * Copyright (c) 2025 Titouan Christophe
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT adafruit_seesaw_pwm

#include <zephyr/drivers/pwm.h>

#include "seesaw.h"

#define LOG_LEVEL CONFIG_SEESAW_LOG_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(seesaw_pwm);

#define FIXED_CYCLES 0xffff

enum seesaw_mod_pwm {
    PWM_VAL  = 0x01,
    PWM_FREQ = 0x02,
};

struct seesaw_pwm_config {
    const struct device *seesaw;
};

static int seesaw_pwm_get_cycles_per_sec(const struct device *dev, uint32_t channel, uint64_t *cycles)
{
    *cycles = FIXED_CYCLES;
    return 0;
}

static int seesaw_pwm_set_cycles(const struct device *dev, uint32_t channel,
                uint32_t period, uint32_t pulse,
                pwm_flags_t flags)
{
    int ret;
    const struct seesaw_pwm_config *const config = dev->config;
    uint16_t freq = FIXED_CYCLES / period;
    uint16_t val = 0xffff * pulse / period;
    const uint8_t commands[2][4] = {
        {PWM_FREQ, channel, (freq >> 8), freq},
        {PWM_VAL, channel, (val >> 8), val},
    };

    for (size_t i=0; i<ARRAY_SIZE(commands); i++) {
        ret = seesaw_write(config->seesaw, MOD_PWM, commands[i][0], &commands[i][1], 3);
        if (ret) {
            return ret;
        }
    }

    return 0;
}

static DEVICE_API(pwm, seesaw_pwm_api) = {
    .set_cycles = seesaw_pwm_set_cycles,
    .get_cycles_per_sec = seesaw_pwm_get_cycles_per_sec,
};

static int seesaw_pwm_init(const struct device *dev)
{
    const struct seesaw_pwm_config *const config = dev->config;
    int r = seesaw_claim_module(config->seesaw, MOD_PWM, dev);
    if (r){
        LOG_ERR("Unable to claim pwm (%d) module: %d", MOD_PWM, r);
    }
    return r;
}

#define SEESAW_PWM_INIT(inst)                                                             \
    static const struct seesaw_pwm_config config_##inst = {                               \
        .seesaw = DEVICE_DT_GET(DT_INST_PARENT(inst)),                                     \
    };                                                                                         \
    DEVICE_DT_INST_DEFINE(inst, seesaw_pwm_init, NULL, NULL, &config_##inst, POST_KERNEL, \
                  CONFIG_I2C_INIT_PRIORITY, &seesaw_pwm_api);

DT_INST_FOREACH_STATUS_OKAY(SEESAW_PWM_INIT)
