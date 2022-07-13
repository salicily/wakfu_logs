#include "dummy.h"
#include <unistd.h>

struct iface_state {
	size_t dummy;
};

static struct iface_state dummy_;

static struct iface_state *dummy_init(void) {
	return &dummy_;
}

static int dummy_refresh(struct iface_state *state, struct logs *logs) {
	(void)logs;
	sleep(1);
	return 1;
}

static void dummy_release(struct iface_state *state) {
	(void)state;
	return;
}

struct interface dummy = {
	.name = "dummy",
	.init = dummy_init,
	.refresh = dummy_refresh,
	.release = dummy_release,
};

