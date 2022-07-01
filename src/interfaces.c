#include "interface.h"
#include "interfaces/dummy.h"
#include "interfaces/basic.h"

static struct interface *interfaces[] = {
	&dummy,
	&basic,
};

#define MAX_IFACES (sizeof(interfaces)/sizeof(interfaces[0]))

size_t supported_interfaces(void) {
	return MAX_IFACES;
}

void get_interface(size_t index, struct interface *iface) {
	if (iface == NULL) {
		return;
	}
	if (index >= MAX_IFACES) {
		return;
	}
	*iface = *interfaces[index];
	return;
}

