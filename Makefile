CXX=g++
CXXFLAGS=-std=c++11 -Wall -Wextra -pedantic -fPIC
LDFLAGS=-shared

ifdef DEBUG
CFLAGS+=-g
CXXFLAGS+=-g
endif
ifndef DEBUG
CFLAGS+=-O3
CXXFLAGS+=-O3
endif

VSQL?=vsql

SDK?=/opt/vertica/sdk
VERTICA_SDK_INCLUDE=$(SDK)/include
INCLUDE=include/


.PHONY: build install uninstall examples clean


# Auxiliary tasks.

build: lib/GenerateSeriesLib.so

install: ddl/install.sql build
	$(VSQL) -f $<

uninstall: ddl/uninstall.sql
	$(VSQL) -f $<

examples:
	$(VSQL) -f examples/numbers.sql
	$(VSQL) -f examples/timestamps.sql

clean:
	rm -rf build/ lib/


# Build the library.

lib/GenerateSeriesLib.so: build/Vertica.o build/Series.o
	mkdir -p `dirname $@`
	$(CXX) $(LDFLAGS) -o $@ $^

build/Vertica.o: $(VERTICA_SDK_INCLUDE)/Vertica.cpp
	mkdir -p `dirname $@`
	$(CXX) -I $(VERTICA_SDK_INCLUDE) $(CXXFLAGS) -c -o $@ $<

build/Series.o: src/Series.cpp
	mkdir -p `dirname $@`
	$(CXX) -I $(VERTICA_SDK_INCLUDE) -I $(INCLUDE) $(CXXFLAGS) -c -o $@ $<
