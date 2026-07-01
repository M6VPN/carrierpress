/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_bulletin.h */

#ifndef CP_BULLETIN_H
#define CP_BULLETIN_H

#include <stddef.h>
#include <stdio.h>

#include "cp_audio.h"
#include "cp_block.h"

#define CP_BULLETIN_NAME_SIZE		48
#define CP_BULLETIN_TEXT_SIZE		192
#define CP_BULLETIN_PATH_SIZE		256
#define CP_BULLETIN_ITEMS_MAX		32
#define CP_BULLETIN_LINE_SIZE		384
#define CP_BULLETIN_DEFAULT_REPEAT	1
#define CP_BULLETIN_DEFAULT_PRE_ROLL_MS	500
#define CP_BULLETIN_DEFAULT_POST_ROLL_MS	800

enum cp_bulletin_status {
	CP_BULLETIN_OK = 0,
	CP_BULLETIN_ERR_NULL = -1400,
	CP_BULLETIN_ERR_RANGE = -1401,
	CP_BULLETIN_ERR_BUFFER = -1402,
	CP_BULLETIN_ERR_PARSE = -1403,
	CP_BULLETIN_ERR_OPEN = -1404,
	CP_BULLETIN_ERR_TX_ARM = -1405
};

enum cp_bulletin_profile_id {
	CP_BULLETIN_PROFILE_HF_SSB_VOICE = 0,
	CP_BULLETIN_PROFILE_HF_SSB_NARROW,
	CP_BULLETIN_PROFILE_VHF_FM_VOICE,
	CP_BULLETIN_PROFILE_AM_BROADCAST_STYLE,
	CP_BULLETIN_PROFILE_DATA_CLEAN_PASS_THROUGH
};

enum cp_bulletin_ptt_mode {
	CP_BULLETIN_PTT_NONE = 0,
	CP_BULLETIN_PTT_CAT,
	CP_BULLETIN_PTT_SERIAL,
	CP_BULLETIN_PTT_VOX
};

enum cp_bulletin_item_type {
	CP_BULLETIN_ITEM_ID = 0,
	CP_BULLETIN_ITEM_FILE,
	CP_BULLETIN_ITEM_TTS,
	CP_BULLETIN_ITEM_PAUSE
};

struct cp_bulletin_profile_summary {
	enum cp_bulletin_profile_id id;
	const char *name;
	const char *description;
	double output_sample_rate;
	size_t channels;
	double highpass_hz;
	double lowpass_hz;
	int speech_compressor_enabled;
	int limiter_enabled;
	int intelligibility_eq_enabled;
	int silence_trimming_enabled;
	int data_clean;
	double target_peak_dbfs;
};

struct cp_bulletin_tx_gate {
	enum cp_bulletin_ptt_mode ptt_mode;
	int arm_tx;
	const char *rig;
	const char *serial_path;
	const char *station_id;
	unsigned int pre_roll_ms;
	unsigned int post_roll_ms;
};

struct cp_bulletin_item {
	enum cp_bulletin_item_type type;
	char text[CP_BULLETIN_TEXT_SIZE];
	char path[CP_BULLETIN_PATH_SIZE];
	unsigned int seconds;
};

struct cp_bulletin_schedule {
	char callsign[CP_BULLETIN_NAME_SIZE];
	enum cp_bulletin_profile_id profile_id;
	unsigned int repeat;
	unsigned int pre_roll_ms;
	unsigned int post_roll_ms;
	struct cp_bulletin_item items[CP_BULLETIN_ITEMS_MAX];
	size_t item_count;
};

int		cp_bulletin_apply_profile(enum cp_bulletin_profile_id,
		    struct cp_block_config *, struct cp_audio_config *);
const struct cp_bulletin_profile_summary
		*cp_bulletin_profile_summary(enum cp_bulletin_profile_id);
int		cp_bulletin_profile_from_string(const char *,
		    enum cp_bulletin_profile_id *);
const char	*cp_bulletin_profile_string(enum cp_bulletin_profile_id);
void		cp_bulletin_schedule_init(struct cp_bulletin_schedule *);
int		cp_bulletin_schedule_parse_file(const char *,
		    struct cp_bulletin_schedule *, char *, size_t);
int		cp_bulletin_schedule_parse_line(const char *, size_t,
		    struct cp_bulletin_schedule *, char *, size_t);
int		cp_bulletin_schedule_print_plan(
		    const struct cp_bulletin_schedule *, FILE *);
int		cp_bulletin_tx_gate_validate(
		    const struct cp_bulletin_tx_gate *, char *, size_t);
const char	*cp_bulletin_ptt_mode_string(enum cp_bulletin_ptt_mode);
int		cp_bulletin_ptt_mode_from_string(const char *,
		    enum cp_bulletin_ptt_mode *);
const char	*cp_bulletin_status_string(int);

#endif
