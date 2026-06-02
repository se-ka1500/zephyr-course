# Lecture 4 Evidence — Devicetree Overlay + Kconfig Integration

## 1. Devicetree Overlay (`app/app.overlay`)

```dts
#include <zephyr/dt-bindings/input/input-event-codes.h>

/ {
    aliases {
        led0 = &my_led;
        led1 = &my_led2;
        button0 = &my_button;
    };

    leds {
        compatible = "gpio-leds";
        my_led: led_0 {
            gpios = <&gpio0 2 GPIO_ACTIVE_HIGH>;
            label = "User LED";
        };
        my_led2: led_1 {
            gpios = <&gpio0 3 GPIO_ACTIVE_HIGH>;
            label = "User LED 2";
        };
    };

    buttons {
        compatible = "gpio-keys";
        my_button: button_0 {
            gpios = <&gpio0 4 GPIO_ACTIVE_LOW>;
            label = "User Button";
            zephyr,code = <INPUT_KEY_0>;
        };
    };
};
```

## 2. Merged Devicetree Snippets (`build/zephyr/zephyr.dts`)

### Aliases
```dts
aliases {
    led0 = &my_led;        /* in app/app.overlay:5 */
    led1 = &my_led2;       /* in app/app.overlay:6 */
    button0 = &my_button;  /* in app/app.overlay:7 */
};
```

### LED Nodes
```dts
leds {
    compatible = "gpio-leds"; /* in app/app.overlay:10 */

    my_led: led_0 {
        gpios = < &gpio0 0x2 0x0 >; /* in app/app.overlay:13 */
        label = "User LED";         /* in app/app.overlay:14 */
    };

    my_led2: led_1 {
        gpios = < &gpio0 0x3 0x0 >; /* in app/app.overlay:17 */
        label = "User LED 2";       /* in app/app.overlay:18 */
    };
};
```

### Button Node
```dts
buttons {
    compatible = "gpio-keys"; /* in app/app.overlay:23 */

    button0: my_button: button_0 {
        gpios = < &gpio0 0x4 0x1 >; /* in app/app.overlay:25 */
        label = "User Button";      /* in app/app.overlay:26 */
        zephyr,code = < 0x100 >;    /* INPUT_KEY_0 */
    };
};
```

## 3. DT Macro Usage in `app/src/main.cpp`

```cpp
/* Devicetree node identifiers extracted via aliases */
#define LED0_NODE    DT_ALIAS(led0)
#define LED1_NODE    DT_ALIAS(led1)
#define BUTTON0_NODE DT_ALIAS(button0)

/* Compile-time safety */
#if !DT_NODE_HAS_STATUS(LED0_NODE, okay)
#error "Alias led0 is missing or not enabled"
#endif
...

/* GPIO specifications extracted from devicetree */
static const struct gpio_dt_spec led0    = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1    = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET(BUTTON0_NODE, gpios);

/* Run-time evidence prints */
LOG_INF("DT alias led0 -> node=%s, label=%s, pin=%u",
        DT_NODE_FULL_NAME(LED0_NODE), DT_PROP(LED0_NODE, label), led0.pin);
LOG_INF("DT alias led1 -> node=%s, label=%s, pin=%u",
        DT_NODE_FULL_NAME(LED1_NODE), DT_PROP(LED1_NODE, label), led1.pin);
LOG_INF("DT alias button0 -> node=%s, label=%s, pin=%u",
        DT_NODE_FULL_NAME(BUTTON0_NODE), DT_PROP(BUTTON0_NODE, label), button0.pin);
```

## 4. Kconfig / DT Integration (`app/Kconfig`)

```kconfig
config BUTTON_SUBSYSTEM
	bool "Enable Button Subsystem"
	default y
	depends on $(dt_alias_enabled,button0)
	help
	  Enable the button reading subsystem.  This symbol is automatically
	  visible / selectable only when the devicetree alias 'button0' is
	  present and its target node is enabled.
```

The C++ source then gates button initialisation and reading with `#if defined(CONFIG_BUTTON_SUBSYSTEM)`.

## 5. Build Commands

```bash
cd /Users/semihkaya/Desktop/workspace/iomico/zephyr-course-hw
rm -rf build
west build -b esp32s3_devkitc/esp32s3/procpu app/
west flash --esp-device /dev/cu.usbserial-A5069RR4
```

Build completed successfully (227/227 tasks, zero errors).

## 6. Serial Log

Captured via Python script reading `/dev/cu.usbserial-A5069RR4` at 115200 baud.

Boot + initialisation:
```
inf> main: DT alias button0 -> node=button_0, label=User Button, pin=4
[00:00:00.184,000] <inf> main: LED mode: BLINK (period=1000 ms)
```

Periodic heartbeat (LED0 and LED1 toggle in opposite phase; button not pressed):
```
[00:00:00.184,000] <inf> main: LED0=OFF LED1=ON BTN0=RELEASED
[00:00:01.184,000] <inf> main: LED0=ON LED1=OFF BTN0=RELEASED
[00:00:02.184,000] <inf> main: LED0=OFF LED1=ON BTN0=RELEASED
[00:00:03.184,000] <inf> main: LED0=ON LED1=OFF BTN0=RELEASED
[00:00:04.184,000] <inf> main: LED0=OFF LED1=ON BTN0=RELEASED
[00:00:05.185,000] <inf> main: LED0=ON LED1=OFF BTN0=RELEASED
```

The log demonstrates:
* Devicetree aliases are resolved correctly (`pin=4` matches overlay `gpio0 4`).
* Both LEDs toggle in opposite phase as programmed.
* Button state is read and reported (`RELEASED` because ACTIVE_LOW pin is pulled high).
