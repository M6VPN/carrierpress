# CarrierPress - Developed by M6VPN (M6VPN@tuta.com)
# carrierpress/Makefile

CC	?= cc
AR	?= ar
INSTALL	?= install
PKG_CONFIG ?= pkg-config
CFLAGS	?= -std=c17 -Wall -Wextra -Wconversion -Wsign-conversion -pedantic
CPPFLAGS += -Iinclude
LDLIBS	+= -lm
PREFIX	?= /usr/local
BINDIR	?= $(PREFIX)/bin
INCLUDEDIR ?= $(PREFIX)/include
LIBDIR	?= $(PREFIX)/lib
MANDIR	?= $(PREFIX)/share/man
PKGCONFIGDIR ?= $(LIBDIR)/pkgconfig
DESTDIR	?=
VERSION	 = 0.2.0
WITH_SNDFILE ?= 0
WITH_PORTAUDIO ?= 0
WITH_SNDIO ?= 0
WITH_TUI ?= 0
WITH_GUI ?= 0
WITH_FFTW ?= 0
WITH_HAMLIB ?= 0
WITH_FLRIG ?= 0

BUILD_DIR = build
DIST_DIR = $(BUILD_DIR)/dist
DIST_NAME = carrierpress-$(VERSION)
DIST_TARBALL = $(DIST_DIR)/$(DIST_NAME).tar.gz
FEATURE_DIR = base
APP_OBJ_DIR = $(BUILD_DIR)/obj/$(FEATURE_DIR)/app
TEST_OBJ_DIR = $(BUILD_DIR)/obj/$(FEATURE_DIR)/test
TEST_BIN_DIR = $(BUILD_DIR)/tests/$(FEATURE_DIR)

CORE_SRCS = \
	src/cp_agc.c \
	src/cp_am.c \
	src/cp_audio.c \
	src/cp_auto_eq.c \
	src/cp_batch.c \
	src/cp_bass_eq.c \
	src/cp_biquad.c \
	src/cp_block.c \
	src/cp_cat.c \
	src/cp_compressor.c \
	src/cp_config_file.c \
	src/cp_control.c \
	src/cp_crossover.c \
	src/cp_dc_blocker.c \
	src/cp_declipper.c \
	src/cp_dehummer.c \
	src/cp_gui_format.c \
	src/cp_limiter.c \
	src/cp_low_level_boost.c \
	src/cp_meter.c \
	src/cp_monitor.c \
	src/cp_multiband.c \
	src/cp_natural_dynamics.c \
	src/cp_playlist_check.c \
	src/cp_profile.c \
	src/cp_report.c \
	src/cp_report_tool.c \
	src/cp_restoration.c \
	src/cp_resampler.c \
	src/cp_ssb.c \
	src/cp_waveform.c

APP_SRCS = src/main.c

TEST_SRCS = \
	tests/test_agc.c \
	tests/test_am.c \
	tests/test_audio.c \
	tests/test_auto_eq.c \
	tests/test_batch.c \
	tests/test_bass_eq.c \
	tests/test_biquad.c \
	tests/test_cat.c \
	tests/test_chain_quality.c \
	tests/test_compressor.c \
	tests/test_config_file.c \
	tests/test_control.c \
	tests/test_crossover.c \
	tests/test_dc_blocker.c \
	tests/test_declipper.c \
	tests/test_dehummer.c \
	tests/test_gui_format.c \
	tests/test_limiter.c \
	tests/test_low_level_boost.c \
	tests/test_meter.c \
	tests/test_monitor.c \
	tests/test_multiband.c \
	tests/test_natural_dynamics.c \
	tests/test_playlist_check.c \
	tests/test_profile.c \
	tests/test_quality_report.c \
	tests/test_report.c \
	tests/test_report_tool.c \
	tests/test_restoration.c \
	tests/test_resampler.c \
	tests/test_ssb.c \
	tests/test_version.c \
	tests/test_waveform.c

