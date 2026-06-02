# Handoff Summary — Zephyr Course Homework (L4 Complete → L5 Ready)

## 1. Current Status

| Item | Status | Details |
|------|--------|---------|
| GitHub fork | ✅ | `se-ka1500/zephyr-course` |
| Local repo | ✅ | `/Users/semihkaya/Desktop/workspace/iomico/zephyr-course-hw` |
| West workspace | ✅ | `.west/config` points to workspace above |
| Zephyr + deps | ✅ | v4.2.0 pulled; `hal_espressif` present |
| L2 homework | ✅ | Tag `l2-task1`; `homework/l2-evidence.md` |
| L3 homework | ✅ | Tag `l3-task1`; `homework/l3-evidence.md` |
| L4 homework | ✅ | Tag `l4-task1`; `homework/l4-evidence.md` |
| Build verification | ✅ | `app/` builds and flashes on `esp32s3_devkitc/esp32s3/procpu` |
| Serial evidence | ✅ | 500 ms heartbeat verified; period-change verification (500→2000→500 ms) done |

### Latest commit
```
181e71e L4-task1: Align with homework spec — app-led alias, 500ms default, verify period change
```

### Tags pushed to origin
- `l2-task1`
- `l3-task1`
- `l4-task1`

---

## 2. Environment Inventory (Verified)

| Tool | Version | Path / Notes |
|------|---------|--------------|
| west | 1.5.0 | `/opt/homebrew/bin/west` |
| Zephyr SDK | 0.17.4 | `/Users/semihkaya/zephyr-sdk-0.17.4/` |
| Python | 3.14.4 | `/opt/homebrew/opt/python@3.14/bin/python3.14` |
| CMake | 4.2.1 | — |
| Ninja | 1.13.2 | — |
| QEMU (x86) | installed | `/opt/homebrew/bin/qemu-system-x86_64` |
| esptool.py | 4.8.1 | via HAL |
| Serial port | `/dev/cu.usbserial-A5069RR4` | ESP32-S3-DevKitC |
| Board target | `esp32s3_devkitc/esp32s3/procpu` | — |

---

## 3. L4 Workspace State

### Files modified for L4
| File | Purpose | Key contents |
|------|---------|-------------|
| `app/app.overlay` | DT overlay | `app-led` alias → `gpio0 2` LED node |
| `app/Kconfig` | App Kconfig | `APP_HEARTBEAT_PERIOD_MS` (default 500, range 100–2000); LED mode choice preserved from L3 |
| `app/prj.conf` | Config overrides | `CONFIG_APP_HEARTBEAT_PERIOD_MS=500` |
| `app/src/main.cpp` | Application | Uses `DT_ALIAS(app_led)` + `CONFIG_APP_HEARTBEAT_PERIOD_MS`; single LED heartbeat |
| `homework/l4-evidence.md` | Evidence | Overlay source, DTS snippets, DT macro usage, build commands, serial logs with period verification |

### What was removed from the earlier over-engineered attempt
- Second LED (`led1`) and `my_led2` node
- `button0` alias and `gpio-keys` node
- `BUTTON_SUBSYSTEM` Kconfig/DT integration
- `LED_LOG_VERBOSE` and `LED_MODE_*` branching in `main.cpp` (simplified to pure heartbeat)

---

## 4. Lecture 5 — Reconstructed Recipe & Roadmap

> **⚠️ Reconstructed requirements:** The L5 PDF could not be text-extracted. The roadmap below is based on standard Zephyr course progression (L5 is commonly *Interrupts & Timers* or *Sensor Drivers / Device Model*). **Review the L5 PDF when available and adjust accordingly.**

### 4.1 Most Likely L5 Topics

Standard Zephyr curricula place one of the following at Lecture 5:

| Topic | Typical Homework |
|-------|-----------------|
| **A. Interrupts & Timers** | Add a GPIO interrupt callback (button press triggers LED toggle or prints a message). Or use `k_timer` / `k_work` for periodic background tasks. |
| **B. Sensor Subsystem** | Add a sensor node to the overlay (e.g., `bosch,bme280` or `ti,hdc1080`), use `sensor_sample_fetch` / `sensor_channel_get`, and log temperature/humidity. |
| **C. Shell / Console Subsystem** | Enable `CONFIG_SHELL`, register custom shell commands, and control LED period or mode at run-time. |
| **D. Flash / Settings / NVS** | Use `settings` or `nvs` subsystem to persist the heartbeat period across reboots. |

**Recommended starting assumption:** Topic **A (Interrupts & Timers)** because it naturally extends the existing GPIO/heartbeat app without requiring extra hardware.

---

### 4.2 Tools & Materials Available

| File | Purpose |
|------|---------|
| `app/CMakeLists.txt` | Application build entry point |
| `app/prj.conf` | Kconfig overrides (GPIO=y, LOG=y, period=500) |
| `app/src/main.cpp` | LED heartbeat app; uses `DT_ALIAS(app_led)` |
| `app/app.overlay` | DT overlay: `app-led` alias → `gpio0 2` |
| `app/Kconfig` | LED Subsystem menu + period symbol |
| `build/zephyr/zephyr.dts` | Merged devicetree output (regenerated on build) |

---

### 4.3 Reconstructed Roadmap (Topic A: Interrupts & Timers — recommended)

#### Step 1 — Extend Overlay with a Button

