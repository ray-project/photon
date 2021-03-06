CC = gcc
CFLAGS = -g -Wall --std=c99 -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=200809L -Icommon -Icommon/thirdparty -fPIC
BUILD = build

all: $(BUILD)/photon_scheduler $(BUILD)/photon_client.a

$(BUILD)/photon_client.a: photon_client.o
	ar rcs $(BUILD)/photon_client.a photon_client.o

$(BUILD)/photon_scheduler: photon.h photon_scheduler.c photon_algorithm.c common
	$(CC) $(CFLAGS) -o $@ photon_scheduler.c photon_algorithm.c common/build/libcommon.a common/thirdparty/hiredis/libhiredis.a -Icommon/thirdparty/ -Icommon/ ../plasma/build/libplasma_client.a -I../plasma/src/

common: FORCE
	git submodule update --init --recursive
	cd common; make

clean:
	cd common; make clean
	rm -r $(BUILD)/*
	rm *.o

FORCE:
