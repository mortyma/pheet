RSYNC = rsync
SSH = ssh

RFLAGS = --recursive --progress --copy-links --exclude=build/ --exclude=.git/ -e "$(SSH)"
SATURN = saturn
MARS = mars
REMOTEDIR = ~/pheet
SOURCE = .

all: build

build:
	mkdir -p build
	cd build; cmake -DCMAKE_BUILD_TYPE="Debug" ..; make -j6

release:
	mkdir -p build
	cd build; cmake -DCMAKE_BUILD_TYPE="Release" ..; make -j6

profile:
	mkdir -p build
	cd build; cmake -DCMAKE_BUILD_TYPE="Profile" ..; make -j6
	
test: build
	cd build;  ./test/msp/test/msp-test 
	
saturn:
	$(RSYNC) $(RFLAGS) $(SOURCE) $(SATURN):$(REMOTEDIR)
	
mars:
	$(RSYNC) $(RFLAGS) $(SOURCE) $(MARS):$(REMOTEDIR)

	
clean:
	rm -rf build

.PHONY: build
