TARGET    = fx2adf435xfw_ex
LIBRARIES = fx2 fx2usb fx2isrs

LIBFX2 	= libfx2/firmware/library
include $(LIBFX2)/fx2rules.mk


.PHONY: lib
lib:
	make -C libfx2/firmware/library


.PHONY: fw_load
fw_load: $(TARGET).ihex
	cycfx2prog -id=0x0456.b40d prg:$< run


.PHONY: fw_load_cy
fw_load_cy: $(TARGET).ihex
	cycfx2prog prg:$< run
