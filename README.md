pyadf435x
=========

`pyadf435x` is a suite of software and firmware for controlling the Analog
Devices ADF435x series of wide-band RF synthesizers.

The software suite consists of the following components:

* **adf435x** - A python library that can control the ADF4350/1 via various hardware interface back-ends.
* **adf435xctl** - A command line tool to control the ADF4350/1 manually.
* **adf435xinit** - A command line tool to transfer the FX2 firmware to the RAM of the EVAL-ADF4351 board.
* **fx2adf435xfw.ihx** - A firmware file for upload to the RAM of the Cypress FX2
  that replaces the proprietary firmware for the EVAL-ADF4351 board.
* **fx2adf435xfw.iic** - The same firmware in the file format for permanent storage in the EEPROM of the Cypress FX2.
* **stm32adf435xfw.bin**  - A similar (untested) firmware for the STM32F103, see [`README_stm32.md`](README_stm32.md).

It's also possible to use the [Bus Pirate](http://dangerousprototypes.com/docs/Bus_Pirate) as the interface for the `SPI`
communications, simply using the `adf435x.interfaces.BusPirate` class.

Another optional interface is the experimental [DigiSpark Tiny85 module](firmware/tinyADF) with Arduino SW
that receives USB serial commands, simply using the `adf435x.interfaces.tinyADF` class.

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

```
usage: adf435xctl [-h] [--device-type DEVICE_TYPE] [--freq FREQ] [--ref-freq REF_FREQ]
                  [--r-counter R_COUNTER] [--ref-doubler REF_DOUBLER] [--ref-div2 REF_DIV2]
                  [--feedback-select FEEDBACK_SELECT]
                  [--band-select-clock-divider BAND_SELECT_CLOCK_DIVIDER]
                  [--band-select-clock-mode BAND_SELECT_CLOCK_MODE] [--enable-gcd ENABLE_GCD]
                  [--int INT] [--frac FRAC] [--mod MOD] [--phase-value PHASE_VALUE]
                  [--prescaler PRESCALER] [--low-noise-spur-mode LOW_NOISE_SPUR_MODE]
                  [--mux-out MUX_OUT] [--ref-div-2 REF_DIV_2]
                  [--double-buff-r4 DOUBLE_BUFF_R4]
                  [--charge-pump-current CHARGE_PUMP_CURRENT] [--ldp LDP]
                  [--pd-polarity PD_POLARITY] [--powerdown POWERDOWN]
                  [--cp-three-state CP_THREE_STATE] [--counter-reset COUNTER_RESET]
                  [--abp ABP] [--charge-cancel CHARGE_CANCEL] [--csr CSR]
                  [--clk-div-mode CLK_DIV_MODE] [--clock-divider-value CLOCK_DIVIDER_VALUE]
                  [--output-divider OUTPUT_DIVIDER] [--vco-powerdown VCO_POWERDOWN]
                  [--mute-till-lock-detect MUTE_TILL_LOCK_DETECT]
                  [--aux-output-select AUX_OUTPUT_SELECT]
                  [--aux-output-enable AUX_OUTPUT_ENABLE]
                  [--aux-output-power AUX_OUTPUT_POWER] [--output-enable OUTPUT_ENABLE]
                  [--output-power OUTPUT_POWER] [--ld-pin-mode LD_PIN_MODE] [--r0 R0]
                  [--r1 R1] [--r2 R2] [--r3 R3] [--r4 R4] [--r5 R5] [-v]
                  [--interface INTERFACE]

Controls an ADF4350/1

optional arguments:
  -h, --help            show this help message and exit
  --device-type DEVICE_TYPE
  --freq FREQ, -f FREQ
  --ref-freq REF_FREQ
  --r-counter R_COUNTER
  --ref-doubler REF_DOUBLER
  --ref-div2 REF_DIV2
  --feedback-select FEEDBACK_SELECT
  --band-select-clock-divider BAND_SELECT_CLOCK_DIVIDER
  --band-select-clock-mode BAND_SELECT_CLOCK_MODE
  --enable-gcd ENABLE_GCD
  --int INT
  --frac FRAC
  --mod MOD
  --phase-value PHASE_VALUE
  --prescaler PRESCALER
  --low-noise-spur-mode LOW_NOISE_SPUR_MODE
  --mux-out MUX_OUT
  --ref-div-2 REF_DIV_2
  --double-buff-r4 DOUBLE_BUFF_R4
  --charge-pump-current CHARGE_PUMP_CURRENT
  --ldp LDP
  --pd-polarity PD_POLARITY
  --powerdown POWERDOWN
  --cp-three-state CP_THREE_STATE
  --counter-reset COUNTER_RESET
  --abp ABP
  --charge-cancel CHARGE_CANCEL
  --csr CSR
  --clk-div-mode CLK_DIV_MODE
  --clock-divider-value CLOCK_DIVIDER_VALUE
  --output-divider OUTPUT_DIVIDER
  --vco-powerdown VCO_POWERDOWN
  --mute-till-lock-detect MUTE_TILL_LOCK_DETECT
  --aux-output-select AUX_OUTPUT_SELECT
  --aux-output-enable AUX_OUTPUT_ENABLE
  --aux-output-power AUX_OUTPUT_POWER
  --output-enable OUTPUT_ENABLE
  --output-power OUTPUT_POWER
  --ld-pin-mode LD_PIN_MODE
  --r0 R0
  --r1 R1
  --r2 R2
  --r3 R3
  --r4 R4
  --r5 R5
  -v, --verbose
  --interface INTERFACE
                        INTERFACE: FX2, BusPirate, tinyADF, NONE
```

### Usage Examples

Sets the output frequency to 1000 MHz:
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
|  25 - PB0 | 30 - MUXOUT     |

The PB0 - MUXOUT connection is required only if you want to read the state of the MUXOUT pin,
e.g. to get the LD (lock detect) condition (set MUXOUT bits of reg 2 to 6).

### Building

1. First init/update all the sub-modules within the git repository, silence the message about changed submodule:
   ```sh
   git submodule update --init
   git update-index --assume-unchanged firmware/fx2/fx2lib
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

   You will get the firmware files `fx2adf435xfw.ihx` and `fx2adf435xfw.iic`

### Usage

1. Install *cycfx2prog*.

   On Debian/Ubuntu:
   ```sh
   sudo apt install cycfx2prog
   ```
2. Connect the FX2 board to the USB, it should come up with VID:PID `04b4:8613`.

3. Load the firware file `fx2adf435xfw.ihx` to the Cypress FX2 RAM with the following command:
   ```sh
   ./adf435xinit
   ```
   The device will now renumerate as an Analog Devices card with the VID:PID `0456:b40d`.
   The upload must be performed each time the module is connected to the PC.

4. You can also store the firmware file `fx2adf435xfw.iic` permanently in the *large* 8K or 16K EEPROM
   of the FX2 board, e.g. with the program [cyusb_linux](https://github.com/Ho-Ro/cyusb_linux).
   The unit will then enumerate on the USB as an Analog Devices card with the VID/PID `0456:b40d`
   and can be used immediately without prior firmware upload.

