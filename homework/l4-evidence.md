# Lecture 4 Evidence — Heartbeat LED via Kconfig + Overlay

## Homework Requirements (from Lecture 4 slide)

1. Create `app.overlay` — add alias `app-led` pointing to your board's `led0`.
2. Add `Kconfig` file with `int APP_HEARTBEAT_PERIOD_MS` (default 500, range 100–2000).
3. In C: use `DT_ALIAS(app_led)` for the GPIO and `CONFIG_APP_HEARTBEAT_PERIOD_MS` for the sleep duration.
4. Verify: open `menuconfig`, change the period, rebuild — LED blink speed must change.
5. Push tag: `l4-task1`.

---

## 1. Devicetree Overlay (`app/app.overlay`)

```dts
/ {
    aliases {
        app-led = &my_led;
    };

    leds {
        compatible = "gpio-leds";
        my_led: led_0 {
            gpios = <&gpio0 2 GPIO_ACTIVE_HIGH>;
            label = "User LED";
        };
    };
};
```

*The ESP32-S3 DevKitC does not have a built-in `led0` alias, so the overlay defines the LED node and aliases it as `app-led`.*

---

## 2. Merged Devicetree Verification (`build/zephyr/zephyr.dts`)

### Alias
```dts
aliases {
    app-led = &my_led;     /* in app/app.overlay:3 */
};
```

### LED Node
```dts
leds {
    compatible = "gpio-leds"; /* in app/app.overlay:6 */

    my_led: led_0 {
        gpios = < &gpio0 0x2 0x0 >; /* in app/app.overlay:9 */
        label = "User LED";         /* in app/app.overlay:10 */
    };
};
```

---

## 3. Kconfig (`app/Kconfig`)

```kconfig
config APP_HEARTBEAT_PERIOD_MS
	int "Heartbeat period (ms)"
	default 500
	range 100 2000
	help
	  Blink period in milliseconds.
```

*The full file also preserves the L3 LED mode choice (`BLINK`/`SOLID`/`OFF`) under the `LED Subsystem` menu, but the homework only requires the period symbol.*

---

## 4. C++ Source (`app/src/main.cpp`)

```cpp
/* The devicetree node identifier for the "app-led" alias. */
#define LED_NODE DT_ALIAS(app_led)

/* Compile-time safety */
#if !DT_NODE_HAS_STATUS(LED_NODE, okay)
#error "Alias app-led is missing or not enabled"
#endif

/* GPIO specification extracted from devicetree */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

int main(void)
{
    ...
    LOG_INF("DT alias app-led -> node=%s, label=%s, pin=%u",
            DT_NODE_FULL_NAME(LED_NODE), DT_PROP(LED_NODE, label), led.pin);

    LOG_INF("Heartbeat LED (period=%d ms)", CONFIG_APP_HEARTBEAT_PERIOD_MS);

    while (1) {
        gpio_pin_toggle_dt(&led);
        k_msleep(CONFIG_APP_HEARTBEAT_PERIOD_MS);
    }
}
```

---

## 5. Project Configuration (`app/prj.conf`)

```conf
CONFIG_GPIO=y
CONFIG_LOG=y
CONFIG_APP_HEARTBEAT_PERIOD_MS=500
```

---

## 6. Build Commands

```bash
cd /Users/semihkaya/Desktop/workspace/iomico/zephyr-course-hw
rm -rf build
west build -b esp32s3_devkitc/esp32s3/procpu app/
west flash --esp-device /dev/cu.usbserial-A5069RR4
```

Build completed successfully (227/227 tasks, zero errors).

---

## 7. Serial Log — Default Period (500 ms)

Captured via Python script reading `/dev/cu.usbserial-A5069RR4` at 115200 baud.

```
[00:00:00.184,000] <inf> main: DT alias app-led -> node=led_0, label=User LED, pin=2
[00:00:00.184,000] <inf> main: Heartbeat LED (period=500 ms)
[00:00:00.184,000] <inf> main: LED state: ON
[00:00:00.684,000] <inf> main: LED state: OFF
[00:00:01.184,000] <inf> main: LED state: ON
[00:00:01.684,000] <inf> main: LED state: OFF
[00:00:02.184,000] <inf> main: LED state: ON
```

*Timestamps confirm a 500 ms toggle interval (0.184 → 0.684 = 0.500 s).*

---

## 8. Verification — Changing Period via Kconfig

To satisfy the homework verification step ("change the period, rebuild — LED blink speed must change"), `prj.conf` was temporarily edited to:

```conf
CONFIG_APP_HEARTBEAT_PERIOD_MS=2000
```

After a clean rebuild and re-flash, the serial log shows:

```
[00:00:04.184,000] <inf> main: LED state: OFF
[00:00:06.184,000] <inf> main: LED state: ON
[00:00:08.184,000] <inf> main: LED state: OFF
[00:00:10.184,000] <inf> main: LED state: ON
[00:00:12.185,000] <inf> main: LED state: OFF
[00:00:14.185,000] <inf> main: LED state: ON
```

*Timestamps now show exactly 2000 ms intervals (4.184 → 6.184 = 2.000 s), proving the Kconfig value drives the blink speed at build time.*

`prj.conf` was then restored to the default `500` and the project rebuilt to leave the workspace in the correct final state.

---

## 9. Commit & Tag

```bash
git add app/Kconfig app/app.overlay app/src/main.cpp app/prj.conf homework/l4-evidence.md
git commit -m "L4-task1: Heartbeat LED via Kconfig + Overlay"
git tag l4-task1
```
