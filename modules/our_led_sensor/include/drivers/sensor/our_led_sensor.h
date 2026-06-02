/*
 * Copyright (c) 2024 Custom Vendor
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef OUR_LED_SENSOR_H
#define OUR_LED_SENSOR_H

#include <zephyr/device.h>

/**
 * @brief Toggle the LED state (custom extension API for Task 2)
 */
int our_led_sensor_toggle(const struct device *dev);

#endif /* OUR_LED_SENSOR_H */
