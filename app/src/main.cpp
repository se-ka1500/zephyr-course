#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/* ==========================================================================
 * Build-time configuration — generated from Kconfig
 * ==========================================================================
 * These macros are produced by the Kconfig build step and allow the
 * application to adapt its behaviour without editing source code.
 * -------------------------------------------------------------------------- */
#define SLEEP_TIME_MS CONFIG_APP_HEARTBEAT_PERIOD_MS

/* The devicetree node identifier for the "led0" alias. */
#define LED_NODE DT_ALIAS(led0)

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

    /* ------------------------------------------------------------------
     * Mode-dependent run loop
     *
     * The active mode is selected at build time via Kconfig:
     *   CONFIG_LED_MODE_BLINK  -> periodic heartbeat
     *   CONFIG_LED_MODE_SOLID  -> keep ON
     *   CONFIG_LED_MODE_OFF    -> keep OFF
     * ------------------------------------------------------------------ */
#if defined(CONFIG_LED_MODE_BLINK)
    LOG_INF("LED mode: BLINK (period=%d ms)", SLEEP_TIME_MS);

    while (1) {
        if (gpio_pin_toggle_dt(&led) < 0) {
            LOG_ERR("GPIO toggle failed");
            return 0;
        }

        led_state = !led_state;

#if defined(CONFIG_LED_LOG_VERBOSE)
        LOG_INF("LED state: %s", led_state ? "ON" : "OFF");
#else
        /* Normal logging still prints the mode message once above. */
        LOG_DBG("LED state: %s", led_state ? "ON" : "OFF");
#endif

        k_msleep(SLEEP_TIME_MS);
    }

#elif defined(CONFIG_LED_MODE_SOLID)
    LOG_INF("LED mode: SOLID ON");

    /* Ensure LED is driven active-high and stay there. */
    (void)gpio_pin_set_dt(&led, 1);
    while (1) {
#if defined(CONFIG_LED_LOG_VERBOSE)
        LOG_INF("LED state: ON (solid)");
#endif
        k_msleep(SLEEP_TIME_MS);
    }

#elif defined(CONFIG_LED_MODE_OFF)
    LOG_INF("LED mode: OFF");

    /* Ensure LED is driven inactive and stay there. */
    (void)gpio_pin_set_dt(&led, 0);
    while (1) {
#if defined(CONFIG_LED_LOG_VERBOSE)
        LOG_INF("LED state: OFF");
#endif
        k_msleep(SLEEP_TIME_MS);
    }

#else
    #error "No LED mode selected — check Kconfig (LED_MODE choice)."
#endif

    return 0;
}
