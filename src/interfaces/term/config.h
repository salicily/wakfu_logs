#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stddef.h>
#include "../../log_engine.h"

struct style {
	unsigned int has_foreground:1;
	unsigned int has_background:1;
	unsigned int italic:1;
	unsigned int underline:1;
	unsigned int bold:1;
	unsigned int faint:1;
	unsigned int listed:1;
	unsigned int hide:1;
	uint8_t foreground;
	uint8_t background;
};

struct profile {
	char name[64];
	struct style style;
};

struct config {
	size_t max_profiles;
	unsigned int show_time:1;
	struct style default_profile;
	struct style channels[16];
	struct profile profiles[];
};

struct config *config_create(size_t max_profiles);

void config_destroy(struct config *cfg);

struct config *config_load(int file, struct logs *logs);

int config_save(int file, const struct config *cfg);

#endif /* CONFIG_H */