HEADERS = $(wildcard include/*.h)

ifeq ($(WITH_SNDFILE),1)
FEATURE_DIR := $(FEATURE_DIR)-sndfile
CPPFLAGS += -DCP_WITH_SNDFILE
LDLIBS	+= -lsndfile
CORE_SRCS += src/cp_wav.c src/cp_batch_wav.c
TEST_SRCS += tests/test_wav.c tests/test_batch_wav.c
SNDFILE_ORDER = check-sndfile
endif

ifeq ($(WITH_PORTAUDIO),1)
FEATURE_DIR := $(FEATURE_DIR)-portaudio
CPPFLAGS += -DCP_WITH_PORTAUDIO
LDLIBS	+= -lportaudio
CORE_SRCS += src/cp_portaudio.c
PORTAUDIO_ORDER = check-portaudio
endif

ifeq ($(WITH_SNDIO),1)
FEATURE_DIR := $(FEATURE_DIR)-sndio
CPPFLAGS += -DCP_WITH_SNDIO
LDLIBS	+= -lsndio
CORE_SRCS += src/cp_sndio.c
SNDIO_ORDER = check-sndio
endif

ifeq ($(WITH_FLRIG),1)
FEATURE_DIR := $(FEATURE_DIR)-flrig
CPPFLAGS += -DCP_WITH_FLRIG
CORE_SRCS += src/cp_cat_flrig.c
TEST_SRCS += tests/test_cat_flrig.c
FLRIG_ORDER = check-flrig
endif

ifeq ($(WITH_HAMLIB),1)
FEATURE_DIR := $(FEATURE_DIR)-hamlib
CPPFLAGS += -DCP_WITH_HAMLIB
LDLIBS	+= -lhamlib
CORE_SRCS += src/cp_cat_hamlib.c
TEST_SRCS += tests/test_cat_hamlib.c
HAMLIB_ORDER = check-hamlib
endif

ifeq ($(WITH_SNDFILE),1)
ifeq ($(WITH_PORTAUDIO),1)
CPPFLAGS += -DCP_WITH_PLAYOUT
CORE_SRCS += src/cp_playout.c
TEST_SRCS += tests/test_playout.c
PLAYOUT_ENABLED = 1
endif
endif

ifeq ($(WITH_TUI),1)
FEATURE_DIR := $(FEATURE_DIR)-tui
CPPFLAGS += -DCP_WITH_TUI
LDLIBS	+= -lncurses
CORE_SRCS += src/cp_tui.c
TEST_SRCS += tests/test_tui.c
TUI_ORDER = check-tui
endif

ifeq ($(WITH_GUI),1)
FEATURE_DIR := $(FEATURE_DIR)-gui
CPPFLAGS += -DCP_WITH_GUI -DCP_WITH_GUI_SDL3 \
	$(shell $(PKG_CONFIG) --cflags sdl3 2>/dev/null)
LDLIBS += $(shell $(PKG_CONFIG) --libs sdl3 2>/dev/null)
CORE_SRCS += src/cp_gui_sdl3.c
GUI_ORDER = check-gui
endif

ifeq ($(WITH_FFTW),1)
FEATURE_DIR := $(FEATURE_DIR)-fftw
CPPFLAGS += -DCP_WITH_FFTW \
	$(shell $(PKG_CONFIG) --cflags fftw3f 2>/dev/null)
LDLIBS += $(shell $(PKG_CONFIG) --libs fftw3f 2>/dev/null || printf -- '-lfftw3f')
CORE_SRCS += src/cp_spectrum.c
TEST_SRCS += tests/test_spectrum.c
FFTW_ORDER = check-fftw
endif

FEATURE_SUMMARY_ORDER =
ifneq ($(WITH_SNDFILE)$(WITH_PORTAUDIO)$(WITH_SNDIO)$(WITH_TUI)$(WITH_GUI)$(WITH_FFTW)$(WITH_HAMLIB)$(WITH_FLRIG),00000000)
FEATURE_SUMMARY_ORDER = feature-summary
endif

BACKEND_ORDER = $(FEATURE_SUMMARY_ORDER) $(SNDFILE_ORDER) \
	$(PORTAUDIO_ORDER) $(SNDIO_ORDER) $(TUI_ORDER) $(GUI_ORDER) \
	$(FFTW_ORDER) $(HAMLIB_ORDER) $(FLRIG_ORDER)

APP_CORE_OBJS = $(CORE_SRCS:src/%.c=$(APP_OBJ_DIR)/src/%.o)
APP_OBJS = $(APP_CORE_OBJS) $(APP_SRCS:src/%.c=$(APP_OBJ_DIR)/src/%.o)
TEST_CORE_OBJS = $(CORE_SRCS:src/%.c=$(TEST_OBJ_DIR)/src/%.o)
TEST_OBJS = $(TEST_SRCS:tests/%.c=$(TEST_OBJ_DIR)/tests/%.o)
VALIDATION_BINS = \
	$(TEST_BIN_DIR)/test_validation \
	$(TEST_BIN_DIR)/test_chain_quality

QUALITY_BINS = \
	$(TEST_BIN_DIR)/test_quality_report

PROFESSIONAL_BINS = \
	$(TEST_BIN_DIR)/test_professional_check

TEST_BINS = \
	$(TEST_BIN_DIR)/test_agc \
	$(TEST_BIN_DIR)/test_am \
	$(TEST_BIN_DIR)/test_audio \
	$(TEST_BIN_DIR)/test_auto_eq \
	$(TEST_BIN_DIR)/test_batch \
	$(TEST_BIN_DIR)/test_bass_eq \
	$(TEST_BIN_DIR)/test_biquad \
	$(TEST_BIN_DIR)/test_cat \
	$(TEST_BIN_DIR)/test_compressor \
	$(TEST_BIN_DIR)/test_config_file \
	$(TEST_BIN_DIR)/test_control \
	$(TEST_BIN_DIR)/test_crossover \
	$(TEST_BIN_DIR)/test_dc_blocker \
	$(TEST_BIN_DIR)/test_declipper \
	$(TEST_BIN_DIR)/test_dehummer \
	$(TEST_BIN_DIR)/test_gui_format \
	$(TEST_BIN_DIR)/test_limiter \
	$(TEST_BIN_DIR)/test_low_level_boost \
	$(TEST_BIN_DIR)/test_meter \
	$(TEST_BIN_DIR)/test_monitor \
	$(TEST_BIN_DIR)/test_multiband \
	$(TEST_BIN_DIR)/test_natural_dynamics \
	$(TEST_BIN_DIR)/test_playlist_check \
	$(TEST_BIN_DIR)/test_profile \
	$(TEST_BIN_DIR)/test_report \
	$(TEST_BIN_DIR)/test_report_tool \
	$(TEST_BIN_DIR)/test_restoration \
	$(TEST_BIN_DIR)/test_resampler \
	$(TEST_BIN_DIR)/test_ssb \
	$(TEST_BIN_DIR)/test_version \
	$(TEST_BIN_DIR)/test_waveform

ifeq ($(WITH_SNDFILE),1)
TEST_BINS += $(TEST_BIN_DIR)/test_wav $(TEST_BIN_DIR)/test_batch_wav
endif

ifeq ($(PLAYOUT_ENABLED),1)
TEST_BINS += $(TEST_BIN_DIR)/test_playout
endif

ifeq ($(WITH_TUI),1)
TEST_BINS += $(TEST_BIN_DIR)/test_tui
endif

ifeq ($(WITH_FFTW),1)
TEST_BINS += $(TEST_BIN_DIR)/test_spectrum
endif

ifeq ($(WITH_FLRIG),1)
TEST_BINS += $(TEST_BIN_DIR)/test_cat_flrig
endif

ifeq ($(WITH_HAMLIB),1)
TEST_BINS += $(TEST_BIN_DIR)/test_cat_hamlib
endif

all: carrierpress

autodetect:
	@mkdir -p $(BUILD_DIR)
	@set -e; \
	sndfile=0; portaudio=0; ncurses=0; tui=0; sndio=0; fftw=0; \
	gui=0; \
	if printf '#include <sndfile.h>\nint main(void) { return 0; }\n' | $(CC) $(CPPFLAGS) $(CFLAGS) -x c - -o $(BUILD_DIR)/autodetect-sndfile -lsndfile >/dev/null 2>&1; then \
		sndfile=1; \
	else \
		printf 'missing: libsndfile development package/library. Install libsndfile (for example libsndfile1-dev, libsndfile-devel, or libsndfile).\n'; \
	fi; \
	rm -f $(BUILD_DIR)/autodetect-sndfile; \
	if printf '#include <portaudio.h>\nint main(void) { return 0; }\n' | $(CC) $(CPPFLAGS) $(CFLAGS) -x c - -o $(BUILD_DIR)/autodetect-portaudio -lportaudio >/dev/null 2>&1; then \
		portaudio=1; \
	else \
		printf 'missing: PortAudio development package/library. Install PortAudio (for example portaudio19-dev, portaudio-devel, or portaudio).\n'; \
	fi; \
	rm -f $(BUILD_DIR)/autodetect-portaudio; \
	if printf '#include <curses.h>\nint main(void) { initscr(); endwin(); return 0; }\n' | $(CC) $(CPPFLAGS) $(CFLAGS) -x c - -o $(BUILD_DIR)/autodetect-tui -lncurses >/dev/null 2>&1; then \
		ncurses=1; \
	else \
		printf 'missing: ncurses development package/library. Install ncurses (for example libncurses-dev, ncurses-devel, or ncurses).\n'; \
	fi; \
	rm -f $(BUILD_DIR)/autodetect-tui; \
	if printf '#include <sndio.h>\nint main(void) { return 0; }\n' | $(CC) $(CPPFLAGS) $(CFLAGS) -x c - -o $(BUILD_DIR)/autodetect-sndio -lsndio >/dev/null 2>&1; then \
		sndio=1; \
	else \
		printf 'missing: sndio development package/library. Install sndio (for example sndio, sndio-devel, or libsndio-dev).\n'; \
	fi; \
	rm -f $(BUILD_DIR)/autodetect-sndio; \
	if $(PKG_CONFIG) --exists sdl3 >/dev/null 2>&1 && printf '#include <SDL3/SDL.h>\nint main(void) { SDL_Renderer *r = 0; (void)r; return 0; }\n' | $(CC) $(CPPFLAGS) $(CFLAGS) $$($(PKG_CONFIG) --cflags sdl3) -x c - -o $(BUILD_DIR)/autodetect-gui $$($(PKG_CONFIG) --libs sdl3) >/dev/null 2>&1; then \
		gui=1; \
	else \
		printf 'missing: SDL3 development package/library. Install SDL3, libsdl3-dev, SDL3-devel, or sdl3.\n'; \
	fi; \
	rm -f $(BUILD_DIR)/autodetect-gui; \
	if $(PKG_CONFIG) --exists fftw3f >/dev/null 2>&1 && printf '#include <fftw3.h>\nint main(void) { fftwf_complex x[2]; (void)x; return 0; }\n' | $(CC) $(CPPFLAGS) $(CFLAGS) $$($(PKG_CONFIG) --cflags fftw3f) -x c - -o $(BUILD_DIR)/autodetect-fftw $$($(PKG_CONFIG) --libs fftw3f) >/dev/null 2>&1; then \
		fftw=1; \
	else \
		printf 'missing: FFTW single-precision development package/library. Install FFTW/fftw3f (for example libfftw3-dev, fftw-devel, or fftw3).\n'; \
	fi; \
	rm -f $(BUILD_DIR)/autodetect-fftw; \
	if [ "$$portaudio" = 1 ] && [ "$$ncurses" = 1 ]; then \
		tui=1; \
	fi; \
	printf 'CarrierPress autodetect summary:\n'; \
	printf '  libsndfile: %s\n' "$$([ "$$sndfile" = 1 ] && printf enabled || printf disabled)"; \
	printf '  PortAudio: %s\n' "$$([ "$$portaudio" = 1 ] && printf enabled || printf disabled)"; \
	printf '  ncurses TUI: %s\n' "$$([ "$$tui" = 1 ] && printf enabled || printf disabled)"; \
	printf '  SDL3 GUI: %s (manual WITH_GUI=1, not auto-enabled)\n' "$$([ "$$gui" = 1 ] && printf detected || printf missing)"; \
	printf '  FFTW spectrum: %s (manual WITH_FFTW=1, not auto-enabled)\n' "$$([ "$$fftw" = 1 ] && printf detected || printf missing)"; \
	printf '  sndio: %s (deferred Linux path, not auto-enabled)\n' "$$([ "$$sndio" = 1 ] && printf detected || printf missing)"; \
	printf '  hamlib CAT: manual WITH_HAMLIB=1 (not auto-enabled)\n'; \
	printf '  flrig CAT: manual WITH_FLRIG=1 (not auto-enabled)\n'; \
	$(MAKE) WITH_SNDFILE=$$sndfile WITH_PORTAUDIO=$$portaudio WITH_TUI=$$tui WITH_SNDIO=0 all

carrierpress: $(APP_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(APP_OBJS) $(LDLIBS)

libcarrierpress.a: $(APP_CORE_OBJS)
	$(AR) rcs $@ $(APP_CORE_OBJS)

$(BUILD_DIR)/carrierpress.pc: carrierpress.pc.in
	@mkdir -p $(BUILD_DIR)
	sed -e 's|@PREFIX@|$(PREFIX)|g' \
	    -e 's|@LIBDIR@|$(LIBDIR)|g' \
	    -e 's|@INCLUDEDIR@|$(INCLUDEDIR)|g' \
	    -e 's|@VERSION@|$(VERSION)|g' \
	    carrierpress.pc.in > $@

$(TEST_BIN_DIR)/test_agc: $(TEST_OBJ_DIR)/tests/test_agc.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_agc.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_am: $(TEST_OBJ_DIR)/tests/test_am.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_am.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_audio: $(TEST_OBJ_DIR)/tests/test_audio.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_audio.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_auto_eq: $(TEST_OBJ_DIR)/tests/test_auto_eq.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_auto_eq.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_batch: $(TEST_OBJ_DIR)/tests/test_batch.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_batch.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_batch_wav: $(TEST_OBJ_DIR)/tests/test_batch_wav.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_batch_wav.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_bass_eq: $(TEST_OBJ_DIR)/tests/test_bass_eq.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_bass_eq.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_biquad: $(TEST_OBJ_DIR)/tests/test_biquad.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_biquad.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_chain_quality: $(TEST_OBJ_DIR)/tests/test_chain_quality.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_chain_quality.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_cat: $(TEST_OBJ_DIR)/tests/test_cat.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_cat.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_cat_flrig: $(TEST_OBJ_DIR)/tests/test_cat_flrig.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_cat_flrig.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_cat_hamlib: $(TEST_OBJ_DIR)/tests/test_cat_hamlib.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_cat_hamlib.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_compressor: $(TEST_OBJ_DIR)/tests/test_compressor.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_compressor.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_config_file: $(TEST_OBJ_DIR)/tests/test_config_file.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_config_file.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_control: $(TEST_OBJ_DIR)/tests/test_control.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_control.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_crossover: $(TEST_OBJ_DIR)/tests/test_crossover.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_crossover.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_dc_blocker: $(TEST_OBJ_DIR)/tests/test_dc_blocker.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_dc_blocker.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_declipper: $(TEST_OBJ_DIR)/tests/test_declipper.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_declipper.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_dehummer: $(TEST_OBJ_DIR)/tests/test_dehummer.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_dehummer.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_gui_format: $(TEST_OBJ_DIR)/tests/test_gui_format.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_gui_format.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_limiter: $(TEST_OBJ_DIR)/tests/test_limiter.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_limiter.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_low_level_boost: $(TEST_OBJ_DIR)/tests/test_low_level_boost.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_low_level_boost.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_meter: $(TEST_OBJ_DIR)/tests/test_meter.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_meter.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_monitor: $(TEST_OBJ_DIR)/tests/test_monitor.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_monitor.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_multiband: $(TEST_OBJ_DIR)/tests/test_multiband.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_multiband.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_natural_dynamics: $(TEST_OBJ_DIR)/tests/test_natural_dynamics.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_natural_dynamics.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_playlist_check: $(TEST_OBJ_DIR)/tests/test_playlist_check.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_playlist_check.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_resampler: $(TEST_OBJ_DIR)/tests/test_resampler.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_resampler.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_restoration: $(TEST_OBJ_DIR)/tests/test_restoration.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_restoration.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_ssb: $(TEST_OBJ_DIR)/tests/test_ssb.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_ssb.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_spectrum: $(TEST_OBJ_DIR)/tests/test_spectrum.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_spectrum.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_tui: $(TEST_OBJ_DIR)/tests/test_tui.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_tui.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_playout: $(TEST_OBJ_DIR)/tests/test_playout.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_playout.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_quality_report: $(TEST_OBJ_DIR)/tests/test_quality_report.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_quality_report.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_report: $(TEST_OBJ_DIR)/tests/test_report.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_report.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_report_tool: $(TEST_OBJ_DIR)/tests/test_report_tool.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_report_tool.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_professional_check: $(TEST_OBJ_DIR)/tests/test_professional_check.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_professional_check.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_profile: $(TEST_OBJ_DIR)/tests/test_profile.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_profile.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_wav: $(TEST_OBJ_DIR)/tests/test_wav.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_wav.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_waveform: $(TEST_OBJ_DIR)/tests/test_waveform.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_waveform.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_validation: $(TEST_OBJ_DIR)/tests/test_validation.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_validation.o $(TEST_CORE_OBJS) $(LDLIBS)

$(TEST_BIN_DIR)/test_version: $(TEST_OBJ_DIR)/tests/test_version.o $(TEST_CORE_OBJS)
	@mkdir -p $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(TEST_OBJ_DIR)/tests/test_version.o $(TEST_CORE_OBJS) $(LDLIBS)

test: $(TEST_BINS)
	./$(TEST_BIN_DIR)/test_dc_blocker
	./$(TEST_BIN_DIR)/test_agc
	./$(TEST_BIN_DIR)/test_am
	./$(TEST_BIN_DIR)/test_audio
	./$(TEST_BIN_DIR)/test_auto_eq
	./$(TEST_BIN_DIR)/test_batch
	./$(TEST_BIN_DIR)/test_bass_eq
	./$(TEST_BIN_DIR)/test_biquad
	./$(TEST_BIN_DIR)/test_cat
	./$(TEST_BIN_DIR)/test_compressor
	./$(TEST_BIN_DIR)/test_config_file
	./$(TEST_BIN_DIR)/test_control
	./$(TEST_BIN_DIR)/test_crossover
	./$(TEST_BIN_DIR)/test_declipper
	./$(TEST_BIN_DIR)/test_dehummer
	./$(TEST_BIN_DIR)/test_gui_format
	./$(TEST_BIN_DIR)/test_limiter
	./$(TEST_BIN_DIR)/test_low_level_boost
	./$(TEST_BIN_DIR)/test_meter
	./$(TEST_BIN_DIR)/test_monitor
	./$(TEST_BIN_DIR)/test_multiband
	./$(TEST_BIN_DIR)/test_natural_dynamics
	./$(TEST_BIN_DIR)/test_playlist_check
	./$(TEST_BIN_DIR)/test_profile
	./$(TEST_BIN_DIR)/test_report
	./$(TEST_BIN_DIR)/test_report_tool
	./$(TEST_BIN_DIR)/test_restoration
	./$(TEST_BIN_DIR)/test_resampler
	./$(TEST_BIN_DIR)/test_ssb
	./$(TEST_BIN_DIR)/test_version
	./$(TEST_BIN_DIR)/test_waveform
ifeq ($(WITH_SNDFILE),1)
	./$(TEST_BIN_DIR)/test_wav
	./$(TEST_BIN_DIR)/test_batch_wav
endif
ifeq ($(PLAYOUT_ENABLED),1)
	./$(TEST_BIN_DIR)/test_playout
endif
ifeq ($(WITH_TUI),1)
	./$(TEST_BIN_DIR)/test_tui
endif
ifeq ($(WITH_FFTW),1)
	./$(TEST_BIN_DIR)/test_spectrum
endif
ifeq ($(WITH_FLRIG),1)
	./$(TEST_BIN_DIR)/test_cat_flrig
endif
ifeq ($(WITH_HAMLIB),1)
	./$(TEST_BIN_DIR)/test_cat_hamlib
endif

validate: $(VALIDATION_BINS)
	./$(TEST_BIN_DIR)/test_validation
	./$(TEST_BIN_DIR)/test_chain_quality

quality: $(QUALITY_BINS)
	./$(TEST_BIN_DIR)/test_quality_report

quality-json: $(QUALITY_BINS)
	@./$(TEST_BIN_DIR)/test_quality_report --json

professional-check: $(PROFESSIONAL_BINS)
	./$(TEST_BIN_DIR)/test_professional_check

release-check:
	./scripts/release-check.sh

install: carrierpress libcarrierpress.a $(BUILD_DIR)/carrierpress.pc docs/carrierpress.1
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) -d $(DESTDIR)$(INCLUDEDIR)/carrierpress
	$(INSTALL) -d $(DESTDIR)$(LIBDIR)
	$(INSTALL) -d $(DESTDIR)$(PKGCONFIGDIR)
	$(INSTALL) -d $(DESTDIR)$(MANDIR)/man1
	$(INSTALL) -m 755 carrierpress $(DESTDIR)$(BINDIR)/carrierpress
	$(INSTALL) -m 644 $(HEADERS) $(DESTDIR)$(INCLUDEDIR)/carrierpress/
	$(INSTALL) -m 644 libcarrierpress.a $(DESTDIR)$(LIBDIR)/libcarrierpress.a
	$(INSTALL) -m 644 $(BUILD_DIR)/carrierpress.pc $(DESTDIR)$(PKGCONFIGDIR)/carrierpress.pc
	$(INSTALL) -m 644 docs/carrierpress.1 $(DESTDIR)$(MANDIR)/man1/carrierpress.1

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/carrierpress
	rm -f $(DESTDIR)$(LIBDIR)/libcarrierpress.a
	rm -f $(DESTDIR)$(PKGCONFIGDIR)/carrierpress.pc
	rm -f $(DESTDIR)$(MANDIR)/man1/carrierpress.1
	rm -rf $(DESTDIR)$(INCLUDEDIR)/carrierpress

install-smoke:
	rm -rf $(BUILD_DIR)/stage
	$(MAKE) all
	$(MAKE) DESTDIR=$(BUILD_DIR)/stage PREFIX=/usr install
	test -x $(BUILD_DIR)/stage/usr/bin/carrierpress
	test -f $(BUILD_DIR)/stage/usr/lib/libcarrierpress.a
	test -f $(BUILD_DIR)/stage/usr/lib/pkgconfig/carrierpress.pc
	test -f $(BUILD_DIR)/stage/usr/share/man/man1/carrierpress.1
	test -f $(BUILD_DIR)/stage/usr/include/carrierpress/carrierpress.h

install-manifest: install-smoke
	find $(BUILD_DIR)/stage -type f | sort

dist:
	mkdir -p $(DIST_DIR)
	git archive --format=tar.gz --prefix=$(DIST_NAME)/ HEAD > $(DIST_TARBALL)
	sha256sum $(DIST_TARBALL) > $(DIST_TARBALL).sha256

dist-check: dist
	test -s $(DIST_TARBALL)
	test -s $(DIST_TARBALL).sha256
	tar -tzf $(DIST_TARBALL) >/dev/null
	sha256sum -c $(DIST_TARBALL).sha256

feature-summary:
	@printf 'CarrierPress feature summary:\n'
	@printf '  libsndfile: %s\n' "$$([ "$(WITH_SNDFILE)" = 1 ] && printf enabled || printf disabled)"
	@printf '  PortAudio: %s\n' "$$([ "$(WITH_PORTAUDIO)" = 1 ] && printf enabled || printf disabled)"
	@printf '  ncurses TUI: %s\n' "$$([ "$(WITH_TUI)" = 1 ] && printf enabled || printf disabled)"
	@printf '  SDL3 GUI: %s%s\n' "$$([ "$(WITH_GUI)" = 1 ] && printf enabled || printf disabled)" "$$([ "$(WITH_GUI)" = 1 ] && printf ' (monitor)' || printf ' (manual WITH_GUI=1)')"
	@printf '  FFTW spectrum: %s%s\n' "$$([ "$(WITH_FFTW)" = 1 ] && printf enabled || printf disabled)" "$$([ "$(WITH_FFTW)" = 1 ] && printf ' (monitor)' || printf ' (manual WITH_FFTW=1)')"
	@printf '  sndio: %s%s\n' "$$([ "$(WITH_SNDIO)" = 1 ] && printf enabled || printf disabled)" "$$([ "$(WITH_SNDIO)" = 1 ] && printf ' (deferred Linux path)' || printf '')"
	@printf '  hamlib CAT: %s%s\n' "$$([ "$(WITH_HAMLIB)" = 1 ] && printf enabled || printf disabled)" "$$([ "$(WITH_HAMLIB)" = 1 ] && printf ' (read-only)' || printf ' (manual WITH_HAMLIB=1)')"
	@printf '  flrig CAT: %s%s\n' "$$([ "$(WITH_FLRIG)" = 1 ] && printf enabled || printf disabled)" "$$([ "$(WITH_FLRIG)" = 1 ] && printf ' (read-only XML-RPC)' || printf ' (manual WITH_FLRIG=1)')"

check-sndfile:
	@mkdir -p $(BUILD_DIR)
	@printf '#include <sndfile.h>\nint main(void) { return 0; }\n' | $(CC) $(CPPFLAGS) $(CFLAGS) -x c - -o $(BUILD_DIR)/check-sndfile -lsndfile >/dev/null 2>&1 || { printf 'error: missing libsndfile development package/library. Install libsndfile (for example libsndfile1-dev, libsndfile-devel, or libsndfile).\n'; exit 1; }
	@rm -f $(BUILD_DIR)/check-sndfile

check-portaudio:
	@mkdir -p $(BUILD_DIR)
	@printf '#include <portaudio.h>\nint main(void) { return 0; }\n' | $(CC) $(CPPFLAGS) $(CFLAGS) -x c - -o $(BUILD_DIR)/check-portaudio -lportaudio >/dev/null 2>&1 || { printf 'error: missing PortAudio development package/library. Install PortAudio (for example portaudio19-dev, portaudio-devel, or portaudio).\n'; exit 1; }
	@rm -f $(BUILD_DIR)/check-portaudio

check-sndio:
	@mkdir -p $(BUILD_DIR)
	@printf '#include <sndio.h>\nint main(void) { return 0; }\n' | $(CC) $(CPPFLAGS) $(CFLAGS) -x c - -o $(BUILD_DIR)/check-sndio -lsndio >/dev/null 2>&1 || { printf 'error: missing sndio development package/library. Install sndio (for example sndio, sndio-devel, or libsndio-dev).\n'; exit 1; }
	@rm -f $(BUILD_DIR)/check-sndio

check-tui:
	@mkdir -p $(BUILD_DIR)
	@printf '#include <curses.h>\nint main(void) { initscr(); endwin(); return 0; }\n' | $(CC) $(CPPFLAGS) $(CFLAGS) -x c - -o $(BUILD_DIR)/check-tui -lncurses >/dev/null 2>&1 || { printf 'error: missing ncurses development package/library. Install ncurses (for example libncurses-dev, ncurses-devel, or ncurses).\n'; exit 1; }
	@rm -f $(BUILD_DIR)/check-tui

check-gui:
	@mkdir -p $(BUILD_DIR)
	@$(PKG_CONFIG) --exists sdl3 >/dev/null 2>&1 || { printf 'error: missing SDL3 development package/library. Install SDL3, libsdl3-dev, SDL3-devel, or sdl3.\n'; exit 1; }
	@printf '#include <SDL3/SDL.h>\nint main(void) { SDL_Window *w = 0; SDL_Renderer *r = 0; (void)SDL_CreateWindowAndRenderer("test", 32, 32, 0, &w, &r); if (r != 0) (void)SDL_RenderDebugText(r, 0.0f, 0.0f, "x"); return 0; }\n' | $(CC) $(CPPFLAGS) $(CFLAGS) $$($(PKG_CONFIG) --cflags sdl3) -x c - -o $(BUILD_DIR)/check-gui $$($(PKG_CONFIG) --libs sdl3) >/dev/null 2>&1 || { printf 'error: missing SDL3 development package/library. Install SDL3, libsdl3-dev, SDL3-devel, or sdl3.\n'; exit 1; }
	@rm -f $(BUILD_DIR)/check-gui

check-fftw:
	@mkdir -p $(BUILD_DIR)
	@if $(PKG_CONFIG) --exists fftw3f >/dev/null 2>&1; then \
		printf '#include <fftw3.h>\nint main(void) { fftwf_complex x[2]; (void)x; return 0; }\n' | $(CC) $(CPPFLAGS) $(CFLAGS) $$($(PKG_CONFIG) --cflags fftw3f) -x c - -o $(BUILD_DIR)/check-fftw $$($(PKG_CONFIG) --libs fftw3f) >/dev/null 2>&1 || { printf 'error: missing FFTW single-precision development package/library. Install FFTW/fftw3f (for example libfftw3-dev, fftw-devel, or fftw3).\n'; exit 1; }; \
	else \
		printf '#include <fftw3.h>\nint main(void) { fftwf_complex x[2]; (void)x; return 0; }\n' | $(CC) $(CPPFLAGS) $(CFLAGS) -x c - -o $(BUILD_DIR)/check-fftw -lfftw3f >/dev/null 2>&1 || { printf 'error: missing FFTW single-precision development package/library. Install FFTW/fftw3f (for example libfftw3-dev, fftw-devel, or fftw3).\n'; exit 1; }; \
	fi
	@rm -f $(BUILD_DIR)/check-fftw

check-flrig:
	@mkdir -p $(BUILD_DIR)
	@printf '#include <sys/types.h>\n#include <sys/socket.h>\n#include <netdb.h>\n#include <poll.h>\nint main(void) { return 0; }\n' | $(CC) $(CPPFLAGS) $(CFLAGS) -x c - -o $(BUILD_DIR)/check-flrig >/dev/null 2>&1 || { printf 'error: missing POSIX socket headers required for WITH_FLRIG=1.\n'; exit 1; }
	@rm -f $(BUILD_DIR)/check-flrig

check-hamlib:
	@mkdir -p $(BUILD_DIR)
	@printf '#include <hamlib/rig.h>\nint main(void) { return 0; }\n' | $(CC) $(CPPFLAGS) $(CFLAGS) -x c - -o $(BUILD_DIR)/check-hamlib -lhamlib >/dev/null 2>&1 || { printf 'error: missing hamlib development package/library. Install hamlib, libhamlib, libhamlib-dev, or hamlib-devel.\n'; exit 1; }
	@rm -f $(BUILD_DIR)/check-hamlib

$(APP_OBJ_DIR)/src/%.o: src/%.c $(HEADERS) | $(BACKEND_ORDER)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(TEST_OBJ_DIR)/src/%.o: src/%.c $(HEADERS) | $(BACKEND_ORDER)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(TEST_OBJ_DIR)/tests/%.o: tests/%.c $(HEADERS) | $(BACKEND_ORDER)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR)
	rm -f carrierpress libcarrierpress.a src/*.o tests/*.o
	rm -f tests/test_agc tests/test_am tests/test_audio tests/test_auto_eq
	rm -f tests/test_biquad tests/test_cat tests/test_cat_flrig
	rm -f tests/test_cat_hamlib
	rm -f tests/test_bass_eq
	rm -f tests/test_chain_quality
	rm -f tests/test_compressor tests/test_crossover
	rm -f tests/test_control
	rm -f tests/test_dc_blocker tests/test_declipper tests/test_dehummer
	rm -f tests/test_gui_format
	rm -f tests/test_limiter tests/test_low_level_boost tests/test_meter
	rm -f tests/test_monitor tests/test_multiband
	rm -f tests/test_natural_dynamics
	rm -f tests/test_playlist_check
	rm -f tests/test_professional_check
	rm -f tests/test_quality_report
	rm -f tests/test_report_tool
	rm -f tests/test_resampler tests/test_restoration tests/test_spectrum tests/test_ssb
	rm -f tests/test_waveform
	rm -f tests/test_playout tests/test_tui tests/test_validation tests/test_wav
	rm -f tests/playout_bad.txt tests/playout_good.txt
	rm -f tests/playout_report.txt
	rm -f tests/playlist_check.txt tests/playlist_too_long.txt
	rm -f tests/wav_input.wav tests/wav_output.wav

.PHONY: all autodetect check-fftw check-flrig check-gui check-hamlib check-portaudio check-sndfile check-sndio check-tui clean dist dist-check feature-summary install install-manifest install-smoke professional-check quality quality-json release-check test uninstall validate
