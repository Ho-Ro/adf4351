stm32adf435xfw
--------------

**The stm32 firmware build, installation and usage is not tested.**

The firmware requires the following wiring:

|  STM32F103 Pin  |  ADF4350/1 Pin  |
|  -------------  |  -------------  |
|  PA4            |  3 - LE         |
|  PA5            |  1 - CLK        |
|  PA7            |  2 - DAT        |

### Building & Installation

1. First init/update all the sub-modules within the git repository:
   ```sh
   git submodule add -f https://github.com/libopencm3/libopencm3.git firmware/stm32/libopencm3
   git submodule update --init
   ```

2. Install GNU Make, OpenOCD, and ARM builds of GCC compiler and Newlib.

   On Debian/Ubuntu:
   ```sh
   sudo apt install gcc-arm-none-eabi libnewlib-arm-none-eabi make openocd
   ```

3. Modify `firmware/stm32/libopencm3/Makefile`:
   ```diff
   --- firmware/stm32/libopencm3/Makefile.orig     2022-12-07 17:47:17.132272001 +0100
   +++ firmware/stm32/libopencm3/Makefile  2022-12-07 17:51:25.673466868 +0100
   @@ -25,7 +25,7 @@
    space:=
    space+=
   -SRCLIBDIR:= $(subst $(space),\$(space),$(realpath lib))
   +SRCLIBDIR:= $(subst $(space),/$(space),$(realpath lib))
    TARGETS := stm32/f0 stm32/f1 stm32/f2 stm32/f3 stm32/f4 stm32/f7
    TARGETS += stm32/l0 stm32/l1 stm32/l4
   ```

4. Build *libopencm3*:
   ```sh
   cd firmware/stm32/libopencm3
   make
   ```

5. Build *stm32f103adf435xfw*:
   ```sh
   cd ..
   make
   ```

6. Run OpenOCD:
   ```sh
   sudo openocd -f /usr/share/openocd/scripts/interface/stlink-v2.cfg -f /usr/share/openocd/scripts/target/stm32f1x.cfg
   ```

7. Install the firmware:
   ```sh
   telnet localhost 4444

   > reset halt
   > flash write_image erase /path/to/stm32adf435xfw.bin 0x08000000
   > reset
   ```
