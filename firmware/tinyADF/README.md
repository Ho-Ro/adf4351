# tiny ADF4351 programmer"


Small frequency generator consisting of an ADF4351 board and a DigiSpark Tiny85 board
with micronucleus bootloader. USB serial interface for commands.

## Usage
```
usage: <hexvalue>[RTV]
<hexvalue>: 1..8 hex digits
R: write Register
T: tx Echo
V: Verbose
```

## HW connections
```
DATA  - PB0
LE    - PB1
CLK   - PB2
```
