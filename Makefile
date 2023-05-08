default: build/Makefile
	(cd build/ && make)

build/Makefile:
	mkdir -p build/
	(cd build/ && cmake ..)

.PHONY: default
