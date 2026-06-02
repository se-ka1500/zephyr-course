#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/* ==========================================================================
 * Build-time configuration — generated from Kconfig
 * ========================================================================== */
#define SLEEP_TIME_MS CONFIG_APP_HEARTBEAT_PERIOD_MS

/* Devicetree node identifiers extracted via aliases */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define BUTTON0_NODE DT_ALIAS(button0)

/* Compile-time safety: ensure the aliases exist in the merged devicetree */
#if !DT_NODE_HAS_STATUS(LED0_NODE, okay)
#error "Alias led0 is missing or not enabled"
#endif
#if !DT_NODE_HAS_STATUS(LED1_NODE, okay)
#error "Alias led1 is missing or not enabled"
#endif
#if defined(CONFIG_BUTTON_SUBSYSTEM)
#if !DT_NODE_HAS_STATUS(BUTTON0_NODE, okay)
#error "Alias button0 is missing or not enabled"
#endif
#endif

/* GPIO specifications extracted from devicetree */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
#if defined(CONFIG_BUTTON_SUBSYSTEM)
static const struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET(BUTTON0_NODE, gpios);
#endif

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* ========================================================================== */
int main(void)
{
    bool led_state = true;

    /* ------------------------------------------------------------------
     * LED0 initialisation
     * ------------------------------------------------------------------ */
    if (!gpio_is_ready_dt(&led0)) {
        LOG_ERR("LED0 GPIO device not ready (%s)", led0.port->name);
        return 0;
    }
    if (gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE) < 0) {
        LOG_ERR("Failed to configure LED0 GPIO");
        return 0;
    }

    /* ------------------------------------------------------------------
     * LED1 initialisation
     * ------------------------------------------------------------------ */
    if (!gpio_is_ready_dt(&led1)) {
        LOG_ERR("LED1 GPIO device not ready (%s)", led1.port->name);
        return 0;
    }
    if (gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE) < 0) {
        LOG_ERR("Failed to configure LED1 GPIO");
        return 0;
    }

    /* ------------------------------------------------------------------
     * Button0 initialisation (conditional on Kconfig symbol generated
     * from DT alias presence via depends on $(dt_alias_enabled,button0))
     * ------------------------------------------------------------------ */
#if defined(CONFIG_BUTTON_SUBSYSTEM)
    if (!gpio_is_ready_dt(&button0)) {
        LOG_ERR("Button0 GPIO device not ready (%s)", button0.port->name);
        return 0;
    }
    if (gpio_pin_configure_dt(&button0, GPIO_INPUT) < 0) {
        LOG_ERR("Failed to configure Button0 GPIO");
        return 0;
    }
#endif

    /* Print extracted devicetree properties as evidence */
    LOG_INF("DT alias led0 -> node=%s, label=%s, pin=%u",
            DT_NODE_FULL_NAME(LED0_NODE), DT_PROP(LED0_NODE, label), led0.pin);
    LOG_INF("DT alias led1 -> node=%s, label=%s, pin=%u",
            DT_NODE_FULL_NAME(LED1_NODE), DT_PROP(LED1_NODE, label), led1.pin);
#if defined(CONFIG_BUTTON_SUBSYSTEM)
    LOG_INF("DT alias button0 -> node=%s, label=%s, pin=%u",
            DT_NODE_FULL_NAME(BUTTON0_NODE), DT_PROP(BUTTON0_NODE, label), button0.pin);
#endif

    /* ------------------------------------------------------------------
     * Mode-dependent run loop
     * ------------------------------------------------------------------ */
#if defined(CONFIG_LED_MODE_BLINK)
    LOG_INF("LED mode: BLINK (period=%d ms)", SLEEP_TIME_MS);

    while (1) {
        if (gpio_pin_toggle_dt(&led0) < 0) {
            LOG_ERR("LED0 toggle failed");
            return 0;
        }
        if (gpio_pin_toggle_dt(&led1) < 0) {
            LOG_ERR("LED1 toggle failed");
            return 0;
        }

        led_state = !led_state;

#if defined(CONFIG_BUTTON_SUBSYSTEM)
        int btn_val = gpio_pin_get_dt(&button0);
        if (btn_val < 0) {
            LOG_ERR("Button0 read failed");
        } else if (btn_val == 0) {
            /* ACTIVE_LOW button -> 0 means pressed */
            LOG_INF("Button0 PRESSED");
        }
#endif

#if defined(CONFIG_LED_LOG_VERBOSE)
#if defined(CONFIG_BUTTON_SUBSYSTEM)
        LOG_INF("LED0=%s LED1=%s BTN0=%s",
                led_state ? "ON" : "OFF",
                led_state ? "OFF" : "ON",
                btn_val == 0 ? "PRESSED" : "RELEASED");
#else
        LOG_INF("LED0=%s LED1=%s",
                led_state ? "ON" : "OFF",
                led_state ? "OFF" : "ON");
#endif
#else
#if defined(CONFIG_BUTTON_SUBSYSTEM)
        LOG_DBG("LED0=%s LED1=%s BTN0=%s",
                led_state ? "ON" : "OFF",
                led_state ? "OFF" : "ON",
                btn_val == 0 ? "PRESSED" : "RELEASED");
#else
        LOG_DBG("LED0=%s LED1=%s",
                led_state ? "ON" : "OFF",
                led_state ? "OFF" : "ON");
#endif
#endif

        k_msleep(SLEEP_TIME_MS);
    }

#elif defined(CONFIG_LED_MODE_SOLID)
    LOG_INF("LED mode: SOLID ON");

    (void)gpio_pin_set_dt(&led0, 1);
    (void)gpio_pin_set_dt(&led1, 1);
    while (1) {
#if defined(CONFIG_LED_LOG_VERBOSE)
        LOG_INF("LED state: both ON (solid)");
#endif
        k_msleep(SLEEP_TIME_MS);
    }

#elif defined(CONFIG_LED_MODE_OFF)
    LOG_INF("LED mode: OFF");

    (void)gpio_pin_set_dt(&led0, 0);
    (void)gpio_pin_set_dt(&led1, 0);
    while (1) {
#if defined(CONFIG_LED_LOG_VERBOSE)
        LOG_INF("LED state: both OFF");
#endif
        k_msleep(SLEEP_TIME_MS);
    }

#else
    #error "No LED mode selected — check Kconfig (LED_MODE choice)."
#endif

    return 0;
}
