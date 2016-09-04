SHELL = /bin/bash
VPATH = ..:.
CXXC = g++ -Wall -I. -I..
CC = g++ -c -I. -I..
TESTGEN = libs/cxxtest-4.4/bin/cxxtestgen
CFLAGS = -D_TEST_ -I../libraries/clock -Imocks -Ilibs/cxxtest-4.4 -Iarduino -std=c++11
TARGETS = error_printer
TESTS = *.h
 
all: $(TARGETS)
clean:
	rm -f *~ *.o *.obj $(TARGETS) $(GUI_TARGETS)
	rm -f tests.cpp error_printer.cpp
	rm -f qt_runner.cpp
 
distclean: clean
	rm -f Makefile
 
run: error_printer
	./error_printer
 
error_printer.cpp: $(TESTS)
	$(TESTGEN) -o $@ --error-printer $(TESTS)
 
stdio_printer.cpp: $(TESTS) clockTest
	$(TESTGEN) -o $@ --runner=StdioPrinter $(TESTS)
 
tests.cpp: $(TESTS)
	$(TESTGEN) -o $@ $(TESTS)

mock.o:
	$(CC) -c mocks/mock_arduino.cpp -o mock.o

clock.o:
	$(CC) $(CFLAGS) ../libraries/clock/clock.cpp -o clock.o
 
error_printer.o: error_printer.cpp
	$(CC) $(CFLAGS) error_printer.cpp -o error_printer.o
 
error_printer: error_printer.o clock.o mock.o
	$(CXXC) error_printer.o clock.o mock.o -o error_printer
 
%: %.cpp
	$(CXXC) -o $@ $<