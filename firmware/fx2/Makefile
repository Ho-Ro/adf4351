TARGET    = fx2adf435xfw
LIBRARIES = fx2 fx2usb fx2isrs

LIBFX2 	= libfx2/firmware/library
include $(LIBFX2)/fx2rules.mk


VID=0456
PID=b40d

CONFIGBYTE=0x01

IHX2IIC = ../fx2.fx2lib/fx2lib/utils/ihx2iic.py --vid 0x$(VID) --pid 0x$(PID) --configbyte $(CONFIGBYTE)


.PHONY: lib
lib:
	make -C libfx2/firmware/library


$(TARGET).iic: $(TARGET).ihex Makefile
	$(IHX2IIC) $< $@


.PHONY: ee_load
ee_load: $(TARGET).iic Makefile
	fx2tool -d $(VID):$(PID) -F bin -B write_eeprom -W2 -p32 -f $<


.PHONY: fw_load
fw_load: $(TARGET).ihex
	cycfx2prog -id=0x0456.b40d prg:$< run


.PHONY: fw_load_cy
fw_load_cy: $(TARGET).ihex
	cycfx2prog prg:$< run
