#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define LED_SENSOR_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(custom_led_sensor)

int main(void)
{
	const struct device *dev = DEVICE_DT_GET(LED_SENSOR_NODE);

	if (!device_is_ready(dev)) {
		LOG_ERR("LED sensor device not ready");
		return 0;
	}

	LOG_INF("LED sensor ready");

	/* Task 1: fetch = ON, channel_get = OFF */
	sensor_sample_fetch(dev);
	LOG_INF("LED should be ON after sample_fetch");
	k_msleep(1000);

	sensor_channel_get(dev, SENSOR_CHAN_ALL, NULL);
	LOG_INF("LED should be OFF after channel_get");
	k_msleep(1000);

	while (1) {
		k_msleep(1000);
	}

	return 0;
}
