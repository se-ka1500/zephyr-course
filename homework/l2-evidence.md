# Lecture 2 Homework Evidence

## Assignment
1. Set up the development environment
2. Build and flash Hello World
   - Use `west build -b <board> zephyr/samples/hello_world`
   - Try with hardware board or `native_sim`
   - Verify boot banner

---

## 1. Development Environment

- **west**: v1.5.0
- **Zephyr SDK**: v0.17.4
- **Python**: 3.14.4
- **CMake**: 4.2.1
- **Ninja**: 1.13.2
- **QEMU**: installed (x86_64)
- **Board**: ESP32-S3-DevKitC (hardware) + qemu_x86 (emulator)

Workspace initialized from fork:
https://github.com/se-ka1500/zephyr-course

---

## 2a. Hardware Target — ESP32-S3-DevKitC

Build command:
```bash
west build -b esp32s3_devkitc/esp32s3/procpu zephyr/samples/hello_world
```

Flash command:
```bash
west flash --esp-device /dev/cu.usbserial-A5069RR4
```

### Boot Banner (serial output)

```
*** Booting Zephyr OS build v4.2.0 ***
Hello World! esp32s3_devkitc/esp32s3/procpu
```

Full serial log:
```
ESP-ROM:esp32s3-20210327
Build:Mar 27 2021
rst:0x1 (POWERON),boot:0x8 (SPI_FAST_FLASH_BOOT)
SPIWP:0xee
mode:DIO, clock div:1
load:0x3fc8c9f0,len:0x1588
load:0x40374000,len:0x89d8
SHA-256 comparison failed:
Calculated: bba5af938715ad5f40b3a797d33996841067e6cc2becfaba45f5024214045abd
Expected: 0000000060600000000000000000000000000000000000000000000000000000
Attempting to boot anyway...
entry 0x403777f0
I (53) soc_init: ESP Simple boot
I (53) soc_init: compile time Jun  2 2026 22:53:39
W (53) soc_init: Unicore bootloader
I (54) soc_init: chip revision: v0.2
I (56) flash_init: Boot SPI Speed : 80MHz
I (60) flash_init: SPI Mode       : DIO
I (63) flash_init: SPI Flash Size : 8MB
I (67) boot: DRAM: lma 0x00000020 vma 0x3fc8c9f0 len 0x1588   (5512)
I (73) boot: IRAM: lma 0x000015b0 vma 0x40374000 len 0x89d8   (35288)
I (79) boot: IRAM: lma 0x00009f98 vma 0x00000000 len 0x6060   (24672)
I (85) boot: IMAP: lma 0x00010000 vma 0x42000000 len 0x3af0   (15088)
I (91) boot: IRAM: lma 0x00013af8 vma 0x00000000 len 0xc500   (50432)
I (97) boot: DROM: lma 0x00020000 vma 0x3c010000 len 0x1090   (4240)
I (104) boot: Image with 6 segments
I (107) boot: IROM segment: paddr=00010000h, vaddr=42000000h, size=03AEEh ( 15086) map
I (114) boot: DROM segment: paddr=00020000h, vaddr=3c010000h, size=01090h (  4240) map
I (133) boot: libc heap size 354 kB.
I (134) spi_flash: detected chip: boya
I (134) spi_flash: flash io: dio
W (134) spi_flash: Detected size(16384k) larger than the size in the binary image header(8192k). Using the size in the binary image header.
*** Booting Zephyr OS build v4.2.0 ***
Hello World! esp32s3_devkitc/esp32s3/procpu
```

---

## 2b. Emulator Target — qemu_x86 (native_sim substitute on macOS)

> Note: `native_sim` is not supported on macOS. `qemu_x86` is used as the emulator substitute.

Build command:
```bash
west build -b qemu_x86 zephyr/samples/hello_world
```

Run command:
```bash
west build -t run
```

### Boot Banner (QEMU output)

```
SeaBIOS (version rel-1.17.0-0-gb52ca86e094d-prebuilt.qemu.org)
Booting from ROM..
*** Booting Zephyr OS build v4.2.0 ***
Hello World! qemu_x86/atom
```

---

## Conclusion

- Environment set up successfully
- `hello_world` built and flashed on ESP32-S3-DevKitC — boot banner verified
- `hello_world` built and run on `qemu_x86` — boot banner verified
