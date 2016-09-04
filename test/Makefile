SHELL = /bin/bash
VPATH = ..:.
CXXC = g++ -Wall -I. -I..
CC = g++ -c -I. -I..
TESTGEN = /cygdrive/c/Users/Stefan/cxxtest-4.4/bin/cxxtestgen
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
 
stdio_printer.cpp: $(TESTS) parserTest
	$(TESTGEN) -o $@ --runner=StdioPrinter $(TESTS)
 
tests.cpp: $(TESTS)
	$(TESTGEN) -o $@ $(TESTS)
 
error_printer.o: error_printer.cpp
	$(CC) error_printer.cpp -o error_printer.o
 
error_printer: error_printer.o parser.o
	$(CXXC) error_printer.o parser.o -o error_printer
 
%: %.cpp
	$(CXXC) -o $@ $<