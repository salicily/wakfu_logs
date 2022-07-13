#include "inout.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

struct iface_state {
	size_t next_entry;
};

static struct iface_state *inout_init(void) {
	struct iface_state *st = malloc(sizeof(*st));
	if (st != NULL) {
		st->next_entry = 0;
	}
	return st;
}

static const char *chan_mod(enum chan_id ci) {
	switch (ci) {
		case chan_commerce:    return NULL;
		case chan_guilde:      return NULL;
		case chan_proximite:   return NULL;
		case chan_recrutement: return NULL;
		case chan_prive_from:  return NULL;
		case chan_prive_to:    return NULL;
		case chan_group:       return NULL;
		case chan_in:          return "\x1b[1m"; //bold
		case chan_out:         return "\x1b[2m"; //thin
		default:               return NULL;
	}
}

static const char *color(unsigned int index) {
	static const char *colors[] = {
		"\x1b[31m",
		"\x1b[32m",
		"\x1b[33m",
		"\x1b[35m",
		"\x1b[36m",
		"\x1b[37m",
		"\x1b[91m",
		"\x1b[92m",
		"\x1b[93m",
		"\x1b[95m",
		"\x1b[96m",
		"\x1b[97m",
	};
	size_t mod = sizeof(colors) / sizeof(const char *);
	return colors[index % mod];
}

static int inout_refresh(struct iface_state *state, struct logs *logs) {
	struct entry e;
	size_t ne = logs_get_next_entry(logs);
	while (state->next_entry < ne) {
		int r = logs_get_entry(logs, state->next_entry, &e);
		if ((r == 0) && (chan_mod(e.chan) != NULL)) {
			static char name[64];
			static char buf[512];
			logs_get_text(logs, e.text.offset, e.text.size, buf);
			logs_name_source(logs, e.src, name, sizeof(name));
			unsigned int aux = e.time;
			unsigned int s = aux % 60;
			aux /= 60;
			unsigned int m = aux % 60;
			aux /= 60;
			printf("%s%s%02u:%02u:%02u - %.26s\x1b[37G: %.*s\x1b[0m\n", color(e.src), chan_mod(e.chan), aux, m, s, name, e.text.size, buf);
		}
		++state->next_entry;
	}
	sleep(1);
	return 1;
}

static void inout_release(struct iface_state *state) {
	free(state);
	return;
}

struct interface inout = {
	.name = "inout",
	.init = inout_init,
	.refresh = inout_refresh,
	.release = inout_release,
};

