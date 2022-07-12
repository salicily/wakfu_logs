#include "config.h"
#include <stdlib.h>
#include <unistd.h>

struct config *config_create(size_t max_profiles) {
	struct config *cfg = malloc(sizeof(*cfg) + max_profiles * sizeof(struct profile));
	if (cfg == NULL) {
		return NULL;
	}
	cfg->show_time = 1;
	cfg->default_profile.has_foreground = 0;
	cfg->default_profile.has_background = 0;
	cfg->default_profile.italic = 0;
	cfg->default_profile.underline = 0;
	cfg->default_profile.bold = 0;
	cfg->default_profile.faint = 0;
	cfg->default_profile.listed = 0;
	cfg->default_profile.hide = 0;
	cfg->default_profile.foreground = 0;
	cfg->default_profile.background = 0;
	for (size_t i = 0; i < 16; ++i) {
		cfg->channels[i].has_foreground = 0;
		cfg->channels[i].has_background = 0;
		cfg->channels[i].italic = 0;
		cfg->channels[i].underline = 0;
		cfg->channels[i].bold = 0;
		cfg->channels[i].faint = 0;
		cfg->channels[i].listed = 0;
		cfg->channels[i].hide = 0;
		cfg->channels[i].foreground = 0;
		cfg->channels[i].background = 0;
	}
	return cfg;
}

void config_destroy(struct config *cfg) {
	if (cfg == NULL) {
		return;
	}

	cfg->max_profiles = 0;
	cfg->show_time = 0;
	cfg->default_profile.has_foreground = 0;
	cfg->default_profile.has_background = 0;
	cfg->default_profile.italic = 0;
	cfg->default_profile.underline = 0;
	cfg->default_profile.bold = 0;
	cfg->default_profile.faint = 0;
	cfg->default_profile.listed = 0;
	cfg->default_profile.hide = 0;
	cfg->default_profile.foreground = 0;
	cfg->default_profile.background = 0;
	for (size_t i = 0; i < 16; ++i) {
		cfg->channels[i].has_foreground = 0;
		cfg->channels[i].has_background = 0;
		cfg->channels[i].italic = 0;
		cfg->channels[i].underline = 0;
		cfg->channels[i].bold = 0;
		cfg->channels[i].faint = 0;
		cfg->channels[i].listed = 0;
		cfg->channels[i].hide = 0;
		cfg->channels[i].foreground = 0;
		cfg->channels[i].background = 0;
	}
	free(cfg);
	return;
}

struct config *config_load(int file, struct logs *logs) {
	struct config cfg_;
	ssize_t rd = read(file, &cfg_, sizeof(cfg_));
	if ((rd < 0) || (((size_t)rd) < sizeof(cfg_))) {
		return NULL;
	}
	struct config *cfg = malloc(sizeof(*cfg) + cfg_.max_profiles * sizeof(struct profile));
	if (cfg == NULL) {
		return NULL;
	}
	*cfg = cfg_;
	rd = read(file, cfg->profiles, cfg->max_profiles * sizeof(struct profile));
	if ((rd < 0) || (((size_t)rd) < (cfg->max_profiles * sizeof(struct profile)))) {
		return NULL;
	}
	size_t i = 0;
	while (i < cfg->max_profiles) {
		size_t j;
		if (cfg->profiles[i].style.listed) {
			int r = logs_index_source(logs, cfg->profiles[i].name, &j);
			if (r != 0) {
				free(cfg);
				return NULL;
			}
			if (j == i) {
				++i;
			} else {
				struct profile prof = cfg->profiles[j];
				cfg->profiles[j] = cfg->profiles[i];
				cfg->profiles[i] = prof;
			}
		}
	}
	return cfg;
}

int config_save(int file, const struct config *cfg) {
	if (cfg == NULL) {
		return -1;
	}
	ssize_t rd = write(file, cfg, sizeof(*cfg) + cfg->max_profiles * sizeof(struct profile));
	if ((rd < 0) || (((size_t)rd) < (sizeof(*cfg) + cfg->max_profiles * sizeof(struct profile)))) {
		return -1;
	}
	return 0;
}