Add a `gpio-keys` node to `app/app.overlay` (reuse the removed button code from the first L4 draft if needed). Example:

```dts
/ {
    aliases {
        app-led = &my_led;
        app-btn = &my_button;
    };

    leds {
        compatible = "gpio-leds";
        my_led: led_0 {
            gpios = <&gpio0 2 GPIO_ACTIVE_HIGH>;
            label = "User LED";
        };
    };

    buttons {
        compatible = "gpio-keys";
        my_button: button_0 {
            gpios = <&gpio0 4 GPIO_ACTIVE_LOW>;
            label = "User Button";
        };
    };
};
```

> Use `gpio0 4` or another free pin. The ESP32-S3 DevKitC does not have a standard on-board user button (other than the BOOT button on `gpio0 0`), so `gpio0 4` is typically safe. Verify with `build/zephyr/zephyr.dts` after the first build.

#### Step 2 — Add GPIO Interrupt Logic in `main.cpp`

Skeleton (interrupt-driven button):

```cpp
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#define LED_NODE    DT_ALIAS(app_led)
#define BUTTON_NODE DT_ALIAS(app_btn)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

static struct gpio_callback button_cb_data;

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    gpio_pin_toggle_dt(&led);
    LOG_INF("Button interrupt — LED toggled");
}

int main(void)
{
    if (!gpio_is_ready_dt(&led) || !gpio_is_ready_dt(&button)) { return 0; }
    gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&button, GPIO_INPUT);

    /* Configure interrupt on falling edge (active-low button) */
    gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);

    LOG_INF("Interrupt-driven LED + button ready");
    while (1) { k_msleep(1000); }
}
```

> If the lecture asks for **timers** instead of interrupts, replace the interrupt block with `k_timer_init` + a timer expiry handler.

#### Step 3 — Update `app/prj.conf`

```conf
CONFIG_GPIO=y
CONFIG_LOG=y
# If using input-event-codes.h for zephyr,code in overlay:
# CONFIG_INPUT=y
```

#### Step 4 — Build, Flash, Verify

```bash
cd /Users/semihkaya/Desktop/workspace/iomico/zephyr-course-hw
rm -rf build
west build -b esp32s3_devkitc/esp32s3/procpu app/
west flash --esp-device /dev/cu.usbserial-A5069RR4
```

Verify:
- `build/zephyr/zephyr.dts` contains the `app-btn` alias and `buttons` node.
- Serial log shows boot message and button-press-triggered toggles.
- (If timer variant) Serial log shows periodic timer expiry messages.

#### Step 5 — Write Evidence

Create `homework/l5-evidence.md` containing:
- Overlay source (if changed for L5).
- `main.cpp` snippets showing interrupt callback / timer setup.
- Build and flash commands.
- Serial log proving interrupts fire and the callback runs.

#### Step 6 — Commit & Tag

```bash
git add -A
git commit -m "L5-task1: Add GPIO interrupt (or timer) to heartbeat app"
git push origin main
git tag l5-task1
git push origin l5-task1
```

---

### 4.4 Alternative Roadmap (Topic B: Sensor Subsystem)

If L5 is about sensors:

1. Add a sensor node to `app/app.overlay` (I2C or SPI, depending on what the lecture provides).
2. Enable `CONFIG_SENSOR=y` in `prj.conf`.
3. In `main.cpp`, use `device_is_ready(sensor)`, `sensor_sample_fetch()`, `sensor_channel_get()`.
4. Log sensor readings every N seconds.
5. Evidence = overlay, DTS merge, `sensor` API usage, serial log with readings.

---

## 5. Known Open Questions for L5

| # | Question | Impact |
|---|----------|--------|
| 1 | **Exact L5 topic?** Interrupts/timers, sensors, shell, or flash settings? | The roadmap above covers the most likely case (interrupts); pivot when the PDF is reviewed |
| 2 | **Does the lecture provide a specific sensor or shield?** | If yes, the overlay must match the exact `compatible` and bus node |
| 3 | **GPIO pin conflicts?** | `gpio0 4` is usually free; switch if the build warns about pin reservations |
| 4 | **Emulator build required?** | Probably not for GPIO interrupt homework (board-specific), but add `qemu_x86` evidence if asked |
| 5 | **Should L3/L4 features (Kconfig period, DT alias) be preserved in L5?** | Recommended: keep them intact and build the new feature on top so prior homework remains verifiable |

---

## 6. Quick-Start Commands for Fresh L5 Session

```bash
# Verify workspace
cd /Users/semihkaya/Desktop/workspace/iomico/zephyr-course-hw
west status

# Clean build for L5
rm -rf build
west build -b esp32s3_devkitc/esp32s3/procpu app/

# Inspect merged devicetree (after adding any new nodes)
grep -A2 "aliases" build/zephyr/zephyr.dts
grep -A5 "buttons" build/zephyr/zephyr.dts

# Flash
west flash --esp-device /dev/cu.usbserial-A5069RR4

# Read serial (non-resetting)
python3 -c "import serial,time;s=serial.Serial('/dev/cu.usbserial-A5069RR4',115200,timeout=10,dsrdtr=False,rtscts=False);s.dtr=False;s.rts=False;time.sleep(0.2);[print(l.decode().rstrip()) for l in [s.readline() for _ in range(25)] if l];s.close()"
```

**Next session goal:** Review the L5 PDF, select the correct topic, implement the homework, create `homework/l5-evidence.md`, and push tag `l5-task1`.
