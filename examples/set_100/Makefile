all: set_100

set_100: main.cpp
	gcc $< -o $@ -l usb-1.0

.PHONY: clean
clean:
	rm -f *~
