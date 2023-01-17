pyadf435x
=========

`pyadf435x` is a suite of software and firmware for controlling the Analog
Devices ADF435x series of wide-band RF synthesizers.

The software suite consists of the following components:

* **adf435x** - A python library that can control the ADF4350/1 via various
  hardware interface back-ends.
* **adf435xctl** - A command line tool to control the ADF4350/1 manually.
* **adf435xinit** - A command line tool to transfer the FX2 firmware to the RAM of the EVAL-ADF4351 board.
* **fx2adf435xfw.ihx** - A firmware file for upload to the RAM of the Cypress FX2
  that replaces the proprietary firmware for the EVAL-ADF4351 board.
* **fx2adf435xfw.iic** - A firmware file for permanent storage in the EEPROM of the Cypress FX2
  that replaces the proprietary firmware for the EVAL-ADF4351 board.
* **stm32adf435xfw.bin**  - A similar firmware for the STM32F103.

It's also possible to use the [Bus Pirate](http://dangerousprototypes.com/docs/Bus_Pirate) as the interface for the ``SPI``
communications, simply using the ``adf435x.interfaces.BusPirate`` class.

adf435x
-------

### Installation

1. Install depedencies:

   On Debian/Ubuntu:
   ```
   $ sudo apt install python3-setuptools python3-usb
   ```

2. Build a Debian package:
   ```
   $ make deb
   ```

3. Install the Debian package:
   ```
   $ make debinstall
   ```

### Usage

See `examples/` sub-directory.

adf435xctl
----------

Requires **adf435x** to be installed.

### Usage Examples

Sets the output frequency to 1000MHz:

```
./adf435xctl --freq=1000
```

fx2adf435xfw
------------

The Cypress FX2 firmware project controls the synthesizer over SPI by
bit-banging the FX2's GPIOs.

The firmware requires the following wiring:

|  FX2 Pin  |  ADF4350/1 Pin  |
|  -------  |  -------------  |
|  33 - PA0 |  3 - LE         |
|  34 - PA1 |  1 - CLK        |
|  35 - PA2 |  2 - DATA       |

### Building

1. First init/update all the sub-modules within the git repository:
   ```
   $ git submodule update --init
   ```

2. Install AutoTools and the SDCC (Small Devices C Compiler).

   On Debian/Ubuntu:
   ```
   $ sudo apt install autoconf automake make sdcc
   ```

3. Build the firmware:
   ```
   $ make firmware
   ```
   You will now have the firmware files `fx2adf435xfw.ihx` and `fx2adf435xfw.iic`

### Usage

1. Install *cycfx2prog*.

   On Debian/Ubuntu:
   ```
   $ sudo apt install cycfx2prog
   ```

2. Load the firware file `fx2adf435xfw.ihx` to the Cypress FX2 RAM with the following command:
   ```
   $ ./adf435xinit
   ```
   The device will now renumerate as an Analog Devices card with the VID/PID '0456:b40d'.
   The upload must be performed each time the module is connected to the PC.

3. You can also store the firmware file `fx2adf435xfw.iic` permanently in the *large* 16K EEPROM
   of the FX2 board, e.g. with the program [cyusb_linux](https://github.com/Ho-Ro/cyusb_linux).
   The unit registers on the USB as an Analog Devices card with the VID/PID '0456:b40d'.

stm32adf435xfw
--------------

The firmware requires the following wiring:

|  STM32F103 Pin  |  ADF4350/1 Pin  |
|  -------------  |  -------------  |
|  PA4            |  3 - LE         |
|  PA5            |  1 - CLK        |
|  PA7            |  2 - DAT        |

### Building & Installation

1. First init/update all the sub-modules within the git repository:
   ```
   $ git submodule update --init
   ```

2. Install GNU Make, OpenOCD, and ARM builds of GCC compiler and Newlib.

   On Debian/Ubuntu:
   ```
   sudo apt install gcc-arm-none-eabi libnewlib-arm-none-eabi make openocd
   ```

3. Build *libopencm3*:
   ```
   $ cd firmware/stm32/libopencm3
   $ make
   ```

4. Build *stm32f103adf435xfw*:
   ```
   $ cd ..
   $ make
   ```

5. Run OpenOCD:
   ```
   sudo openocd -f /usr/share/openocd/scripts/interface/stlink-v2.cfg -f /usr/share/openocd/scripts/target/stm32f1x.cfg
   ```

6. Install the firmware:
   ```
   telnet localhost 4444

   > reset halt
   > flash write_image erase /path/to/stm32adf435xfw.bin 0x08000000
   > reset
   ```
