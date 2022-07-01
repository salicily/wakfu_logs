#ifndef INTERFACE
#define INTERFACE

#include <stddef.h>
#include "log_engine.h"

struct iface_state;

struct interface {
	const char *name;
	struct iface_state *(*init)(void);
	int (*refresh)(struct iface_state *state, struct logs *logs);
	void (*release)(struct iface_state *state);
};

size_t supported_interfaces(void);

void get_interface(size_t index, struct interface *iface);

#endif

