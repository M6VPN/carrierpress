/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/src/cp_biquad.c */

#include <math.h>

#include "cp_biquad.h"

#define CP_BIQUAD_DENORMAL_FLOOR	(0.00000000000000000001f)
#define CP_BIQUAD_MAX_GAIN_DB	(24.0f)
#define CP_BIQUAD_MIN_GAIN_DB	(-24.0f)
#define CP_BIQUAD_MAX_Q		(1000.0f)
#define CP_BIQUAD_MIN_Q		(0.1f)
#define CP_BIQUAD_TWO_PI	(6.28318530717958647692f)

static cp_sample_t	cp_biquad_clean(cp_sample_t);
static int		cp_biquad_check_filter(const struct cp_biquad_coeff *);
static int		cp_biquad_check_params(cp_sample_t, cp_sample_t,
			    cp_sample_t);

int
cp_biquad_low_shelf(struct cp_biquad_coeff *coeff, cp_sample_t sample_rate,
	cp_sample_t frequency, cp_sample_t gain_db)
{
	cp_sample_t a;
	cp_sample_t a0;
	cp_sample_t alpha;
	cp_sample_t cos_w0;
	cp_sample_t root_a;
	cp_sample_t sin_w0;
	cp_sample_t w0;

	if (coeff == NULL)
		return CP_ERR_NULL;
	if (cp_biquad_check_params(sample_rate, frequency, 1.0f) != CP_OK)
		return CP_ERR_RANGE;
	if (!isfinite(gain_db) || gain_db < CP_BIQUAD_MIN_GAIN_DB ||
	    gain_db > CP_BIQUAD_MAX_GAIN_DB)
		return CP_ERR_RANGE;

	w0     = (CP_BIQUAD_TWO_PI * frequency) / sample_rate;
	sin_w0 = sinf(w0);
	cos_w0 = cosf(w0);
	a      = powf(10.0f, gain_db / 40.0f);
	root_a = sqrtf(a);
	alpha  = sin_w0 * 0.5f * sqrtf(2.0f);
	a0 = (a + 1.0f) + ((a - 1.0f) * cos_w0) +
	    (2.0f * root_a * alpha);
	if (!isfinite(a0) || a0 <= 0.0f)
		return CP_ERR_RANGE;

	coeff->b0 = (a * ((a + 1.0f) - ((a - 1.0f) * cos_w0) +
	    (2.0f * root_a * alpha))) / a0;
	coeff->b1 = (2.0f * a * ((a - 1.0f) -
	    ((a + 1.0f) * cos_w0))) / a0;
	coeff->b2 = (a * ((a + 1.0f) - ((a - 1.0f) * cos_w0) -
	    (2.0f * root_a * alpha))) / a0;
	coeff->a1 = (-2.0f * ((a - 1.0f) +
	    ((a + 1.0f) * cos_w0))) / a0;
	coeff->a2 = ((a + 1.0f) + ((a - 1.0f) * cos_w0) -
	    (2.0f * root_a * alpha)) / a0;

	return cp_biquad_check_filter(coeff);
}

int
cp_biquad_high_shelf(struct cp_biquad_coeff *coeff, cp_sample_t sample_rate,
	cp_sample_t frequency, cp_sample_t gain_db)
{
	cp_sample_t a;
	cp_sample_t a0;
	cp_sample_t alpha;
	cp_sample_t cos_w0;
	cp_sample_t root_a;
	cp_sample_t sin_w0;
	cp_sample_t w0;

	if (coeff == NULL)
		return CP_ERR_NULL;
	if (cp_biquad_check_params(sample_rate, frequency, 1.0f) != CP_OK)
		return CP_ERR_RANGE;
	if (!isfinite(gain_db) || gain_db < CP_BIQUAD_MIN_GAIN_DB ||
	    gain_db > CP_BIQUAD_MAX_GAIN_DB)
		return CP_ERR_RANGE;

	w0     = (CP_BIQUAD_TWO_PI * frequency) / sample_rate;
	sin_w0 = sinf(w0);
	cos_w0 = cosf(w0);
	a      = powf(10.0f, gain_db / 40.0f);
	root_a = sqrtf(a);
	alpha  = sin_w0 * 0.5f * sqrtf(2.0f);
	a0 = (a + 1.0f) - ((a - 1.0f) * cos_w0) +
	    (2.0f * root_a * alpha);
	if (!isfinite(a0) || a0 <= 0.0f)
		return CP_ERR_RANGE;

	coeff->b0 = (a * ((a + 1.0f) + ((a - 1.0f) * cos_w0) +
	    (2.0f * root_a * alpha))) / a0;
	coeff->b1 = (-2.0f * a * ((a - 1.0f) +
	    ((a + 1.0f) * cos_w0))) / a0;
	coeff->b2 = (a * ((a + 1.0f) + ((a - 1.0f) * cos_w0) -
	    (2.0f * root_a * alpha))) / a0;
	coeff->a1 = (2.0f * ((a - 1.0f) -
	    ((a + 1.0f) * cos_w0))) / a0;
	coeff->a2 = ((a + 1.0f) - ((a - 1.0f) * cos_w0) -
	    (2.0f * root_a * alpha)) / a0;

	return cp_biquad_check_filter(coeff);
}

