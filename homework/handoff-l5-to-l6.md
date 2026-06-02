# Handoff Summary — Zephyr Course Homework (L5 Complete → L6 Ready)

## 1. What Was Accomplished This Session

### L5 Homework — Fully Completed

| Item | Status | Details |
|------|--------|---------|
| L5 PDF decoded | ✅ | 33 pages rendered to PNG; topic: Custom Board Development |
| Task 1 (`l5-task1`) | ✅ | `boards/our_board/` created via Copy/Rename; built & flashed `hello_world`; serial verified; tag pushed |
| Task 2 (`l5-task2`) | ✅ | `boards/our_board2/` created from scratch; `board.c` init hook prints **"Board Initialized"** before Hello World!; tag pushed |
| Evidence | ✅ | `homework/l5-evidence.md` created and committed |
| Tags pushed | ✅ | `l5-task1`, `l5-task2` on origin |

**Task 1 serial log:**
```
*** Booting Zephyr OS build v4.2.0 ***
Hello World! our_board/esp32s3/procpu
```

**Task 2 serial log:**
```
Board Initialized
*** Booting Zephyr OS build v4.2.0 ***
Hello World! our_board2/esp32s3/procpu
```

---

## 2. L6 Lecture Decoded

### PDF
- **File:** `Zephyr Course with iomico_Lecture 6_Driver Development.pdf`
- **Pages:** 35 (rendered to `homework/l6_pages/`)
- **No extractable text** — all info read from PNG images.

### Topic: Zephyr Driver Development

**Agenda (page 4):**
1. Driver fundamentals in Zephyr
2. Module vs driver
3. Minimal module layout and required files
4. DTS binding and compatible linkage
5. Kconfig and CMake integration
6. Driver implementation pattern (Sensor API)
7. Device instantiation and multi-instance behavior
8. Homework

**Key Concepts:**
- **Module layout** (page 10):
  ```
  our-driver-module/
  ├── zephyr/module.yml          # West module registration
  ├── CMakeLists.txt             # Root build directives
  ├── Kconfig                    # Root config menu
  ├── include/our_drivers/
  │   └── our_driver.h           # Public C API (optional)
  ├── drivers/our_driver/
  │   ├── our_driver.c           # Driver implementation
  │   ├── CMakeLists.txt         # Build logic
  │   └── Kconfig                # Driver config menu
  └── dts/bindings/driver/
      └── our,driver.yaml        # Device Tree binding
  ```

- **module.yml** (page 13) registers the module with West:
  ```yaml
  name: our_custom_drivers
  build:
    cmake: .
    kconfig: Kconfig
    settings:
      dts_root: .
  ```

- **DTS Node** (page 15) links hardware to driver via `compatible`:
  ```dts
  sensor: temp_sensor {
      compatible = "our,driver";
      reg = <0x76>;
      calibration = <100>;
      status = "okay";
  };
  ```

- **Binding File** (page 16) defines schema and auto-generates `DT_HAS_OUR_DRIVER_ENABLED`:
  ```yaml
  description: Our custom sensor driver binding
  compatible: "our,driver"
  include: i2c-device.yaml
  properties:
    calibration:
      type: int
      required: true
  ```

- **C Side Linkage** (page 18): `#define DT_DRV_COMPAT our_driver` (underscore form)

- **Driver Kconfig** (page 21) auto-enables when DT node is present:
  ```kconfig
  menuconfig OUR_DRIVER
      bool "Our Custom Driver"
      default y
      depends on DT_HAS_OUR_DRIVER_ENABLED
      select I2C
  ```

- **Driver CMake** (page 22) compiles source when config is enabled:
  ```cmake
  if(CONFIG_OUR_DRIVER)
    zephyr_library()
    zephyr_library_sources(our_driver.c)
  endif()
  ```

- **Instantiation Macro** (page 26 / 33) creates per-DT-node device instances:
  ```c
  #define OUR_DRIVER_DEFINE(inst)                                         \
      static struct our_driver_data data_##inst;                            \
      static const struct our_driver_config cfg_##inst = {                  \
          .i2c = I2C_DT_SPEC_INST_GET(inst),                                \
      };                                                                    \
      DEVICE_DT_INST_DEFINE(inst, our_driver_init, NULL,                   \
          &data_##inst, &cfg_##inst,                                        \
          POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY, &our_driver_api)

  DT_INST_FOREACH_STATUS_OKAY(OUR_DRIVER_DEFINE)
  ```

- **Init Levels** (page 34): Sensor drivers use `POST_KERNEL`.

- **App Integration** (page 41):
  ```c
  const struct device *dev = DEVICE_DT_GET_ANY(our_driver);
  if (!device_is_ready(dev)) { return -ENODEV; }
  sensor_sample_fetch(dev);
  ```

---

## 3. Environment Inventory (Verified)

