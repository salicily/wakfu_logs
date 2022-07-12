#include "interface.h"
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#define BOLD "\x1b[1m"
#define NORM "\x1b[0m"

int main(int argc, char **argv) {
	char c;
	_Bool help_set = 0;
	_Bool iface_set = 0;
	_Bool log_set = 0;
	char *iface = "";
	char *lpath = "";
	char *opts = "i:l:";
	c = getopt(argc, argv, opts);
	while (c != -1) {
		switch (c) {
			case 'i':
				if (iface_set) {
					help_set = 1;
				}
				iface_set = 1;
				iface = optarg;
				break;
			case 'l':
				if (log_set) {
					help_set = 1;
				}
				log_set = 1;
				lpath = optarg;
				break;
			default:
				help_set = 1;
		}
		c = getopt(argc, argv, opts);
	}
	struct interface siface;
	if (!help_set) {
		if (!iface_set) {
			iface_set = 1;
			iface = "dummy";
		}
		size_t iface_idx = 0;
		size_t ifaces = supported_interfaces();
		dprintf(2, "%zu interfaces available\n", ifaces);
		while (iface_idx < ifaces) {
			get_interface(iface_idx, &siface);
			if (strcmp(siface.name, iface) == 0) {
				break;
			}
			++iface_idx;
		}
		if (iface_idx >= ifaces) {
			help_set = 1;
		} else {
			dprintf(2, "Selected interface %zu (%s)\n", iface_idx, siface.name);
		}
	}
	int log;
	if (!help_set) {
		if (!log_set) {
			help_set = 1;
		} else {
			log = open(lpath, O_RDWR);
			if (log == -1) {
				help_set = 1;
			}
		}
	}
	if (help_set) {
		char *progname = (argc > 0) ? argv[0] : "wlog";
		dprintf(2, BOLD "%s -i" NORM " <interface> " BOLD "-l" NORM " <logfile>\n", progname);
		dprintf(2, "List of available interfaces:\n");
		size_t ifaces = supported_interfaces();
		for (size_t iface_idx = 0; iface_idx < ifaces; ++iface_idx) {
			get_interface(iface_idx, &siface);
			dprintf(2, "  %s\n", siface.name);
		}
		return -1;
	}
	struct logs *lgs = logs_create(log, 200, 1000000, 5000);
	if (lgs == NULL) {
		dprintf(2, "Could not create logs structure, aborting\n");
		close(log);
		return -1;
	}
	struct iface_state *istate = siface.init();
	if (istate == NULL) {
		dprintf(2, "Could not create interface state, aborting\n");
		logs_destroy(lgs);
		close(log);
		return -1;
	}
	int cont = siface.refresh(istate, lgs);
	while (cont == 1) {
		int r = logs_refresh(lgs);
		while (r == 0) {
			r = logs_refresh(lgs);
		}
		if (r == 1) {
			cont = siface.refresh(istate, lgs);
			sleep(1);
		} else {
			dprintf(2, "Could not refresh logs\n");
			cont = 0;
		}
	}
	siface.release(istate);
	logs_destroy(lgs);
	close(log);
	return cont ? -1 : 0;
}

