PROJECT = adf435x


VID = 0x0456
PID = 0xb40d
CONFIGBYTE = 0x01

IHX2IIC = firmware/fx2.fx2lib/fx2lib/utils/ihx2iic.py --vid $(VID) --pid $(PID) --configbyte $(CONFIGBYTE)
FX2EEPROM = fx2eeprom w $(VID) $(PID)

.PHONY: all
all: firmware gui examples


.PHONY: firmware
firmware: fx2adf435xfw.ihx fx2adf435xfw.iic


fx2adf435xfw.iic: fx2adf435xfw.ihx
	$(IHX2IIC) $< $@


fx2adf435xfw.ihx: firmware/fx2/fx2adf435xfw.ihex
	cp $< $@


firmware/fx2/fx2adf435xfw.ihex: firmware/fx2/main.c firmware/fx2/Makefile
	make -j4 -C firmware/fx2


.PHONY: firmware_stm32
firmware_stm32: stm32adf435xfw.bin


stm32adf435xfw.bin: firmware/stm32/stm32adf435xfw.bin
	cp $< $@
	chmod -x $@


firmware/stm32/stm32adf435xfw.bin: firmware/stm32/stm32adf435xfw.c
	make -j4 -C firmware/stm32/libopencm3
	make -j4 -C firmware/stm32


.PHONY:	store_fw
store_fw: fx2adf435xfw.iic
	$(FX2EEPROM) < $<


.PHONY:	upload_fw
upload_fw: fx2adf435xfw.ihx adf435xinit
	./adf435xinit


qtgui/Makefile: qtgui/adf435xgui.pro
	cd qtgui && qmake && cd ..


adf435xgui: qtgui/linux/adf435xgui
	cp $< .


.PHONY: gui
gui: adf435xgui


qtgui/linux/adf435xgui: qtgui/Makefile
	make -j -C qtgui

.PHONY: examples
examples:
	-make -C examples


# create a python source package
.PHONY:	sdist
sdist:
	python setup.py --command-packages=stdeb.command sdist


# create a debian source package
.PHONY:	dsc
dsc:
	python setup.py --command-packages=stdeb.command sdist_dsc


# create a debian binary package
.PHONY:	deb
deb:	distclean firmware gui examples
	git log --pretty="%cs: %s [%h]" > Changelog
	python setup.py --command-packages=stdeb.command bdist_deb
	-rm -f $(PROJECT)_*_all.deb $(PROJECT)-*.tar.gz
	ln `ls deb_dist/$(PROJECT)_*_all.deb | tail -1` .
	ls -l $(PROJECT)_*_all.deb


# install the latest debian package
.PHONY:	debinstall
debinstall:
	sudo dpkg -i $(PROJECT)_*_all.deb


# prepare a clean build
.PHONY:	clean
clean:
	python setup.py clean
	-rm -rf *~ .*~ deb_dist dist *.tar.gz *.egg* build tmp adf435xgui
	-make -C firmware/fx2 clean
	-make -C firmware/fx2.fx2lib clean
	-make -C firmware/stm32 clean
	-make -C qtgui clean
	-make -C examples clean


# removes all build artefacts
.PHONY:	distclean
distclean: clean
	-rm -Rf *.deb firmware/fx2.fx2lib/Makefile
	-make -j4 -C firmware/stm32/libopencm3 clean
	-make -C qtgui distclean
	-make -C examples distclean


# show the versions from python package and firmware
.PHONY:	show_versions
show_versions:
	-@grep VERSION  adf435x/core.py | cut -d= -f2
	-@fgrep .bcdDevice firmware/fx2/main.c | cut -d= -f2
	-@fgrep 'VER =' firmware/fx2.fx2lib/dscr.a51 | cut -d= -f2
