#include <zephyr/drivers/led.h>
#include <zephyr/input/input.h>

static const struct device *leds = DEVICE_DT_GET(DT_NODELABEL(neotrellis_leds));

static void key_event(struct input_event *evt, void *user_data)
{
    uint8_t row = evt->code >> 3;
    uint8_t col = evt->code & 0x07;
    uint8_t lednum = 4*row + col;

    if (evt->value) {
        led_on(leds, lednum);
    } else {
        led_off(leds, lednum);
    }
}

#define NEOTRELLIS_KEYPAD DEVICE_DT_GET(DT_NODELABEL(neotrellis_keypad))
INPUT_CALLBACK_DEFINE(NEOTRELLIS_KEYPAD, key_event, NULL);

int main()
{
    printk("Sample seesaw_neotrellis\n");
    return 0;
}
