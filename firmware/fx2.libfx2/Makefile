TARGET    = fx2adf435xfw
LIBRARIES = fx2 fx2usb fx2isrs

LIBFX2 	= libfx2/firmware/library
include $(LIBFX2)/fx2rules.mk

all:	$(TARGET).ihex $(TARGET).iic

.PHONY: lib
lib:
	make -C libfx2/firmware/library

$(TARGET).iic:	$(TARGET).ihex
	../fx2/fx2lib/utils/ihx2iic.py --vid 0x0456 --pid 0xb40d --configbyte 0x01 $< $@

.PHONY: fw_load
fw_load: $(TARGET).ihex
	cycfx2prog -id=0x0456.b40d prg:$< run

.PHONY: fw_load_cy
fw_load_cy: $(TARGET).ihex
	cycfx2prog prg:$< run
