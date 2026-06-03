/**
 * L7 Shell Commands for our_led_sensor driver
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/shell/shell.h>

#include <drivers/sensor/our_led_sensor.h>

#define LED_SENSOR_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(custom_led_sensor)

/* --------------------------------------------------------------------------
 * Shared device helper
 * -------------------------------------------------------------------------- */
static const struct device *get_sensor_dev(void)
{
	const struct device *dev = DEVICE_DT_GET(LED_SENSOR_NODE);

	if (!device_is_ready(dev)) {
		return NULL;
	}
	return dev;
}

/* --------------------------------------------------------------------------
 * sensor fetch -- calls sensor_sample_fetch(dev)
 * -------------------------------------------------------------------------- */
static int cmd_sensor_fetch(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	const struct device *dev = get_sensor_dev();

	if (dev == NULL) {
		shell_error(sh, "Sensor device not ready");
		return -ENODEV;
	}

	int ret = sensor_sample_fetch(dev);

	if (ret != 0) {
		shell_error(sh, "sensor_sample_fetch failed: %d", ret);
		return ret;
	}

	shell_print(sh, "sensor_sample_fetch OK — LED should be ON");
	return 0;
}

/* --------------------------------------------------------------------------
 * sensor read -- calls sensor_channel_get(dev, SENSOR_CHAN_ALL, NULL)
 * -------------------------------------------------------------------------- */
static int cmd_sensor_read(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	const struct device *dev = get_sensor_dev();

	if (dev == NULL) {
		shell_error(sh, "Sensor device not ready");
		return -ENODEV;
	}

	int ret = sensor_channel_get(dev, SENSOR_CHAN_ALL, NULL);

	if (ret != 0) {
		shell_error(sh, "sensor_channel_get failed: %d", ret);
		return ret;
	}

	shell_print(sh, "sensor_channel_get OK — LED should be OFF");
	return 0;
}

/* --------------------------------------------------------------------------
 * sensor info -- prints device name and ready state
 * -------------------------------------------------------------------------- */
static int cmd_sensor_info(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	const struct device *dev = get_sensor_dev();

	shell_print(sh, "Device name : %s", dev ? dev->name : "(null)");
	shell_print(sh, "Ready state : %s", (dev && device_is_ready(dev)) ? "ready" : "not ready");
	return 0;
}

/* --------------------------------------------------------------------------
 * sensor set <value> -- calls our_led_sensor_toggle() when value == toggle
 *                         or controls LED directly with 0/1
 * -------------------------------------------------------------------------- */
static int cmd_sensor_set(const struct shell *sh, size_t argc, char **argv)
{
	if (argc != 2) {
		shell_error(sh, "Usage: sensor set <0|1|toggle>");
		return -EINVAL;
	}

	const struct device *dev = get_sensor_dev();

	if (dev == NULL) {
		shell_error(sh, "Sensor device not ready");
		return -ENODEV;
	}

	if (strcmp(argv[1], "toggle") == 0) {
		int ret = our_led_sensor_toggle(dev);
		if (ret != 0) {
			shell_error(sh, "our_led_sensor_toggle failed: %d", ret);
			return ret;
		}
		shell_print(sh, "LED toggled via custom API");
		return 0;
	}

	/* Expect numeric 0 or 1 */
	char *endptr;
	long val = strtol(argv[1], &endptr, 10);

	if ((*endptr != '\0') || (val != 0 && val != 1)) {
		shell_error(sh, "Invalid argument: '%s'. Expected 0, 1, or toggle.", argv[1]);
		return -EINVAL;
	}

	/* Use the public sensor API to set LED state */
	int ret;

	if (val == 1) {
		ret = sensor_sample_fetch(dev);
	} else {
		ret = sensor_channel_get(dev, SENSOR_CHAN_ALL, NULL);
	}

	if (ret != 0) {
		shell_error(sh, "Failed to set LED state: %d", ret);
		return ret;
	}

	shell_print(sh, "LED set to %s", val ? "ON" : "OFF");
	return 0;
}

/* --------------------------------------------------------------------------
 * Subcommand table
 * -------------------------------------------------------------------------- */
SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_sensor,
	SHELL_CMD(fetch, NULL, "Call sensor_sample_fetch() (turns LED ON)", cmd_sensor_fetch),
	SHELL_CMD(read, NULL, "Call sensor_channel_get() (turns LED OFF)", cmd_sensor_read),
	SHELL_CMD(info, NULL, "Show sensor device name and ready state", cmd_sensor_info),
	SHELL_CMD(set, NULL, "Set LED: sensor set <0|1|toggle>", cmd_sensor_set),
	SHELL_SUBCMD_SET_END);

/* --------------------------------------------------------------------------
 * Root command registration
 * -------------------------------------------------------------------------- */
SHELL_CMD_REGISTER(sensor, &sub_sensor, "Custom LED-sensor shell commands", NULL);
