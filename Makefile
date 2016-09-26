CC = gcc
CFLAGS = -g -Wall --std=c99 -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=200809L -Icommon/thirdparty
BUILD = build

all: $(BUILD)/halo_scheduler $(BUILD)/halo_client.so

$(BUILD)/halo_client.so: halo_client.h halo_client.c common
	$(CC) $(CFLAGS) halo_client.c common/build/libcommon.a -fPIC -shared -o $(BUILD)/halo_client.so

$(BUILD)/halo_scheduler: halo.h halo.c common
	$(CC) $(CFLAGS) -o $@ halo.c common/build/libcommon.a common/thirdparty/hiredis/libhiredis.a -Icommon/thirdparty -Icommon/

common: FORCE
	git submodule update --init --recursive
	cd common; make

clean:
	cd common; make clean
	rm -r $(BUILD)/*

FORCE:
