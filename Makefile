PROJECT = pyadf435x

all:	firmware


.PHONY: firmware
firmware: fx2adf435xfw.ihx


fx2adf435xfw.ihx: firmware/fx2/fx2adf435xfw.ihx
	cp $< $@


firmware/fx2/fx2adf435xfw.ihx: firmware/fx2/fx2adf435xfw.c firmware/fx2/Makefile
	make -C firmware/fx2


firmware/fx2/Makefile: firmware/fx2/Makefile.am
	cd firmware/fx2 && ./autogen.sh && ./configure


.PHONY:	upload_fw
upload_fw: fx2adf435xfw.ihx adf435xinit
	./adf435xinit


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
deb:	clean fx2adf435xfw.ihx
	git log --pretty="%cs: %s [%h]" > Changelog
	python setup.py --command-packages=stdeb.command bdist_deb
	-rm -f $(PROJECT)_*_all.deb $(PROJECT)-*.tar.gz
	ln `ls deb_dist/$(PROJECT)_*_all.deb | tail -1` .
	ls -l $(PROJECT)_*_all.deb


# install the latest debian package
.PHONY:	debinstall
debinstall: deb
	sudo dpkg -i $(PROJECT)_*_all.deb


# prepare a clean build
.PHONY:	clean
clean:
	python setup.py clean
	-rm -rf *~ .*~ deb_dist dist *.tar.gz *.egg* build tmp
	-make -C firmware/fx2 clean
	-make -C firmware/stm32 clean


# removes all build artefacts
.PHONY:	distclean
distclean: clean
	-rm -f *.deb *.ihx firmware/fx2/Makefile

