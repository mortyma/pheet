all: build

build:
	mkdir -p build
	cd build; cmake -DCMAKE_BUILD_TYPE="Debug" ..; make -j6

release:
	mkdir -p build
	cd build; cmake -DCMAKE_BUILD_TYPE="Release" ..; make -j6

check: build
	cd build; ctest --output-on-failure

clean:
	rm -rf build

.PHONY: build