| Tool | Version | Path / Notes |
|------|---------|--------------|
| west | 1.5.0 | `/opt/homebrew/bin/west` |
| Zephyr SDK | 0.17.4 | `/Users/semihkaya/zephyr-sdk-0.17.4/` |
| Python | 3.14.4 | `/opt/homebrew/opt/python@3.14/bin/python3.14` |
| Zephyr base | 4.2.0 | `/Users/semihkaya/deps/zephyr` |
| Board target | `our_board2/esp32s3/procpu` | Custom board from L5 Task 2 |
| Serial port | `/dev/cu.usbserial-A5069RR4` | ESP32-S3-DevKitC |
| Project root | `/Users/semihkaya/Desktop/workspace/iomico/zephyr-course-hw` |
| App dir | `/Users/semihkaya/Desktop/workspace/iomico/zephyr-course-hw/app` | Existing app with `main.cpp`, overlay, prj.conf |
| LED GPIO | `gpio0 pin 2` | From `app/app.overlay` (alias `app-led`) |

---

## 4. L6 Homework Requirements

### Home Task 1 (`l6-task1`)
- Create a **sensor driver** following the lecture module structure.
- The "sensor" is a **simple on-board LED** (GPIO-controlled).
- Driver must implement:
  - `sensor_sample_fetch(dev)` → **turn LED ON**
  - `sensor_channel_get(dev, chan, val)` → **turn LED OFF**
- Commit and tag: **`l6-task1`**

### Home Task 2 (`l6-task2`)
- Add a **custom extension API function** to the driver (e.g. `our_led_sensor_toggle()`).
- Call it from `main.c` (or `main.cpp`).
- This function changes a parameter in the **dynamic data struct** (e.g. `led_on` state).
- Commit and tag: **`l6-task2`**

---

## 5. Recipe — Step-by-Step Roadmap

### Phase A: Create the Driver Module (Task 1)

Create directory: `modules/our_led_sensor/`

**File 1:** `modules/our_led_sensor/zephyr/module.yml`
```yaml
name: our_led_sensor
build:
  cmake: .
  kconfig: Kconfig
  settings:
    dts_root: .
```

**File 2:** `modules/our_led_sensor/CMakeLists.txt`
```cmake
cmake_minimum_required(VERSION 3.20.0)
add_subdirectory(drivers/sensor/our_led_sensor)
```

**File 3:** `modules/our_led_sensor/Kconfig`
```kconfig
menu "Our LED Sensor Module"
rsource "drivers/sensor/our_led_sensor/Kconfig"
endmenu
```

**File 4:** `modules/our_led_sensor/dts/bindings/sensor/custom,led-sensor.yaml`
```yaml
description: Custom LED Sensor (GPIO-based)

compatible: "custom,led-sensor"

include: sensor-device.yaml

properties:
  gpios:
    type: phandle-array
    required: true
    description: GPIO connected to the LED
```

**File 5:** `modules/our_led_sensor/drivers/sensor/our_led_sensor/CMakeLists.txt`
```cmake
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_OUR_LED_SENSOR)
  zephyr_library()
  zephyr_library_sources(our_led_sensor.c)
endif()
```

**File 6:** `modules/our_led_sensor/drivers/sensor/our_led_sensor/Kconfig`
```kconfig
# SPDX-License-Identifier: Apache-2.0

config OUR_LED_SENSOR
	bool "Our LED Sensor driver"
	default y
	depends on DT_HAS_CUSTOM_LED_SENSOR_ENABLED
	select GPIO
	help
	  Enable the custom LED sensor driver.
```

**File 7:** `modules/our_led_sensor/drivers/sensor/our_led_sensor/our_led_sensor.c`
```c
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
```

**File 8:** `modules/our_led_sensor/include/drivers/sensor/our_led_sensor.h`
```c
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
```

**File 9:** Add custom API implementation to `our_led_sensor.c` (for Task 2):
```c
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
```

---

### Phase B: Add DTS Node

**Option A — App overlay (recommended, non-invasive):**
Edit `app/app.overlay` and add:
```dts
/ {
    our_led_sensor: our_led_sensor {
        compatible = "custom,led-sensor";
        gpios = <&gpio0 2 GPIO_ACTIVE_HIGH>;
        status = "okay";
    };
};
```

**Option B — Board DTS:**
Edit `boards/our_board2/our_board2_procpu.dts` and add the same node inside `/ { };`.

---

### Phase C: Update Application

**Edit `app/prj.conf`** — add:
```
CONFIG_SENSOR=y
CONFIG_OUR_LED_SENSOR=y
```

