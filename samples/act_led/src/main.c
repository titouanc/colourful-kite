#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_NODELABEL(seesaw_red_led), gpios);

int main()
{
    bool on = false;
    printk("Sample seesaw_act_led\n");

    if (! device_is_ready(led.port)) {
        printk("Device is not ready !\n");
        return -1;
    }

    if (gpio_pin_configure_dt(&led, GPIO_OUTPUT)) {
        printk("Unable to configure as output");
        return -1;
    }

    while (1) {
        on = !on;
        gpio_pin_set_dt(&led, on);
        k_sleep(K_MSEC(500));
    }
}
