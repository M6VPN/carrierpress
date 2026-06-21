/* CarrierPress - Developed by M6VPN (M6VPN@tuta.com) */
/* carrierpress/include/cp_config_file.h */

#ifndef CP_CONFIG_FILE_H
#define CP_CONFIG_FILE_H

#include <stddef.h>

#include "cp_audio.h"
#include "cp_types.h"

#define CP_CONFIG_FILE_PATH_SIZE	256
#define CP_CONFIG_FILE_DEVICE_SIZE	128
#define CP_CONFIG_FILE_KEY_SIZE		64
#define CP_CONFIG_FILE_ERROR_SIZE	128
#define CP_CONFIG_FILE_LINE_SIZE	256

struct cp_config_file {
	char profile_path[CP_CONFIG_FILE_PATH_SIZE];
	char device_name[CP_CONFIG_FILE_DEVICE_SIZE];
	enum cp_audio_backend audio_backend;
	int input_device;
	int output_device;
	double sample_rate;
	size_t channels;
	size_t block_size;
	unsigned int meter_interval_ms;
	int tui_enabled;
	int gui_enabled;
	unsigned int seen_profile;
	unsigned int seen_audio_backend;
	unsigned int seen_device;
	unsigned int seen_input_device;
	unsigned int seen_output_device;
	unsigned int seen_sample_rate;
	unsigned int seen_channels;
	unsigned int seen_block_size;
	unsigned int seen_meter_interval_ms;
	unsigned int seen_tui;
	unsigned int seen_gui;
};

struct cp_config_file_error {
	size_t line_number;
	char key[CP_CONFIG_FILE_KEY_SIZE];
	char message[CP_CONFIG_FILE_ERROR_SIZE];
};

void	cp_config_file_init(struct cp_config_file *);
int	cp_config_file_apply_to_audio_config(const struct cp_config_file *,
	    struct cp_audio_config *);
int	cp_config_file_has_profile(const struct cp_config_file *);
int	cp_config_file_parse_file(const char *, struct cp_config_file *,
	    struct cp_config_file_error *);
int	cp_config_file_parse_line(const char *, size_t,
	    struct cp_config_file *, struct cp_config_file_error *);
const char	*cp_config_file_profile_path(const struct cp_config_file *);
int	cp_config_file_validate(const struct cp_config_file *,
	    struct cp_config_file_error *);

#endif
