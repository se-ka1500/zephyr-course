# L5 Evidence — Custom Board Development

## Task 1: Copy/Rename Method

### Board File Tree
```
boards/our_board/
├── Kconfig
├── Kconfig.our_board
├── Kconfig.sysbuild
├── board.cmake
├── board.yml
├── our_board-pinctrl.dtsi
├── our_board_appcpu.dts
├── our_board_appcpu.yaml
├── our_board_appcpu_defconfig
├── our_board_procpu.dts
├── our_board_procpu.yaml
├── our_board_procpu_defconfig
├── doc/
└── support/
```

### board.yml
```yaml
board:
  name: our_board
  full_name: Our Board
  vendor: custom
  socs:
  - name: esp32s3
```

### Build Command
```bash
west build -b our_board/esp32s3/procpu \
  /Users/semihkaya/deps/zephyr/samples/hello_world \
  -- -DBOARD_ROOT=/Users/semihkaya/Desktop/workspace/iomico/zephyr-course-hw
```

### Flash Command
```bash
west flash --esp-device /dev/cu.usbserial-A5069RR4
```

### Serial Log
```
ESP-ROM:esp32s3-20210327
Build:Mar 27 2021
rst:0x1 (POWERON),boot:0x8 (SPI_FAST_FLASH_BOOT)
...
*** Booting Zephyr OS build v4.2.0 ***
Hello World! our_board/esp32s3/procpu
```

---

## Task 2: From Scratch Method

### Board File Tree
```
boards/our_board2/
├── CMakeLists.txt
├── Kconfig
├── Kconfig.our_board2
├── Kconfig.sysbuild
├── board.c
├── board.cmake
├── board.yml
├── our_board2-pinctrl.dtsi
├── our_board2_appcpu.dts
├── our_board2_appcpu.yaml
├── our_board2_appcpu_defconfig
├── our_board2_procpu.dts
├── our_board2_procpu.yaml
└── our_board2_procpu_defconfig
```

### board.yml
```yaml
board:
  name: our_board2
  full_name: Our Board 2
  vendor: custom
  socs:
  - name: esp32s3
```

### our_board2_procpu.dts (chosen + model)
```dts
/ {
	model = "Custom Our Board 2 PROCPU";
	compatible = "custom,our-board2";

	chosen {
		zephyr,sram = &sram1;
		zephyr,console = &uart0;
		zephyr,shell-uart = &uart0;
		zephyr,flash = &flash0;
		zephyr,code-partition = &slot0_partition;
		zephyr,bt-hci = &esp32_bt_hci;
	};
};
```

### Kconfig.our_board2
```kconfig
config BOARD_OUR_BOARD2
	select SOC_ESP32S3_WROOM_N8
	select SOC_ESP32S3_PROCPU if BOARD_OUR_BOARD2_ESP32S3_PROCPU
	select SOC_ESP32S3_APPCPU if BOARD_OUR_BOARD2_ESP32S3_APPCPU
```

### board.c — Init Hook
```c
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

static int board_our_board2_init(void)
{
	printk("Board Initialized\n");
	return 0;
}

SYS_INIT(board_our_board2_init, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
```

### Build Command
```bash
rm -rf build
west build -b our_board2/esp32s3/procpu \
  /Users/semihkaya/deps/zephyr/samples/hello_world \
  -- -DBOARD_ROOT=/Users/semihkaya/Desktop/workspace/iomico/zephyr-course-hw
```

### Flash Command
```bash
west flash --esp-device /dev/cu.usbserial-A5069RR4
```

### Serial Log
```
...
Board Initialized
*** Booting Zephyr OS build v4.2.0 ***
Hello World! our_board2/esp32s3/procpu
```

The line `Board Initialized` appears **before** the Zephyr boot banner and `Hello World!`, confirming the `board.c` init hook runs prior to `main()`.
