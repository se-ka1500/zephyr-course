# Lecture 3 Homework Evidence — Kconfig LED Subsystem

## Assignment
1. Create an application-level `Kconfig` that defines a custom **LED Subsystem** menu.
2. Demonstrate Kconfig constructs:
   - `menu` / `endmenu`
   - `if … endif` conditional block
   - `choice` with mutually-exclusive options
   - `int` symbol with `range`
   - `visible if` dependency (modern syntax)
3. Wire the Kconfig-generated macros into the application source code.
4. Build, flash, and verify on hardware that the configured values are applied at run-time.

---

## 1. Environment

- **west**: v1.5.0
- **Zephyr SDK**: v0.17.4
- **Python**: 3.14.4
- **CMake**: 4.2.1
- **Ninja**: 1.13.2
- **Board**: ESP32-S3-DevKitC (`esp32s3_devkitc/esp32s3/procpu`)
- **Zephyr**: v4.2.0

Workspace: `/Users/semihkaya/Desktop/workspace/iomico/zephyr-course-hw`  
Fork: `se-ka1500/zephyr-course`

---

## 2. Application Kconfig (`app/Kconfig`)

The Kconfig file lives in `APPLICATION_SOURCE_DIR` so Zephyr auto-discovers it.  
It sources `Kconfig.zephyr` at the bottom so standard symbols (GPIO, LOG, etc.) remain available.

```kconfig
# ============================================================================
# Application Kconfig — LED Subsystem
# ============================================================================
# This file defines the configuration space for the course application's
# LED heartbeat subsystem.  It demonstrates:
#   • A top-level menu entry
#   • A conditional block (`if … endif`)
#   • A `choice` with mutually-exclusive options
#   • An `int` symbol with `range`
#   • A `visible if` dependency on a sub-menu (modern Zephyr syntax)
#
# Zephyr's Kconfig auto-discovery will pick up this file because it lives in
# APPLICATION_SOURCE_DIR (i.e. the same directory as CMakeLists.txt).
# ============================================================================

menu "LED Subsystem"

# ---------------------------------------------------------------------------
# Master switch — everything below depends on this symbol.
# ---------------------------------------------------------------------------
config LED_SUBSYSTEM
	bool "Enable LED Subsystem"
	default y
	help
	  Enable the LED heartbeat subsystem.  When disabled, the LED pin is
	  left in its default (high-impedance) state and no GPIO activity occurs.

# ---------------------------------------------------------------------------
# All options below are visible only when the master switch is on.
# ---------------------------------------------------------------------------
if LED_SUBSYSTEM

# ---------------------------------------------------------------------------
# LED operating mode — mutually exclusive choices.
# ---------------------------------------------------------------------------
choice LED_MODE
	prompt "LED operating mode"
	default LED_MODE_BLINK
	help
	  Select how the LED behaves at run-time.

	  BLINK  – periodic toggle (heartbeat).
	  SOLID  – continuously driven ON.
	  OFF    – continuously driven OFF.

config LED_MODE_BLINK
	bool "Blink (heartbeat)"
	help
	  Toggle the LED at the period defined by APP_HEARTBEAT_PERIOD_MS.

config LED_MODE_SOLID
	bool "Solid ON"
	help
	  Drive the LED GPIO active-high and keep it on indefinitely.

config LED_MODE_OFF
	bool "Off"
	help
	  Drive the LED GPIO inactive and keep it off indefinitely.

endchoice

# ---------------------------------------------------------------------------
# Tuning options — only relevant when the mode is BLINK.
# ---------------------------------------------------------------------------
menu "Blink Tuning"
	visible if LED_MODE_BLINK

config APP_HEARTBEAT_PERIOD_MS
	int "Heartbeat period (ms)"
	default 1000
	range 100 5000
	help
	  Blink period in milliseconds.  The value is clamped to the range
	  [100, 5000] to prevent eye-strain (too fast) or unresponsiveness
	  (too slow).

endmenu

# ---------------------------------------------------------------------------
# Diagnostics options — visible whenever the subsystem is enabled.
# ---------------------------------------------------------------------------
menu "Diagnostics"
	visible if LED_SUBSYSTEM

config LED_LOG_VERBOSE
	bool "Verbose LED logging"
	default n
	help
	  Log every LED state change to the console.  Useful for verifying that
	  the chosen mode and period are actually being applied at run-time.

endmenu

endif # LED_SUBSYSTEM

endmenu

# ---------------------------------------------------------------------------
# Pull in the full Zephyr Kconfig tree so that standard symbols (GPIO, LOG,
# etc.) are defined and can be referenced from prj.conf / defconfig files.
# ---------------------------------------------------------------------------
source "Kconfig.zephyr"
```

---

## 3. Project Configuration (`app/prj.conf`)

