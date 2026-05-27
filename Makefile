# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/Makefile

CC	?= cc
AR	?= ar
CFLAGS	?= -std=c17 -Wall -Wextra -Wconversion -Wsign-conversion -pedantic
CPPFLAGS += -Iinclude
LDLIBS	+= -lm

CORE_OBJS = \
	src/cp_agc.o \
	src/cp_block.o \
	src/cp_dc_blocker.o \
	src/cp_limiter.o \
	src/cp_meter.o

TEST_BINS = \
	tests/test_agc \
	tests/test_dc_blocker \
	tests/test_limiter \
	tests/test_meter

all: carrierpress

carrierpress: $(CORE_OBJS) src/main.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(CORE_OBJS) src/main.o $(LDLIBS)

libcarrierpress.a: $(CORE_OBJS)
	$(AR) rcs $@ $(CORE_OBJS)

tests/test_agc: tests/test_agc.o $(CORE_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ tests/test_agc.o $(CORE_OBJS) $(LDLIBS)

tests/test_dc_blocker: tests/test_dc_blocker.o $(CORE_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ tests/test_dc_blocker.o $(CORE_OBJS) $(LDLIBS)

tests/test_limiter: tests/test_limiter.o $(CORE_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ tests/test_limiter.o $(CORE_OBJS) $(LDLIBS)

tests/test_meter: tests/test_meter.o $(CORE_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ tests/test_meter.o $(CORE_OBJS) $(LDLIBS)

test: $(TEST_BINS)
	./tests/test_dc_blocker
	./tests/test_agc
	./tests/test_limiter
	./tests/test_meter

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -f carrierpress libcarrierpress.a src/*.o tests/*.o $(TEST_BINS)

.PHONY: all clean test
