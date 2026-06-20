/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_spectrum.h */

#ifndef CP_SPECTRUM_H
#define CP_SPECTRUM_H

#include <sys/types.h>

#include <fftw3.h>

#include "cp_types.h"

#define CP_SPECTRUM_FFT_SIZE	512
#define CP_SPECTRUM_BINS	128
#define CP_SPECTRUM_SCALE	10000
#define CP_SPECTRUM_MIN_RATE	1000.0

struct cp_spectrum_input {
	int values[CP_SPECTRUM_FFT_SIZE];
	size_t frame_count;
	size_t channel_count;
	double sample_rate_hz;
	int valid;
};

struct cp_spectrum_snapshot {
	int magnitudes[CP_SPECTRUM_BINS];
	size_t bin_count;
	double sample_rate_hz;
	size_t peak_bin;
	int peak_magnitude;
	int valid;
};

struct cp_spectrum_analyzer {
	float input[CP_SPECTRUM_FFT_SIZE];
	float window[CP_SPECTRUM_FFT_SIZE];
	fftwf_complex output[(CP_SPECTRUM_FFT_SIZE / 2) + 1];
	fftwf_plan plan;
	int initialized;
};

int	cp_spectrum_analyze(struct cp_spectrum_analyzer *,
	    const struct cp_spectrum_input *, struct cp_spectrum_snapshot *);
void	cp_spectrum_analyzer_close(struct cp_spectrum_analyzer *);
int	cp_spectrum_analyzer_init(struct cp_spectrum_analyzer *);
int	cp_spectrum_capture_input(struct cp_spectrum_input *,
	    const cp_sample_t *, size_t, size_t, double);
void	cp_spectrum_clear(struct cp_spectrum_snapshot *);
void	cp_spectrum_input_clear(struct cp_spectrum_input *);

#endif
