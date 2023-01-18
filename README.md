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
* **stm32adf435xfw.bin**  - A similar firmware for the STM32F103, see [`README_stm32.md`](README_stm32.md).

It's also possible to use the [Bus Pirate](http://dangerousprototypes.com/docs/Bus_Pirate) as the interface for the ``SPI``
communications, simply using the ``adf435x.interfaces.BusPirate`` class.

adf435x
-------

### Installation

1. Install depedencies:

   On Debian/Ubuntu:
   ```sh
   sudo apt install python3-setuptools python3-usb
   ```

2. Build a Debian package:
   ```sh
   make deb
   ```

3. Install the Debian package:
   ```sh
   make debinstall
   ```

### Usage

See `examples/` sub-directory.

adf435xctl
----------

Requires **adf435x** to be installed.

### Usage Examples

Sets the output frequency to 1000MHz:
```sh
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
   ```sh
   git submodule update --init
   ```

2. Install AutoTools and the SDCC (Small Devices C Compiler).

   On Debian/Ubuntu:
   ```sh
   sudo apt install autoconf automake make sdcc
   ```

3. Build the firmware:
   ```sh
   make firmware
   ```

   You will now have the firmware files `fx2adf435xfw.ihx` and `fx2adf435xfw.iic`

### Usage

1. Install *cycfx2prog*.

   On Debian/Ubuntu:
   ```sh
   sudo apt install cycfx2prog
   ```

2. Load the firware file `fx2adf435xfw.ihx` to the Cypress FX2 RAM with the following command:
   ```sh
   ./adf435xinit
   ```

   The device will now renumerate as an Analog Devices card with the VID/PID '0456:b40d'.
   The upload must be performed each time the module is connected to the PC.

3. You can also store the firmware file `fx2adf435xfw.iic` permanently in the *large* 16K EEPROM
   of the FX2 board, e.g. with the program [cyusb_linux](https://github.com/Ho-Ro/cyusb_linux).
   The unit registers on the USB as an Analog Devices card with the VID/PID '0456:b40d'.

