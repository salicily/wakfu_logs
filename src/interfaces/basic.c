#include "basic.h"
#include <stdlib.h>
#include <stdio.h>

struct iface_state {
	size_t next_entry;
};

static struct iface_state *basic_init(void) {
	printf("%s\n", __func__);
	struct iface_state *st = malloc(sizeof(*st));
	if (st != NULL) {
		st->next_entry = 0;
	}
	return st;
}

static const char *chan(enum chan_id ci) {
	switch (ci) {
		case chan_commerce:    return "commerce";
		case chan_guilde:      return "guilde";
		case chan_proximite:   return "proximite";
		case chan_recrutement: return "recrutement";
		case chan_prive_from:  return "prive_from";
		case chan_prive_to:    return "prive_to";
		default:               return "invalid";
	}
}

static int basic_refresh(struct iface_state *state, struct logs *logs) {
	printf("%s\n", __func__);
	struct entry e;
	size_t ne = logs_get_next_entry(logs);
	printf("%zu\n", ne);
	while (state->next_entry < ne) {
		printf("Trying entry %zu\n", state->next_entry);
		int r = logs_get_entry(logs, state->next_entry, &e);
		if (r == 0) {
			printf("Entry found\n");
			static char name[64];
			static char buf[512];
			printf("Getting text\n");
			logs_get_text(logs, e.text.offset, e.text.size, buf);
			printf("Getting source\n");
			logs_name_source(logs, e.src, name, sizeof(name));
			printf("%u - %s, %s: %.*s\n", e.time, chan(e.chan), name, e.text.size, buf);
		} else {
			printf("Entry not found\n");
		}
		++state->next_entry;
	}
	return 1;
}

static void basic_release(struct iface_state *state) {
	printf("%s\n", __func__);
	free(state);
	return;
}

struct interface basic = {
	.name = "basic",
	.init = basic_init,
	.refresh = basic_refresh,
	.release = basic_release,
};

