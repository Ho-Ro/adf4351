adf435xgui
==========

QT-based user interface for controlling the ADF4351 Eval board.

![adf435xgui](adf435xgui.png)

### Development

You need a libusb and QT development installation.
If you use `qtcreator` you can simply open the file `adf435xgui.pro` to edit the project.

### Building

Just type:
```sh
qmake
make
```
This creates the executable `adf435xgui` in the build directory,
either `linux` or `mac` or `windows`, depending on your OS.

