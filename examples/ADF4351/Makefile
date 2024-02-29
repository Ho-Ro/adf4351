all: ADF4351

ADF4351: main.o ADF4351.o USB.o
	g++ $^ -o $@ -l usb-1.0 -lm

main.o: main.cpp ADF4351.h USB.h
	g++ -c $< -o $@

ADF4351.o: ADF4351.cpp ADF4351.h
	g++ -c $< -o $@

USB.o: USB.cpp USB.h
	g++ -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o *~
