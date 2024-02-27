all: ADF435X

ADF435X: main.o ADF435X.o
	g++ $^ -o $@ -l usb-1.0 -lm

main.o: main.cpp ADF435X.h
	g++ -c $< -o $@

ADF435X.o: ADF435X.cpp ADF435X.h
	g++ -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o *~