int
cp_biquad_notch(struct cp_biquad_coeff *coeff, cp_sample_t sample_rate,
	cp_sample_t frequency, cp_sample_t q_factor)
{
	cp_sample_t a0;
	cp_sample_t alpha;
	cp_sample_t cos_w0;
	cp_sample_t sin_w0;
	cp_sample_t w0;

	if (coeff == NULL)
		return CP_ERR_NULL;
	if (!isfinite(sample_rate) || !isfinite(frequency) ||
	    !isfinite(q_factor))
		return CP_ERR_RANGE;
	if (sample_rate <= 0.0f || frequency <= 0.0f ||
	    frequency >= (sample_rate * 0.5f))
		return CP_ERR_RANGE;
	if (q_factor < CP_BIQUAD_MIN_Q || q_factor > CP_BIQUAD_MAX_Q)
		return CP_ERR_RANGE;

	/*
	 * RBJ notch filter:
	 * H(z) = (1 - 2*cos(w0)z^-1 + z^-2) /
	 *        (1 + alpha - 2*cos(w0)z^-1 + (1 - alpha)z^-2)
	 */
	w0     = (CP_BIQUAD_TWO_PI * frequency) / sample_rate;
	sin_w0 = sinf(w0);
	cos_w0 = cosf(w0);
	alpha  = sin_w0 / (2.0f * q_factor);
	a0     = 1.0f + alpha;

	if (!isfinite(a0) || a0 <= 0.0f)
		return CP_ERR_RANGE;

	coeff->b0 = 1.0f / a0;
	coeff->b1 = (-2.0f * cos_w0) / a0;
	coeff->b2 = 1.0f / a0;
	coeff->a1 = (-2.0f * cos_w0) / a0;
	coeff->a2 = (1.0f - alpha) / a0;

	if (!isfinite(coeff->b0) || !isfinite(coeff->b1) ||
	    !isfinite(coeff->b2) || !isfinite(coeff->a1) ||
	    !isfinite(coeff->a2))
		return CP_ERR_RANGE;

	return CP_OK;
}

int
cp_biquad_highpass(struct cp_biquad_coeff *coeff, cp_sample_t sample_rate,
	cp_sample_t frequency, cp_sample_t q_factor)
{
	cp_sample_t a0;
	cp_sample_t alpha;
	cp_sample_t cos_w0;
	cp_sample_t sin_w0;
	cp_sample_t w0;

	if (coeff == NULL)
		return CP_ERR_NULL;
	if (cp_biquad_check_params(sample_rate, frequency, q_factor) != CP_OK)
		return CP_ERR_RANGE;

	w0     = (CP_BIQUAD_TWO_PI * frequency) / sample_rate;
	sin_w0 = sinf(w0);
	cos_w0 = cosf(w0);
	alpha  = sin_w0 / (2.0f * q_factor);
	a0     = 1.0f + alpha;

	if (!isfinite(a0) || a0 <= 0.0f)
		return CP_ERR_RANGE;

	coeff->b0 = ((1.0f + cos_w0) * 0.5f) / a0;
	coeff->b1 = -(1.0f + cos_w0) / a0;
	coeff->b2 = ((1.0f + cos_w0) * 0.5f) / a0;
	coeff->a1 = (-2.0f * cos_w0) / a0;
	coeff->a2 = (1.0f - alpha) / a0;

	return cp_biquad_check_filter(coeff);
}

