# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/Makefile

CC	?= cc
AR	?= ar
CFLAGS	?= -std=c17 -Wall -Wextra -Wconversion -Wsign-conversion -pedantic
CPPFLAGS += -Iinclude
LDLIBS	+= -lm
WITH_SNDFILE ?= 0
WITH_PORTAUDIO ?= 0

BUILD_DIR = build
FEATURE_DIR = base
APP_OBJ_DIR = $(BUILD_DIR)/obj/$(FEATURE_DIR)/app
TEST_OBJ_DIR = $(BUILD_DIR)/obj/$(FEATURE_DIR)/test
TEST_BIN_DIR = $(BUILD_DIR)/tests

CORE_SRCS = \
	src/cp_agc.c \
	src/cp_am.c \
	src/cp_audio.c \
	src/cp_biquad.c \
	src/cp_block.c \
	src/cp_compressor.c \
	src/cp_crossover.c \
	src/cp_dc_blocker.c \
	src/cp_dehummer.c \
	src/cp_limiter.c \
	src/cp_meter.c \
	src/cp_multiband.c

APP_SRCS = src/main.c

TEST_SRCS = \
	tests/test_agc.c \
	tests/test_am.c \
	tests/test_audio.c \
	tests/test_biquad.c \
	tests/test_compressor.c \
	tests/test_crossover.c \
	tests/test_dc_blocker.c \
	tests/test_dehummer.c \
	tests/test_limiter.c \
	tests/test_meter.c \
	tests/test_multiband.c

ifeq ($(WITH_SNDFILE),1)
FEATURE_DIR := $(FEATURE_DIR)-sndfile
CPPFLAGS += -DCP_WITH_SNDFILE
LDLIBS	+= -lsndfile
CORE_SRCS += src/cp_wav.c
TEST_SRCS += tests/test_wav.c
SNDFILE_ORDER = check-sndfile
endif

ifeq ($(WITH_PORTAUDIO),1)
FEATURE_DIR := $(FEATURE_DIR)-portaudio
CPPFLAGS += -DCP_WITH_PORTAUDIO
LDLIBS	+= -lportaudio
CORE_SRCS += src/cp_portaudio.c
PORTAUDIO_ORDER = check-portaudio
endif

BACKEND_ORDER = $(SNDFILE_ORDER) $(PORTAUDIO_ORDER)

APP_CORE_OBJS = $(CORE_SRCS:src/%.c=$(APP_OBJ_DIR)/src/%.o)
APP_OBJS = $(APP_CORE_OBJS) $(APP_SRCS:src/%.c=$(APP_OBJ_DIR)/src/%.o)
TEST_CORE_OBJS = $(CORE_SRCS:src/%.c=$(TEST_OBJ_DIR)/src/%.o)
TEST_OBJS = $(TEST_SRCS:tests/%.c=$(TEST_OBJ_DIR)/tests/%.o)

TEST_BINS = \
	$(TEST_BIN_DIR)/test_agc \
	$(TEST_BIN_DIR)/test_am \
	$(TEST_BIN_DIR)/test_audio \
	$(TEST_BIN_DIR)/test_biquad \
	$(TEST_BIN_DIR)/test_compressor \
	$(TEST_BIN_DIR)/test_crossover \
	$(TEST_BIN_DIR)/test_dc_blocker \
	$(TEST_BIN_DIR)/test_dehummer \
	$(TEST_BIN_DIR)/test_limiter \
	$(TEST_BIN_DIR)/test_meter \
	$(TEST_BIN_DIR)/test_multiband

ifeq ($(WITH_SNDFILE),1)
TEST_BINS += $(TEST_BIN_DIR)/test_wav
endif

all: carrierpress