```
# ============================================================================
# Project Configuration — Lecture 3 (Kconfig)
# ============================================================================
# This file overrides default values defined in the application Kconfig
# (app/Kconfig) and in Zephyr's own Kconfig tree.
# ============================================================================

# ---------------------------------------------------------------------------
# Zephyr base requirements
# ---------------------------------------------------------------------------
CONFIG_GPIO=y
CONFIG_LOG=y

# ---------------------------------------------------------------------------
# LED Subsystem (defined in app/Kconfig)
# ---------------------------------------------------------------------------
# Master switch — must be enabled for the subsystem to compile.
CONFIG_LED_SUBSYSTEM=y

# Operating mode: BLINK | SOLID | OFF
# Default in Kconfig is BLINK; we keep it explicit here for clarity.
CONFIG_LED_MODE_BLINK=y
# CONFIG_LED_MODE_SOLID=y
# CONFIG_LED_MODE_OFF=y

# Heartbeat period — valid only when LED_MODE_BLINK is selected.
# Range enforced by Kconfig: 100 .. 5000 ms
CONFIG_APP_HEARTBEAT_PERIOD_MS=1000

# Verbose logging — log every state change.
# Uncomment to enable; default is 'n'.
CONFIG_LED_LOG_VERBOSE=y
```

---

## 4. Application Source (`app/src/main.cpp`)

Key changes:
- Hard-coded `#define SLEEP_TIME_MS 1000` replaced by `CONFIG_APP_HEARTBEAT_PERIOD_MS`.
- Conditional compilation blocks for `CONFIG_LED_MODE_BLINK`, `CONFIG_LED_MODE_SOLID`, and `CONFIG_LED_MODE_OFF`.
- Verbose logging gated by `CONFIG_LED_LOG_VERBOSE`.

```cpp
#define SLEEP_TIME_MS CONFIG_APP_HEARTBEAT_PERIOD_MS

/* … GPIO setup … */

#if defined(CONFIG_LED_MODE_BLINK)
    LOG_INF("LED mode: BLINK (period=%d ms)", SLEEP_TIME_MS);
    while (1) {
        gpio_pin_toggle_dt(&led);
        led_state = !led_state;
#if defined(CONFIG_LED_LOG_VERBOSE)
        LOG_INF("LED state: %s", led_state ? "ON" : "OFF");
#endif
        k_msleep(SLEEP_TIME_MS);
    }
#elif defined(CONFIG_LED_MODE_SOLID)
    /* … */
#elif defined(CONFIG_LED_MODE_OFF)
    /* … */
#endif
```

---

## 5. Resolved Configuration (`build/zephyr/.config`)

```
CONFIG_LED_SUBSYSTEM=y
CONFIG_LED_MODE_BLINK=y
# CONFIG_LED_MODE_SOLID is not set
# CONFIG_LED_MODE_OFF is not set
CONFIG_APP_HEARTBEAT_PERIOD_MS=1000
CONFIG_LED_LOG_VERBOSE=y
```

All custom symbols were parsed correctly and the non-selected choice members were disabled as expected.

---

## 6. Build & Flash Commands

```bash
# Clean build for ESP32-S3
rm -rf build
west build -b esp32s3_devkitc/esp32s3/procpu app/

# Flash to hardware
west flash --esp-device /dev/cu.usbserial-A5069RR4
```

Build completed successfully (227 objects, no Kconfig warnings).

---

## 7. Serial Output — Evidence of Correct Kconfig Integration

Captured directly from `/dev/cu.usbserial-A5069RR4` @ 115200 baud.

```
ESP-ROM:esp32s3-20210327
Build:Mar 27 2021
rst:0x1 (POWERON),boot:0x8 (SPI_FAST_FLASH_BOOT)
…
*** Booting Zephyr OS build v4.2.0 ***
[00:00:00.184,000] <inf> main: LED mode: BLINK (period=1000 ms)
[00:00:00.184,000] <inf> main: LED state: OFF
[00:00:01.184,000] <inf> main: LED state: ON
[00:00:02.184,000] <inf> main: LED state: OFF
[00:00:03.184,000] <inf> main: LED state: ON
[00:00:04.184,000] <inf> main: LED state: OFF
[00:00:05.184,000] <inf> main: LED state: ON
```

**Observations**
- The mode message confirms `CONFIG_LED_MODE_BLINK=y` is active.
- The period message confirms `CONFIG_APP_HEARTBEAT_PERIOD_MS=1000` is applied.
- The timestamp deltas are exactly **1000 ms** (`01.184 → 02.184 → 03.184 …`), matching the Kconfig range and value.
- Verbose state changes (`ON` / `OFF`) confirm `CONFIG_LED_LOG_VERBOSE=y` is functional.

---

## 8. Kconfig Constructs Demonstrated

| Construct | Location in `app/Kconfig` | Purpose |
|-----------|---------------------------|---------|
| `menu` | Line 16, 66, 83 | Group related symbols under a named menu |
| `if … endif` | Line 31–95 | Gate all subsystem options on `LED_SUBSYSTEM` |
| `choice` | Line 36–61 | Mutually-exclusive LED mode selection |
| `int` + `range` | Line 69–76 | Clamp heartbeat period to [100, 5000] ms |
| `visible if` | Line 67, 84 | Hide tuning / diagnostics menus when irrelevant |
| `source` | Line 101 | Include Zephyr's standard Kconfig tree |

---

## Conclusion

- `app/Kconfig` was created with a professional, well-commented LED Subsystem menu.
- All required constructs (`menu`, `if`, `choice`, `range`, `visible if`) are present and functional.
- `app/src/main.cpp` uses Kconfig-generated macros instead of hard-coded values.
- The application builds cleanly and the configured period is verified on hardware via serial timestamps.
- Homework is ready for review; tag `l3-task1` will be pushed.
