TARGET = set_100

all: $(TARGET)

$(TARGET): main.cpp
	gcc $< -o $@ -l usb-1.0

.PHONY: clean
clean:
	rm -f *~

.PHONY: distclean
distclean: clean
	rm -f $(TARGET)
