/*
 * Copyright (c) 2024 Custom Vendor
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT custom_led_sensor

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>

struct our_led_sensor_data {
	bool led_on;
};

struct our_led_sensor_config {
	struct gpio_dt_spec gpio;
};

static int our_led_sensor_sample_fetch(const struct device *dev,
				       enum sensor_channel chan)
{
	const struct our_led_sensor_config *cfg = dev->config;
	struct our_led_sensor_data *data = dev->data;
	int ret;

	ret = gpio_pin_set_dt(&cfg->gpio, 1);  /* LED ON */
	if (ret == 0) {
		data->led_on = true;
	}

	return ret;
}

static int our_led_sensor_channel_get(const struct device *dev,
				      enum sensor_channel chan,
				      struct sensor_value *val)
{
	const struct our_led_sensor_config *cfg = dev->config;
	struct our_led_sensor_data *data = dev->data;
	int ret;

	ret = gpio_pin_set_dt(&cfg->gpio, 0);  /* LED OFF */
	if (ret == 0) {
		data->led_on = false;
	}

	return ret;
}

static DEVICE_API(sensor, our_led_sensor_api) = {
	.sample_fetch = our_led_sensor_sample_fetch,
	.channel_get = our_led_sensor_channel_get,
};

static int our_led_sensor_init(const struct device *dev)
{
	const struct our_led_sensor_config *cfg = dev->config;

	if (!gpio_is_ready_dt(&cfg->gpio)) {
		return -ENODEV;
	}

	return gpio_pin_configure_dt(&cfg->gpio, GPIO_OUTPUT_INACTIVE);
}

int our_led_sensor_toggle(const struct device *dev)
{
	struct our_led_sensor_data *data = dev->data;
	const struct our_led_sensor_config *cfg = dev->config;
	int ret;

	if (data->led_on) {
		ret = gpio_pin_set_dt(&cfg->gpio, 0);
		if (ret == 0) {
			data->led_on = false;
		}
	} else {
		ret = gpio_pin_set_dt(&cfg->gpio, 1);
		if (ret == 0) {
			data->led_on = true;
		}
	}

	return ret;
}

#define OUR_LED_SENSOR_DEFINE(inst)                                       \
	static struct our_led_sensor_data our_led_sensor_data_##inst;         \
	static const struct our_led_sensor_config our_led_sensor_cfg_##inst = { \
		.gpio = GPIO_DT_SPEC_INST_GET(inst, gpios),                     \
	};                                                                    \
	DEVICE_DT_INST_DEFINE(inst, our_led_sensor_init, NULL,               \
		      &our_led_sensor_data_##inst,                                \
		      &our_led_sensor_cfg_##inst,                                 \
		      POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,                   \
		      &our_led_sensor_api);

DT_INST_FOREACH_STATUS_OKAY(OUR_LED_SENSOR_DEFINE)
