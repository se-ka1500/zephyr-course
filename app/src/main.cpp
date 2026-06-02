#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/* ==========================================================================
 * Build-time configuration — generated from Kconfig
 * ========================================================================== */
#define SLEEP_TIME_MS CONFIG_APP_HEARTBEAT_PERIOD_MS

/* The devicetree node identifier for the "app-led" alias. */
#define LED_NODE DT_ALIAS(app_led)

/* Compile-time safety: ensure the alias exists in the merged devicetree */
#if !DT_NODE_HAS_STATUS(LED_NODE, okay)
#error "Alias app-led is missing or not enabled"
#endif

/* GPIO specification extracted from devicetree */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* ========================================================================== */
int main(void)
{
    bool led_state = true;

    /* ------------------------------------------------------------------
     * GPIO initialisation
     * ------------------------------------------------------------------ */
    if (!gpio_is_ready_dt(&led)) {
        LOG_ERR("LED GPIO device not ready");
        return 0;
    }

    if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE) < 0) {
        LOG_ERR("Failed to configure LED GPIO");
        return 0;
    }

    /* Print extracted devicetree properties as evidence */
    LOG_INF("DT alias app-led -> node=%s, label=%s, pin=%u",
            DT_NODE_FULL_NAME(LED_NODE), DT_PROP(LED_NODE, label), led.pin);

    /* ------------------------------------------------------------------
     * Heartbeat loop
     * ------------------------------------------------------------------ */
    LOG_INF("Heartbeat LED (period=%d ms)", SLEEP_TIME_MS);

    while (1) {
        if (gpio_pin_toggle_dt(&led) < 0) {
            LOG_ERR("GPIO toggle failed");
            return 0;
        }

        led_state = !led_state;
        LOG_INF("LED state: %s", led_state ? "ON" : "OFF");

        k_msleep(SLEEP_TIME_MS);
    }
}