carrierpress: $(APP_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(APP_OBJS) $(LDLIBS)

libcarrierpress.a: $(APP_CORE_OBJS)
	$(AR) rcs $@ $(APP_CORE_OBJS)

$(TEST_BIN_DIR)/test_agc: $(TEST_OBJ_DIR)/tests/test_agc.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_agc.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_am: $(TEST_OBJ_DIR)/tests/test_am.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_am.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_audio: $(TEST_OBJ_DIR)/tests/test_audio.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_audio.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_biquad: $(TEST_OBJ_DIR)/tests/test_biquad.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_biquad.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_compressor: $(TEST_OBJ_DIR)/tests/test_compressor.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_compressor.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_crossover: $(TEST_OBJ_DIR)/tests/test_crossover.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_crossover.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_dc_blocker: $(TEST_OBJ_DIR)/tests/test_dc_blocker.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_dc_blocker.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_dehummer: $(TEST_OBJ_DIR)/tests/test_dehummer.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_dehummer.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_limiter: $(TEST_OBJ_DIR)/tests/test_limiter.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_limiter.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_meter: $(TEST_OBJ_DIR)/tests/test_meter.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_meter.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_multiband: $(TEST_OBJ_DIR)/tests/test_multiband.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_multiband.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_wav: $(TEST_OBJ_DIR)/tests/test_wav.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_wav.o $(TEST_CORE_OBJS) $(LDLIBS)

test: $(TEST_BINS)
	./$(TEST_BIN_DIR)/test_dc_blocker
	./$(TEST_BIN_DIR)/test_agc
	./$(TEST_BIN_DIR)/test_am
	./$(TEST_BIN_DIR)/test_audio
	./$(TEST_BIN_DIR)/test_biquad
	./$(TEST_BIN_DIR)/test_compressor
	./$(TEST_BIN_DIR)/test_crossover
	./$(TEST_BIN_DIR)/test_dehummer
	./$(TEST_BIN_DIR)/test_limiter
	./$(TEST_BIN_DIR)/test_meter
	./$(TEST_BIN_DIR)/test_multiband
ifeq ($(WITH_SNDFILE),1)
	./$(TEST_BIN_DIR)/test_wav
endif

check-sndfile:
	@mkdir -p $(BUILD_DIR)
	@printf '#include <sndfile.h>\nint main(void) { return 0; }\n' | $(CC) $(CPPFLAGS) $(CFLAGS) -x c - -o $(BUILD_DIR)/check-sndfile -lsndfile >/dev/null 2>&1 || { printf 'error: missing libsndfile development package/library. Install libsndfile (for example libsndfile1-dev, libsndfile-devel, or libsndfile).\n'; exit 1; }
	@rm -f $(BUILD_DIR)/check-sndfile

check-portaudio:
	@mkdir -p $(BUILD_DIR)
	@printf '#include <portaudio.h>\nint main(void) { return 0; }\n' | $(CC) $(CPPFLAGS) $(CFLAGS) -x c - -o $(BUILD_DIR)/check-portaudio -lportaudio >/dev/null 2>&1 || { printf 'error: missing PortAudio development package/library. Install PortAudio (for example portaudio19-dev, portaudio-devel, or portaudio).\n'; exit 1; }
	@rm -f $(BUILD_DIR)/check-portaudio

$(APP_OBJ_DIR)/src/%.o: src/%.c | $(BACKEND_ORDER)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(TEST_OBJ_DIR)/src/%.o: src/%.c | $(BACKEND_ORDER)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(TEST_OBJ_DIR)/tests/%.o: tests/%.c | $(BACKEND_ORDER)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR)
	rm -f carrierpress libcarrierpress.a src/*.o tests/*.o
	rm -f tests/test_agc tests/test_am tests/test_audio tests/test_biquad
	rm -f tests/test_compressor tests/test_crossover
	rm -f tests/test_dc_blocker tests/test_dehummer
	rm -f tests/test_limiter tests/test_meter
	rm -f tests/test_multiband
	rm -f tests/test_wav tests/wav_input.wav tests/wav_output.wav

.PHONY: all check-portaudio check-sndfile clean test
