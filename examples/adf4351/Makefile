TARGET = adf4351

all: $(TARGET)

$(TARGET): main.o ADF4351.o USB.o
	g++ $^ -o $@ -l usb-1.0 -lm

main.o: main.cpp ADF4351.h USB.h Makefile
	g++ -Wall -c $< -o $@

ADF4351.o: ADF4351.cpp ADF4351.h Makefile
	g++ -Wall -c $< -o $@

USB.o: USB.cpp USB.h Makefile
	g++ -Wall -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o *~

.PHONY: distclean
distclean: clean
	rm -f $(TARGET)
