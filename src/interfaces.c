#include "interface.h"
#include "interfaces/dummy.h"
#include "interfaces/basic.h"
#include "interfaces/simple_colors.h"
#include "interfaces/inout.h"
#include "interfaces/term.h"

static struct interface *interfaces[] = {
	&dummy,
	&basic,
	&simple_colors,
	&inout,
	&term,
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

