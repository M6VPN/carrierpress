/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_profile.h */

#ifndef CP_PROFILE_H
#define CP_PROFILE_H

#include <stddef.h>

#include "cp_am.h"
#include "cp_bass_eq.h"
#include "cp_dehummer.h"
#include "cp_multiband.h"
#include "cp_ssb.h"
#include "cp_types.h"

struct cp_audio_config;
struct cp_block_config;

#define CP_PROFILE_NAME_SIZE		64
#define CP_PROFILE_DESCRIPTION_SIZE	192
#define CP_PROFILE_KEY_SIZE		64
#define CP_PROFILE_ERROR_SIZE		128
#define CP_PROFILE_LINE_SIZE		256

enum cp_profile_mode {
	CP_PROFILE_MODE_UNSET = 0,
	CP_PROFILE_MODE_NEUTRAL,
	CP_PROFILE_MODE_AM,
	CP_PROFILE_MODE_SSB,
	CP_PROFILE_MODE_FILE_CLEANUP
};

enum cp_profile_switch {
	CP_PROFILE_SWITCH_UNSET = 0,
	CP_PROFILE_SWITCH_OFF,
	CP_PROFILE_SWITCH_ON
};

enum cp_profile_multiband_setting {
	CP_PROFILE_MULTIBAND_UNSET = 0,
	CP_PROFILE_MULTIBAND_OFF,
	CP_PROFILE_MULTIBAND_SPEECH,
	CP_PROFILE_MULTIBAND_MUSIC
};

enum cp_profile_bass_eq_setting {
	CP_PROFILE_BASS_EQ_UNSET = 0,
	CP_PROFILE_BASS_EQ_OFF,
	CP_PROFILE_BASS_EQ_WARM,
	CP_PROFILE_BASS_EQ_MUSIC,
	CP_PROFILE_BASS_EQ_SPEECH
};

enum cp_profile_am_preset_setting {
	CP_PROFILE_AM_PRESET_UNSET = 0,
	CP_PROFILE_AM_PRESET_OFF,
	CP_PROFILE_AM_PRESET_SAFE,
	CP_PROFILE_AM_PRESET_SHORTWAVE,
	CP_PROFILE_AM_PRESET_WIDE,
	CP_PROFILE_AM_PRESET_VOICE
};

enum cp_profile_ssb_preset_setting {
	CP_PROFILE_SSB_PRESET_UNSET = 0,
	CP_PROFILE_SSB_PRESET_OFF,
	CP_PROFILE_SSB_PRESET_SPEECH,
	CP_PROFILE_SSB_PRESET_NARROW,
	CP_PROFILE_SSB_PRESET_WIDE,
	CP_PROFILE_SSB_PRESET_GENTLE
};

struct cp_profile {
	char name[CP_PROFILE_NAME_SIZE];
	char description[CP_PROFILE_DESCRIPTION_SIZE];
	enum cp_profile_mode mode;
	enum cp_profile_switch dehummer;
	unsigned int hum_frequency;
	size_t hum_harmonics;
	enum cp_profile_multiband_setting multiband;
	size_t multiband_bands;
	enum cp_profile_multiband_setting multiband2;
	size_t multiband2_bands;
	enum cp_profile_bass_eq_setting bass_eq;
	enum cp_profile_switch natural_dynamics;
	enum cp_profile_switch low_level_boost;
	enum cp_profile_switch restoration_analysis;
	enum cp_profile_switch declipper;
	enum cp_profile_am_preset_setting am_preset;
	enum cp_profile_ssb_preset_setting ssb_preset;
	unsigned int seen_name;
	unsigned int seen_description;
	unsigned int seen_mode;
	unsigned int seen_dehummer;
	unsigned int seen_hum_frequency;
	unsigned int seen_hum_harmonics;
	unsigned int seen_multiband;
	unsigned int seen_multiband_bands;
	unsigned int seen_multiband2;
	unsigned int seen_multiband2_bands;
	unsigned int seen_bass_eq;
	unsigned int seen_natural_dynamics;
	unsigned int seen_low_level_boost;
	unsigned int seen_restoration_analysis;
	unsigned int seen_declipper;
	unsigned int seen_am_preset;
	unsigned int seen_ssb_preset;
};

struct cp_profile_error {
	size_t line_number;
	char key[CP_PROFILE_KEY_SIZE];
	char message[CP_PROFILE_ERROR_SIZE];
};

int		cp_profile_apply_to_configs(const struct cp_profile *,
		    struct cp_block_config *, struct cp_audio_config *);
void		cp_profile_init(struct cp_profile *);
int		cp_profile_mode_from_string(const char *,
		    enum cp_profile_mode *);
const char	*cp_profile_mode_string(enum cp_profile_mode);
int		cp_profile_parse_file(const char *, struct cp_profile *,
		    struct cp_profile_error *);
int		cp_profile_parse_line(const char *, size_t,
		    struct cp_profile *, struct cp_profile_error *);
int		cp_profile_validate(const struct cp_profile *,
		    struct cp_profile_error *);

#endif