int
cp_biquad_allpass(struct cp_biquad_coeff *coeff, cp_sample_t sample_rate,
	cp_sample_t frequency, cp_sample_t q_factor)
{
	cp_sample_t a0;
	cp_sample_t alpha;
	cp_sample_t cos_w0;
	cp_sample_t sin_w0;
	cp_sample_t w0;

	if (coeff == NULL)
		return CP_ERR_NULL;
	if (cp_biquad_check_params(sample_rate, frequency, q_factor) != CP_OK)
		return CP_ERR_RANGE;

	w0     = (CP_BIQUAD_TWO_PI * frequency) / sample_rate;
	sin_w0 = sinf(w0);
	cos_w0 = cosf(w0);
	alpha  = sin_w0 / (2.0f * q_factor);
	a0     = 1.0f + alpha;

	if (!isfinite(a0) || a0 <= 0.0f)
		return CP_ERR_RANGE;

	coeff->b0 = (1.0f - alpha) / a0;
	coeff->b1 = (-2.0f * cos_w0) / a0;
	coeff->b2 = 1.0f;
	coeff->a1 = (-2.0f * cos_w0) / a0;
	coeff->a2 = (1.0f - alpha) / a0;

	return cp_biquad_check_filter(coeff);
}

int
cp_biquad_lowpass(struct cp_biquad_coeff *coeff, cp_sample_t sample_rate,
	cp_sample_t frequency, cp_sample_t q_factor)
{
	cp_sample_t a0;
	cp_sample_t alpha;
	cp_sample_t cos_w0;
	cp_sample_t sin_w0;
	cp_sample_t w0;

	if (coeff == NULL)
		return CP_ERR_NULL;
	if (cp_biquad_check_params(sample_rate, frequency, q_factor) != CP_OK)
		return CP_ERR_RANGE;

	/*
	 * RBJ low-pass filter. M5 cascades two identical 2nd-order sections
	 * per crossover point for a conservative 4th-order slope.
	 */
	w0     = (CP_BIQUAD_TWO_PI * frequency) / sample_rate;
	sin_w0 = sinf(w0);
	cos_w0 = cosf(w0);
	alpha  = sin_w0 / (2.0f * q_factor);
	a0     = 1.0f + alpha;

	if (!isfinite(a0) || a0 <= 0.0f)
		return CP_ERR_RANGE;

	coeff->b0 = ((1.0f - cos_w0) * 0.5f) / a0;
	coeff->b1 = (1.0f - cos_w0) / a0;
	coeff->b2 = ((1.0f - cos_w0) * 0.5f) / a0;
	coeff->a1 = (-2.0f * cos_w0) / a0;
	coeff->a2 = (1.0f - alpha) / a0;

	return cp_biquad_check_filter(coeff);
}

cp_sample_t
cp_biquad_process_sample(const struct cp_biquad_coeff *coeff,
	struct cp_biquad_state *state, cp_sample_t input)
{
	cp_sample_t output;
	cp_sample_t sample;
	cp_sample_t z1;
	cp_sample_t z2;

	if (coeff == NULL || state == NULL)
		return 0.0f;

	sample = cp_biquad_clean(input);
	output = (coeff->b0 * sample) + state->z1;
	output = cp_biquad_clean(output);

	z1 = (coeff->b1 * sample) - (coeff->a1 * output) + state->z2;
	z2 = (coeff->b2 * sample) - (coeff->a2 * output);

	state->z1 = cp_biquad_clean(z1);
	state->z2 = cp_biquad_clean(z2);

	return output;
}

int
cp_biquad_reset(struct cp_biquad_state *state)
{
	if (state == NULL)
		return CP_ERR_NULL;

	state->z1 = 0.0f;
	state->z2 = 0.0f;

	return CP_OK;
}

static cp_sample_t
cp_biquad_clean(cp_sample_t sample)
{
	if (!isfinite(sample))
		return 0.0f;
	if (fabsf(sample) < CP_BIQUAD_DENORMAL_FLOOR)
		return 0.0f;

	return sample;
}

static int
cp_biquad_check_filter(const struct cp_biquad_coeff *coeff)
{
	if (coeff == NULL)
		return CP_ERR_NULL;
	if (!isfinite(coeff->b0) || !isfinite(coeff->b1) ||
	    !isfinite(coeff->b2) || !isfinite(coeff->a1) ||
	    !isfinite(coeff->a2))
		return CP_ERR_RANGE;

	return CP_OK;
}

static int
cp_biquad_check_params(cp_sample_t sample_rate, cp_sample_t frequency,
	cp_sample_t q_factor)
{
	if (!isfinite(sample_rate) || !isfinite(frequency) ||
	    !isfinite(q_factor))
		return CP_ERR_RANGE;
	if (sample_rate <= 0.0f || frequency <= 0.0f ||
	    frequency >= (sample_rate * 0.5f))
		return CP_ERR_RANGE;
	if (q_factor < CP_BIQUAD_MIN_Q || q_factor > CP_BIQUAD_MAX_Q)
		return CP_ERR_RANGE;

	return CP_OK;
}