**Create `app/src/main.c`** (or adapt `main.cpp`):
```c
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/* Task 2 custom extension API header */
#include <drivers/sensor/our_led_sensor.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define LED_SENSOR_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(custom_led_sensor)

int main(void)
{
	const struct device *dev = DEVICE_DT_GET(LED_SENSOR_NODE);

	if (!device_is_ready(dev)) {
		LOG_ERR("LED sensor device not ready");
		return 0;
	}

	LOG_INF("LED sensor ready");

	/* Task 1: fetch = ON, channel_get = OFF */
	sensor_sample_fetch(dev);
	LOG_INF("LED should be ON after sample_fetch");
	k_msleep(1000);

	sensor_channel_get(dev, SENSOR_CHAN_ALL, NULL);
	LOG_INF("LED should be OFF after channel_get");
	k_msleep(1000);

	/* Task 2: call custom extension API */
	our_led_sensor_toggle(dev);
	LOG_INF("LED toggled via custom API");

	while (1) {
		k_msleep(1000);
	}

	return 0;
}
```

> **Note:** If keeping `main.cpp`, wrap the custom header include with `extern "C"` or just include it normally if the header is C-compatible.

**Update `app/CMakeLists.txt`** if needed (should already be fine):
```cmake
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED)
project(my_zephyr_app)
target_sources(app PRIVATE src/main.c)
```

---

### Phase D: Build, Flash, Verify

```bash
cd /Users/semihkaya/Desktop/workspace/iomico/zephyr-course-hw
rm -rf app/build

west build -b our_board2/esp32s3/procpu app \
  -- -DBOARD_ROOT=/Users/semihkaya/Desktop/workspace/iomico/zephyr-course-hw \
     -DZEPHYR_EXTRA_MODULES=/Users/semihkaya/Desktop/workspace/iomico/zephyr-course-hw/modules/our_led_sensor
```

**Flash:**
```bash
west flash --esp-device /dev/cu.usbserial-A5069RR4
```

**Verify serial output:**
```bash
python3 -c "
import serial, time
s = serial.Serial('/dev/cu.usbserial-A5069RR4', 115200, timeout=10, dsrdtr=False, rtscts=False)
s.dtr = False; s.rts = False; time.sleep(1)
lines = [l.decode(errors='replace').rstrip() for l in [s.readline() for _ in range(40)] if l]
for line in lines: print(line)
s.close()
"
```

Expected log:
```
LED sensor ready
LED should be ON after sample_fetch
LED should be OFF after channel_get
LED toggled via custom API
```

You can also **visually verify** the LED on the DevKitC blinks / toggles.

---

### Phase E: Commit & Tag

**Task 1:**
```bash
cd /Users/semihkaya/Desktop/workspace/iomico/zephyr-course-hw
git add modules/our_led_sensor/ app/
git commit -m "L6-task1: Custom LED sensor driver with Sensor API"
git push origin main
git tag l6-task1
git push origin l6-task1
```

**Task 2:**
```bash
cd /Users/semihkaya/Desktop/workspace/iomico/zephyr-course-hw
git add -A
git commit -m "L6-task2: Custom extension API for LED sensor driver"
git push origin main
git tag l6-task2
git push origin l6-task2
```

---

## 6. Quick-Start Commands for Fresh L6 Session

```bash
# Verify workspace
cd /Users/semihkaya/Desktop/workspace/iomico/zephyr-course-hw
git status

# Render L6 PDF (already done — images in homework/l6_pages/)
ls homework/l6_pages/

# Phase A: Create module files (use recipe above or copy from handoff)
mkdir -p modules/our_led_sensor/{zephyr,drivers/sensor/our_led_sensor,dts/bindings/sensor,include/drivers/sensor}

# Phase B: Add DTS node to app overlay
cat >> app/app.overlay << 'EOF'
/ {
    our_led_sensor: our_led_sensor {
        compatible = "custom,led-sensor";
        gpios = <&gpio0 2 GPIO_ACTIVE_HIGH>;
        status = "okay";
    };
};
EOF

# Phase C: Update prj.conf
echo "CONFIG_SENSOR=y" >> app/prj.conf
echo "CONFIG_OUR_LED_SENSOR=y" >> app/prj.conf

# Phase D: Build and flash
rm -rf app/build
west build -b our_board2/esp32s3/procpu app \
  -- -DBOARD_ROOT=/Users/semihkaya/Desktop/workspace/iomico/zephyr-course-hw \
     -DZEPHYR_EXTRA_MODULES=/Users/semihkaya/Desktop/workspace/iomico/zephyr-course-hw/modules/our_led_sensor
west flash --esp-device /dev/cu.usbserial-A5069RR4

# Phase E: Read serial
python3 -c "
import serial, time
s = serial.Serial('/dev/cu.usbserial-A5069RR4', 115200, timeout=10, dsrdtr=False, rtscts=False)
s.dtr = False; s.rts = False; time.sleep(1)
lines = [l.decode(errors='replace').rstrip() for l in [s.readline() for _ in range(40)] if l]
for line in lines: print(line)
s.close()
"
```

**Next session goal:** Create the `modules/our_led_sensor/` driver module, wire it to the app via overlay + prj.conf, build/flash for `our_board2/esp32s3/procpu`, verify LED toggles and serial logs, then push tags `l6-task1` and `l6-task2`.
