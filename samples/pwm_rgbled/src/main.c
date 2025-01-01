#include <zephyr/kernel.h>
#include <zephyr/drivers/led.h>

const struct led_dt_spec pwm_led = LED_DT_SPEC_GET(DT_NODELABEL(pwm_led));

#define MAX_BRIGHTNESS 30

int main()
{
    uint32_t t = 0;

    printk("Sample seesaw_pwm_led\n");

    while (1) {
        led_set_brightness_dt(&pwm_led, t % MAX_BRIGHTNESS);
        k_sleep(K_MSEC(50));
        t++;
    }
}
