RSYNC = rsync
SSH = ssh

RFLAGS = --recursive --progress --copy-links --exclude=build/ --exclude=.git/ -e "$(SSH)"
SATURN = saturn
MARS = mars
REMOTEDIR = ~/pheet
SOURCE = .

all: debug

debug:
	mkdir -p build
	cd build; cmake -DCMAKE_BUILD_TYPE="Debug" ..; make -j6

release:
	mkdir -p build
	cd build; cmake -DCMAKE_BUILD_TYPE="Release" ..; make -j6

profile:
	mkdir -p build
	cd build; cmake -DCMAKE_BUILD_TYPE="Profile" ..; make -j6
	
test: debug
	cd build;  ./test/msp/test/msp-test 
	
saturn:
	$(RSYNC) $(RFLAGS) $(SOURCE) $(SATURN):$(REMOTEDIR)
	
mars:
	$(RSYNC) $(RFLAGS) $(SOURCE) $(MARS):$(REMOTEDIR)

	
clean:
	rm -rf build

.PHONY: clean debug release profile test saturn mars
