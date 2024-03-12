TARGET = adf4351-eval

all: $(TARGET)

$(TARGET): main.o adf4351.o eval.o
	g++ $^ -o $@ -l usb-1.0 -lm

main.o: main.cpp adf4351.h eval.h Makefile
	g++ -Wall -c $< -o $@

adf4351.o: adf4351.cpp adf4351.h Makefile
	g++ -Wall -c $< -o $@

eval.o: eval.cpp eval.h Makefile
	g++ -Wall -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o *~

.PHONY: distclean
distclean: clean
	rm -f $(TARGET)
